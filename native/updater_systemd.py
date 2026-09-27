"""Restricted building block for root-owned disposable systemd rehearsals only.

No production entrypoint: Activation's root guard remains authoritative.
Instances own fixed test units only after an absent-unit check under a lifetime
lock. Explicit restart recovery requires protected namespace provenance; no boot
or production adoption is supported.
"""
from contextlib import contextmanager
from dataclasses import dataclass
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import selectors
import stat
import subprocess
import tempfile
import time
import uuid

from updater_environment import regular_bytes
from updater_service import ProcessService

PREFIX = 'critter-lab-updater-test-'
SHA = re.compile(r'[0-9a-f]{40}\Z')


@dataclass(frozen=True)
class SystemdPolicy:
    unit_prefix: str
    release_mount: str
    state_mount: str
    gateway: str

    def unit(self, role):
        if role not in ('runtime', 'health', 'check'):
            raise ValueError('unknown fixed service role')
        return self.unit_prefix + role + '.service'


DISPOSABLE_POLICY = SystemdPolicy(PREFIX, '/run/critter-lab-updater-test-release',
                                 '/run/critter-lab-updater-test-state', '/run/critter-lab-updater-test-gateway')
INSTALLED_POLICY = SystemdPolicy('critterlab-native-managed-', '/run/critterlab-native-release',
                                '/run/critterlab-native-state', '/run/critterlab-native-gateway')
LOCAL_INSTALLED_POLICY = SystemdPolicy('critter-lab-installed-test-managed-', '/run/critter-lab-installed-test-release',
                                      '/run/critter-lab-installed-test-state', '/run/critter-lab-installed-test-gateway')


def protected(path, directory=False):
    path = Path(path)
    if not path.is_absolute() or path.resolve() != path:
        raise ValueError('absolute nonsymlink path required')
    for parent in (path, *path.parents):
        info = parent.lstat()
        if info.st_uid != 0 or info.st_mode & 0o022:
            raise ValueError('root-owned nonwritable path required')
    info = path.lstat()
    if directory and not stat.S_ISDIR(info.st_mode):
        raise ValueError('directory required')
    return path


class SystemdService:
    def __init__(self, environment):
        self.environment = environment
        self.config = environment.config

    @property
    def policy(self):
        return getattr(self.environment, 'policy', DISPOSABLE_POLICY)

    def _unit(self, health=False):
        return self.policy.unit('health' if health else 'runtime')

    def _command(self, unit, uid, gid, writable, executable):
        return ['/usr/bin/systemd-run', '--quiet', '--collect', '--unit=' + unit,
                '--service-type=exec',
                *(['--property=Description=' + self.policy.unit_prefix + self.environment.namespace_id]
                  if hasattr(self.environment, 'namespace_id') else []),
                '--property=User=' + str(uid),
                '--property=Group=' + str(gid), '--property=KillMode=control-group',
                '--property=NoNewPrivileges=yes', '--property=ProtectSystem=strict',
                '--property=ProtectHome=yes', '--property=PrivateTmp=yes',
                '--property=RestrictSUIDSGID=yes', '--property=UMask=0077',
                '--property=MemoryMax=256M',
                '--property=LimitFSIZE=8388608', '--property=LimitCPU=10',
                '--property=BindPaths=' + str(writable) + ':' + self.policy.state_mount,
                '--property=ReadWritePaths=' + self.policy.state_mount, *executable]

    def _drain(self, unit):
        # A synchronous stop waits for KillMode=control-group termination. Verify
        # both manager state and the old cgroup; failed/unknown responses fail closed.
        loaded = self.environment.command(['/usr/bin/systemctl', 'show', unit,
                                          '--property=LoadState', '--value']).stdout.strip()
        if loaded == b'not-found':
            return
        if loaded != b'loaded':
            raise ValueError('unknown service load state')
        result = self.environment.command(['/usr/bin/systemctl', 'show', unit,
                                          '--property=ControlGroup', '--value'])
        group = result.stdout.decode().strip()
        if group and (not group.startswith('/system.slice/' + unit) or '..' in group):
            raise ValueError('unexpected service cgroup')
        self.environment.command(['/usr/bin/systemctl', 'stop', unit])
        result = self.environment.command(['/usr/bin/systemctl', 'show', unit,
                                          '--property=ActiveState', '--value'])
        if result.stdout.strip() not in (b'inactive', b'failed'):
            raise ValueError('service did not stop')
        if group:
            directory = Path('/sys/fs/cgroup' + group)
            if directory.exists():
                for processes in directory.rglob('cgroup.procs'):
                    if processes.read_text().strip():
                        raise ValueError('service descendants remain')

    def stop(self):
        self.environment._require_ownership()
        self.environment.begin_maintenance()
        for health in (False, True):
            self._drain(self._unit(health))

    def start(self, release, health=False):
        self.stop()
        release = self.environment.release_path(release)
        env = self.environment
        command = self._launch_command(release, health)
        env._record_launch('health' if health else 'runtime', release, command)
        env.command(command)

    def _launch_command(self, release, health=False):
        command = self._command(self._unit(health), self.config['runtime_uid'],
                                self.config['runtime_gid'], self.environment.runtime,
                                ['/usr/bin/python3', self.policy.release_mount + '/presenter/server.py'])
        command[2:2] = ['--property=BindReadOnlyPaths=' + str(release) + ':' + self.policy.release_mount]
        environment = {'CRITTER_DEMO_BIND': '127.0.0.1',
                       'CRITTER_DEMO_PORT': str(self.config['health_port' if health else 'port']),
                       'CRITTER_DEMO_BINARY': self.policy.release_mount + '/bin/critter_lab',
                       'CRITTER_DEMO_SAVE': self.policy.state_mount + '/' + getattr(self.environment, 'save', Path('state')).name}
        command[2:2] = ['--setenv=' + key + '=' + value for key, value in environment.items()]
        return command

    def health(self, sha, deployed_at, health=False):
        # Share the bounded loopback health contract, without constructing its
        # unprivileged process-management backend.
        ProcessService.health(self, sha, deployed_at, health)


class SystemdEnvironmentBase:
    policy = DISPOSABLE_POLICY

    def __init__(self, config):
        if type(self) is SystemdEnvironmentBase:
            raise ValueError('explicit environment profile required')
        self._initialize(config, reopening=False)

    @classmethod
    def reopen_disposable_test(cls, config):
        if cls is not DisposableSystemdEnvironment:
            raise ValueError('exact disposable restart backend required')
        environment = cls.__new__(cls)
        environment._initialize(config, reopening=True)
        return environment

    @property
    def scratch_root(self):
        return self.root

    def _initialize(self, config, reopening):
        keys = {'root', 'port', 'health_port', 'timeout', 'disposable_systemd',
                'runtime_uid', 'runtime_gid', 'check_uid', 'check_gid'}
        if os.name != 'posix' or os.geteuid() != 0 or set(config) != keys:
            raise ValueError('root disposable systemd configuration required')
        if config['disposable_systemd'] is not True:
            raise ValueError('disposable systemd marker required')
        for key in ('runtime_uid', 'runtime_gid', 'check_uid', 'check_gid'):
            if type(config[key]) is not int or not 1 <= config[key] <= 2147483647:
                raise ValueError('nonzero numeric identities required')
        if config['runtime_uid'] == config['check_uid']:
            raise ValueError('separate check identity required')
        for key in ('port', 'health_port'):
            if type(config[key]) is not int or not 1024 <= config[key] <= 65535:
                raise ValueError('invalid loopback port')
        if config['port'] == config['health_port'] or type(config['timeout']) is not int or not 1 <= config['timeout'] <= 30:
            raise ValueError('invalid bounds')
        self.root = protected(config['root'], True)
        if not self.root.name.startswith(PREFIX) or self.root.stat().st_mode & 0o077:
            raise ValueError('private explicitly named test root required')
        marker = protected(self.root / '.disposable-systemd')
        if regular_bytes(marker, 100) != b'critter-lab-disposable-systemd\n':
            raise ValueError('test root marker missing')
        self.config = dict(config)
        self.gateway = protected(self.policy.gateway, True)
        if self.gateway.stat().st_mode & 0o777 != 0o755:
            raise ValueError('gateway directory requires0755')
        for parent in self.gateway.parents:
            if parent.stat().st_mode & 0o001 == 0:
                raise ValueError('gateway ancestors must be traversable')
        self.releases = protected(self.root / 'releases', True)
        self.control = protected(self.root / 'control', True)
        self.runtime = self.root / 'runtime'
        info = self.runtime.lstat()
        if self.runtime.resolve() != self.runtime or not stat.S_ISDIR(info.st_mode) or info.st_uid != config['runtime_uid'] or info.st_mode & 0o077:
            raise ValueError('private runtime directory required')
        self.save = self.runtime / 'state'
        self.service = SystemdService(self)
        self._lifetime_lock = None
        self._claim_test_namespace(reopening)
        try:
            self._finish_claim(reopening)
        except BaseException:
            self.close()
            raise

    def _finish_claim(self, reopening):
        self.begin_maintenance()
        self._namespace(reopening)
        if reopening:
            self._reclaim_units()

    def _claim_test_namespace(self, reopening=False):
        descriptor = os.open(self.gateway / 'environment.lock',
                             os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
        try:
            info = os.fstat(descriptor)
            if not stat.S_ISREG(info.st_mode) or info.st_uid != 0 or info.st_mode & 0o077:
                raise ValueError('unsafe test namespace lock')
            fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            for suffix in (() if reopening else ('runtime', 'health', 'check')):
                unit = self.policy.unit(suffix)
                result = self.command(['/usr/bin/systemctl', 'show', unit,
                                       '--property=LoadState', '--value'])
                if result.stdout.strip() != b'not-found':
                    raise ValueError('test namespace contains an existing unit')
            self._lifetime_lock = descriptor
        except BaseException:
            os.close(descriptor)
            raise

    def _atomic_control(self, path, record):
        protected(path.parent, True)
        temporary = path.parent / ('.namespace-' + uuid.uuid4().hex)
        descriptor = os.open(temporary, os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW | os.O_WRONLY, 0o600)
        try:
            with os.fdopen(descriptor, 'wb', closefd=False) as output:
                output.write(json.dumps(record, sort_keys=True).encode())
                output.flush()
                os.fsync(descriptor)
            os.replace(temporary, path)
            directory = os.open(path.parent, os.O_DIRECTORY | os.O_NOFOLLOW)
            try:
                os.fsync(directory)
            finally:
                os.close(directory)
        finally:
            os.close(descriptor)
            if temporary.exists():
                temporary.unlink()

    def _read_control(self, path):
        protected(path)
        if path.stat().st_mode & 0o077:
            raise ValueError('private namespace record required')
        return json.loads(regular_bytes(path, 32768))

    def _namespace(self, reopening):
        config_digest = hashlib.sha256(json.dumps(self.config, sort_keys=True).encode()).hexdigest()
        expected = {'schema': 1, 'root': str(self.root), 'config_sha256': config_digest,
                    'units': [self.policy.unit(role) for role in ('runtime', 'health', 'check')]}
        path = self.control / 'namespace.json'
        if reopening:
            record = self._read_control(path)
            if not isinstance(record, dict) or type(record.get('schema')) is not int or set(record) != set(expected) | {'namespace_id'}:
                raise ValueError('unknown disposable namespace record')
            if any(record[key] != value for key, value in expected.items()):
                raise ValueError('disposable namespace configuration mismatch')
            token = record['namespace_id']
            if not isinstance(token, str) or not re.fullmatch('[0-9a-f]{32}', token):
                raise ValueError('invalid disposable namespace identity')
            self.namespace_id = token
        else:
            if path.exists() or path.is_symlink():
                raise ValueError('fresh namespace already recorded; use explicit reopen')
            self.namespace_id = uuid.uuid4().hex
            self._atomic_control(path, {**expected, 'namespace_id': self.namespace_id})

    def _record_launch(self, role, release, command, scratch=None, arguments=()):
        self._require_ownership()
        if role not in ('runtime', 'health', 'check'):
            raise ValueError('unknown fixed role')
        record = {'schema': 1, 'namespace_id': self.namespace_id, 'role': role,
                  'sha': release.name, 'command': command,
                  'scratch': str(scratch) if scratch else None, 'arguments': list(map(str, arguments))}
        self._atomic_control(self.control / ('intent-' + role + '.json'), record)

    def _bus_property(self, object_path, interface, name):
        result = self.command(['/usr/bin/busctl', '--json=short', 'get-property',
                               'org.freedesktop.systemd1', object_path,
                               'org.freedesktop.systemd1.' + interface, name])
        return json.loads(result.stdout)['data']

    def _unit_object_path(self, unit):
        result = self.command(['/usr/bin/busctl', '--json=short', 'call',
                               'org.freedesktop.systemd1', '/org/freedesktop/systemd1',
                               'org.freedesktop.systemd1.Manager', 'GetUnit', 's', unit])
        data = json.loads(result.stdout)['data']
        if not isinstance(data, list) or len(data) != 1 or not data[0].startswith('/org/freedesktop/systemd1/unit/'):
            raise ValueError('unexpected unit object identity')
        return data[0]

    def _unit_properties(self, unit):
        object_path = self._unit_object_path(unit)
        properties = {}
        for interface, names in (
            ('Unit', ('Transient', 'FragmentPath', 'DropInPaths', 'Description')),
            ('Service', ('User', 'Group', 'ExecStart', 'Environment', 'ControlGroup',
                         'KillMode', 'NoNewPrivileges', 'ProtectSystem', 'ProtectHome',
                         'PrivateTmp', 'RestrictSUIDSGID', 'UMask', 'MemoryMax',
                         'LimitFSIZE', 'LimitCPU', 'BindPaths', 'BindReadOnlyPaths',
                         'ReadWritePaths', 'RuntimeMaxUSec', 'EnvironmentFiles',
                         'PassEnvironment', 'UnsetEnvironment', 'ExecStartPre',
                         'ExecStartPost', 'ExecStop', 'ExecStopPost', 'ExecCondition',
                         'RootDirectory', 'RootImage', 'DynamicUser', 'AmbientCapabilities',
                         'Type', 'RemainAfterExit'))):
            for name in names:
                properties[name] = self._bus_property(object_path, interface, name)
        return properties

    def _validate_fragment(self, unit, properties):
        expected = '/run/systemd/transient/' + unit
        if properties.get('FragmentPath') != expected:
            raise ValueError(unit + ': FragmentPath mismatch')
        path = protected(expected)
        if not stat.S_ISREG(path.stat().st_mode):
            raise ValueError(unit + ': FragmentPath must be regular')

    def _validate_unit(self, unit, intent, properties):
        roles = {self.policy.unit(role): role for role in ('runtime', 'health', 'check')}
        if unit not in roles:
            raise ValueError('unknown fixed unit')
        role = roles[unit]
        keys = {'schema', 'namespace_id', 'role', 'sha', 'command', 'scratch', 'arguments'}
        if (not isinstance(intent, dict) or set(intent) != keys or type(intent['schema']) is not int or intent['schema'] != 1
                or intent['namespace_id'] != self.namespace_id or intent['role'] != role):
            raise ValueError(unit + ': launch intent mismatch')
        release = self.release_path(self.releases / intent['sha'])
        # Reconstruct from validated fixed fields; never execute a stored command.
        command = self._expected_command(role, release, intent['scratch'], intent['arguments'])
        if command != intent['command']:
            raise ValueError(unit + ': command intent mismatch')
        executable = command.index('/usr/bin/python3')
        uid = self.config['check_uid' if role == 'check' else 'runtime_uid']
        gid = self.config['check_gid' if role == 'check' else 'runtime_gid']
        self._validate_fragment(unit, properties)
        expected = {'Transient': True, 'FragmentPath': '/run/systemd/transient/' + unit, 'DropInPaths': [],
                    'Description': self.policy.unit_prefix + self.namespace_id, 'User': str(uid), 'Group': str(gid),
                    'KillMode': 'control-group', 'NoNewPrivileges': True, 'ProtectSystem': 'strict',
                    'ProtectHome': 'yes', 'PrivateTmp': True, 'RestrictSUIDSGID': True,
                    'UMask': 0o077, 'MemoryMax': 256 * 1024 * 1024, 'LimitFSIZE': 8388608,
                    'LimitCPU': 10, 'EnvironmentFiles': [], 'PassEnvironment': [], 'UnsetEnvironment': [],
                    'ExecStartPre': [], 'ExecStartPost': [], 'ExecStop': [], 'ExecStopPost': [],
                    'ExecCondition': [], 'RootDirectory': '', 'RootImage': '', 'DynamicUser': False,
                    'AmbientCapabilities': 0, 'Type': 'exec', 'RemainAfterExit': False,
                    'ReadWritePaths': [self.policy.state_mount],
                    'Environment': sorted(item.removeprefix('--setenv=') for item in command if item.startswith('--setenv='))}
        for name, value in expected.items():
            observed = properties.get(name)
            if name == 'Environment' and isinstance(observed, list):
                observed = sorted(observed)
            if type(observed) is not type(value) or observed != value:
                raise ValueError(unit + ': ' + name + ' mismatch')
        starts = properties.get('ExecStart')
        if (not isinstance(starts, list) or len(starts) != 1 or len(starts[0]) < 3 or starts[0][2] is not False or starts[0][:3] != ['/usr/bin/python3', command[executable:], False]):
            raise ValueError(unit + ': ExecStart mismatch')
        writable = intent['scratch'] if role == 'check' else str(self.runtime)
        for name, source, destination in (('BindPaths', writable, self.policy.state_mount),
                                         ('BindReadOnlyPaths', str(release), self.policy.release_mount)):
            mounts = properties.get(name)
            if not isinstance(mounts, list) or len(mounts) != 1 or len(mounts[0]) != 4 or mounts[0][2] is not False or type(mounts[0][3]) is not int or mounts[0] != [source, destination, False, 16384]:
                raise ValueError(unit + ': ' + name + ' mismatch')
        group = properties.get('ControlGroup')
        if group not in ('', '/system.slice/' + unit):
            raise ValueError(unit + ': ControlGroup mismatch')
        runtime = properties.get('RuntimeMaxUSec')
        if runtime != (15_000_000 if role == 'check' else 18446744073709551615):
            raise ValueError(unit + ': RuntimeMaxUSec mismatch')

    def _expected_command(self, role, release, scratch, arguments):
        if role == 'check':
            scratch = Path(scratch)
            if scratch.parent != self.scratch_root or not scratch.name.startswith('check-') or scratch.resolve() != scratch:
                raise ValueError('recorded scratch outside private namespace')
            if scratch.exists():
                info = scratch.stat()
                if not stat.S_ISDIR(info.st_mode) or info.st_uid != self.config['check_uid'] or info.st_mode & 0o077:
                    raise ValueError('recorded scratch ownership mismatch')
            if not isinstance(arguments, list) or not 1 <= len(arguments) <= 8 or any(not isinstance(arg, str) or len(arg) > 128 for arg in arguments):
                raise ValueError('invalid recorded native arguments')
            return self._native_command(release, scratch / 'state', arguments)
        if scratch is not None or arguments != []:
            raise ValueError('unexpected runtime command fields')
        return self.service._launch_command(release, health=role == 'health')

    def _reclaim_units(self):
        owned = []
        # Validate every loaded unit before touching any; mismatch stays gated.
        for role in ('runtime', 'health', 'check'):
            unit = self.policy.unit(role)
            loaded = self.command(['/usr/bin/systemctl', 'show', unit, '--property=LoadState', '--value']).stdout.strip()
            if loaded == b'not-found':
                continue
            if loaded != b'loaded':
                raise ValueError(unit + ': LoadState mismatch')
            intent = self._read_control(self.control / ('intent-' + role + '.json'))
            self._validate_unit(unit, intent, self._unit_properties(unit))
            owned.append(unit)
        for unit in owned:
            self.service._drain(unit)

    def _require_ownership(self):
        if self._lifetime_lock is None:
            raise ValueError('test namespace ownership has ended')

    def close(self):
        if self._lifetime_lock is not None:
            descriptor, self._lifetime_lock = self._lifetime_lock, None
            os.close(descriptor)

    def __enter__(self):
        return self

    def __exit__(self, *exception):
        self.close()

    def command(self, arguments):
        process = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   env={'PATH': '/usr/bin:/bin'})
        streams = {process.stdout: bytearray(), process.stderr: bytearray()}
        deadline = time.monotonic() + 40
        try:
            with selectors.DefaultSelector() as selector:
                for stream in streams:
                    os.set_blocking(stream.fileno(), False)
                    selector.register(stream, selectors.EVENT_READ)
                while selector.get_map():
                    remaining = deadline - time.monotonic()
                    if remaining <= 0:
                        raise ValueError('fixed manager command timeout')
                    for key, _ in selector.select(min(remaining, .1)):
                        data = os.read(key.fileobj.fileno(), 16384)
                        if not data:
                            selector.unregister(key.fileobj)
                            continue
                        if len(streams[key.fileobj]) + len(data) > 65536:
                            raise ValueError('fixed manager response exceeds bound')
                        streams[key.fileobj].extend(data)
            result = process.wait(timeout=max(.01, deadline - time.monotonic()))
            if result:
                # Do not expose manager stderr or arbitrary unit environment.
                raise ValueError('fixed manager command failed')
            return subprocess.CompletedProcess(arguments, result, bytes(streams[process.stdout]), bytes(streams[process.stderr]))
        finally:
            if process.poll() is None:
                process.kill()
            process.wait()
            process.stdout.close()
            process.stderr.close()

    def admission_ready(self, release):
        """Verify privileged admission, including candidate namespace access."""
        release = protected(release, True)
        if release.stat().st_mode & 0o005 != 0o005:
            raise ValueError('release must be readable and traversable by isolated identities')
        for directory, dirs, files in os.walk(protected(release, True)):
            for name in dirs + files:
                entry = protected(Path(directory) / name)
                if not entry.is_dir() and not stat.S_ISREG(entry.stat().st_mode):
                    raise ValueError('admitted release requires regular files')
                required = 0o005 if entry.is_dir() else 0o004
                if entry.stat().st_mode & required != required:
                    raise ValueError('release entries must be readable by isolated identities')
        return release

    def release_path(self, release):
        release = Path(release)
        if release.parent != self.releases or not SHA.fullmatch(release.name):
            raise ValueError('fixed admitted release path required')
        return self.admission_ready(release)

    def begin_maintenance(self):
        self._require_ownership()
        self._gate_record(False, None)

    def accept(self, sha):
        if not isinstance(sha, str) or not SHA.fullmatch(sha):
            raise ValueError('accepted SHA required')
        self._require_ownership()
        # Caller must have durably reconciled acceptance before invoking this.
        self._gate_record(True, sha)

    def _gate_record(self, approved, sha):
        path = self.gateway / 'gate.json'
        temporary = self.gateway / ('.gate-' + uuid.uuid4().hex)
        descriptor = os.open(temporary, os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW | os.O_WRONLY, 0o600)
        try:
            info = os.fstat(descriptor)
            if not stat.S_ISREG(info.st_mode) or info.st_uid != 0:
                raise ValueError('unsafe gate temporary')
            os.fchmod(descriptor, 0o644)
            with os.fdopen(descriptor, 'wb', closefd=False) as output:
                output.write(json.dumps({'schema': 1, 'approved': approved,
                                         'accepted_sha': sha}).encode())
                output.flush()
                os.fsync(descriptor)
            os.replace(temporary, path)
            directory = os.open(self.gateway, os.O_DIRECTORY | os.O_NOFOLLOW)
            try:
                os.fsync(directory)
            finally:
                os.close(directory)
        finally:
            os.close(descriptor)
            if os.path.exists(temporary):
                os.unlink(temporary)

    def gate_open(self):
        try:
            record = json.loads(regular_bytes(protected(self.gateway / 'gate.json'), 256))
            return (set(record) == {'schema', 'approved', 'accepted_sha'} and type(record['schema']) is int
                    and record['schema'] == 1 and record['approved'] is True
                    and isinstance(record['accepted_sha'], str) and bool(SHA.fullmatch(record['accepted_sha'])))
        except (OSError, ValueError, TypeError):
            return False

    @contextmanager
    def _lock(self, path, owner=0):
        descriptor = os.open(path, os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
        try:
            info = os.fstat(descriptor)
            if not stat.S_ISREG(info.st_mode) or info.st_uid != owner or info.st_mode & 0o077:
                raise ValueError('unsafe lock')
            fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            yield
        finally:
            os.close(descriptor)

    def lock(self):
        return self._lock(self.control / 'activation.lock')

    def save_lock(self):
        path = Path(str(self.save) + '.lock')
        if not path.exists():
            descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_RDWR | os.O_NOFOLLOW, 0o600)
            os.fchown(descriptor, self.config['runtime_uid'], self.config['runtime_gid'])
            os.close(descriptor)
        return self._lock(path, self.config['runtime_uid'])

    @contextmanager
    def scratch(self):
        directory = Path(tempfile.mkdtemp(prefix='check-', dir=self.scratch_root))
        os.chown(directory, self.config['check_uid'], self.config['check_gid'])
        try:
            yield directory
        finally:
            shutil.rmtree(directory)

    def _native_command(self, release, save, arguments):
        unit = self.policy.unit('check')
        command = self.service._command(unit, self.config['check_uid'], self.config['check_gid'], save.parent,
                                        ['/usr/bin/python3', '-c', 'import os,sys; os.execv(sys.argv[1],sys.argv[1:])',
                                         self.policy.release_mount + '/bin/critter_lab', '--save',
                                         self.policy.state_mount + '/' + save.name, *map(str, arguments)])
        command[2:2] = ['--property=BindReadOnlyPaths=' + str(release) + ':' + self.policy.release_mount]
        command[2:2] = ['--wait', '--pipe', '--property=RuntimeMaxSec=15']
        return command

    def run_native(self, release, save, *arguments):
        self._require_ownership()
        release = self.release_path(release)
        save = Path(save)
        if save.name != 'state' or save.parent.parent != self.scratch_root or not save.parent.name.startswith('check-') or save.parent.resolve() != save.parent:
            raise ValueError('private compatibility scratch required')
        info = save.parent.stat()
        if info.st_uid != self.config['check_uid'] or info.st_mode & 0o077:
            raise ValueError('unsafe check scratch')
        if save.exists():
            regular_bytes(save, 8 * 1024 * 1024)
            os.chown(save, self.config['check_uid'], self.config['check_gid'], follow_symlinks=False)
            os.chmod(save, 0o600, follow_symlinks=False)
        unit = self.policy.unit('check')
        command = self._native_command(release, save, arguments)
        self._record_launch('check', release, command, save.parent, arguments)
        try:
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                       env={'PATH': '/usr/bin:/bin'})
            output = bytearray()
            totals = {process.stdout: 0, process.stderr: 0}
            deadline = time.monotonic() + 15
            try:
                with selectors.DefaultSelector() as selector:
                    for stream in totals:
                        os.set_blocking(stream.fileno(), False)
                        selector.register(stream, selectors.EVENT_READ)
                    while selector.get_map():
                        remaining = deadline - time.monotonic()
                        if remaining <= 0:
                            raise ValueError('native time bound exceeded')
                        for key, _ in selector.select(min(remaining, .1)):
                            stream = key.fileobj
                            data = os.read(stream.fileno(), 65536)
                            if not data:
                                selector.unregister(stream)
                                continue
                            totals[stream] += len(data)
                            if totals[stream] > 8 * 1024 * 1024:
                                raise ValueError('native output bound exceeded')
                            if stream is process.stdout:
                                output.extend(data)
                if process.wait(timeout=max(.01, deadline - time.monotonic())):
                    raise ValueError('native compatibility check failed')
                return bytes(output)
            finally:
                if process.poll() is None:
                    process.kill()
                process.wait()
                process.stdout.close()
                process.stderr.close()
        finally:
            self.service._drain(unit)


class DisposableSystemdEnvironment(SystemdEnvironmentBase):
    """Explicit marked disposable profile; existing constructors are unchanged."""
    pass


TOOL_FILES = ('activation_manager.py', 'updater_activation.py', 'updater_systemd.py',
              'updater_environment.py', 'updater_service.py', 'package_staging.py', 'deployment_gateway.py')
INSTALL_PROFILES = {
    'installed': {
        'config': '/etc/critterlab-native-updater/config.json',
        'tools': '/usr/local/lib/critterlab-native-updater',
        'releases': '/opt/critterlab-native/releases',
        'control': '/var/lib/critterlab-native-activation',
        'policy': INSTALLED_POLICY,
        'permanent_units': ('critterlab-native-activation.service', 'critterlab-native-gateway.service'),
    },
    'local-installed-test': {
        'config': '/etc/critter-lab-installed-test/config.json',
        'tools': '/usr/local/lib/critter-lab-installed-test',
        'releases': '/opt/critter-lab-installed-test/releases',
        'control': '/var/lib/critter-lab-installed-test/control',
        'save': '/var/lib/critter-lab-installed-test-owner/state',
        'policy': LOCAL_INSTALLED_POLICY,
        'permanent_units': ('critter-lab-installed-test-activation.service', 'critter-lab-installed-test-gateway.service'),
    },
}


class InstalledSystemdEnvironment(SystemdEnvironmentBase):
    """Recover-only installed-layout adapter; no installer/request/capture path."""
    def __init__(self, config_path):
        self._initialize_installed(config_path, new_local_test=False)

    @classmethod
    def initialize_local_test(cls, config_path):
        if cls is not InstalledSystemdEnvironment or Path(config_path) != Path(INSTALL_PROFILES['local-installed-test']['config']):
            raise ValueError('fixed local installed-layout test profile required')
        environment = cls.__new__(cls)
        environment._initialize_installed(config_path, new_local_test=True)
        return environment

    @property
    def scratch_root(self):
        return self._scratch_root

    def _initialize_installed(self, config_path, new_local_test):
        if os.name != 'posix' or os.geteuid() != 0 or type(self) is not InstalledSystemdEnvironment:
            raise ValueError('exact root installed environment required')
        path = Path(config_path)
        profiles = [(name, values) for name, values in INSTALL_PROFILES.items() if path == Path(values['config'])]
        if len(profiles) != 1:
            raise ValueError('fixed installed configuration path required')
        self.profile, layout = profiles[0]
        config = json.loads(regular_bytes(protected(path), 16384))
        keys = {'schema', 'profile', 'save', 'runtime_uid', 'runtime_gid', 'check_uid', 'check_gid',
                'port', 'health_port', 'timeout', 'tools_sha256', 'unit_sha256'}
        if not isinstance(config, dict) or set(config) != keys or type(config['schema']) is not int or config['schema'] != 1 or config['profile'] != self.profile:
            raise ValueError('exact installed configuration schema required')
        for key in ('runtime_uid', 'runtime_gid', 'check_uid', 'check_gid'):
            if type(config[key]) is not int or not 1 <= config[key] <= 2147483647:
                raise ValueError('nonzero installed identities required')
        if config['runtime_uid'] == config['check_uid']:
            raise ValueError('separate installed check identity required')
        for key in ('port', 'health_port'):
            if type(config[key]) is not int or not 1024 <= config[key] <= 65535:
                raise ValueError('invalid installed loopback port')
        if config['port'] == config['health_port'] or type(config['timeout']) is not int or not 1 <= config['timeout'] <= 30:
            raise ValueError('invalid installed bounds')
        hashes = config['tools_sha256']
        if not isinstance(hashes, dict) or set(hashes) != set(TOOL_FILES):
            raise ValueError('complete installed tool hashes required')
        self.tools = protected(layout['tools'], True)
        for name in TOOL_FILES:
            if not isinstance(hashes[name], str) or not re.fullmatch('[0-9a-f]{64}', hashes[name]):
                raise ValueError('invalid installed tool hash')
            tool = protected(self.tools / name)
            if hashlib.sha256(regular_bytes(tool, 2 * 1024 * 1024)).hexdigest() != hashes[name]:
                raise ValueError('installed tool integrity mismatch: ' + name)
        if Path(__file__).resolve() != self.tools / 'updater_systemd.py':
            raise ValueError('installed backend must execute from verified fixed tools')
        units = config['unit_sha256']
        if not isinstance(units, dict) or set(units) != set(layout['permanent_units']):
            raise ValueError('exact permanent manager/gateway fragment hashes required')
        for name in layout['permanent_units']:
            if not isinstance(units[name], str) or not re.fullmatch('[0-9a-f]{64}', units[name]):
                raise ValueError('invalid permanent fragment hash')
            fragment = protected('/etc/systemd/system/' + name)
            if hashlib.sha256(regular_bytes(fragment, 65536)).hexdigest() != units[name]:
                raise ValueError('permanent fragment integrity mismatch: ' + name)
            self._validate_permanent_unit(name)
        self.config = config
        self.policy = layout['policy']
        self.control = protected(layout['control'], True)
        if self.control.stat().st_mode & 0o077:
            raise ValueError('installed control must be private')
        self.root = self.control
        self.releases = protected(layout['releases'], True)
        self._scratch_root = protected(self.control / 'checks', True)
        if self._scratch_root.stat().st_mode & 0o077:
            raise ValueError('installed scratch parent must be private')
        self.save = Path(config['save'])
        self.runtime = self.save.parent
        self.service = SystemdService(self)
        self._lifetime_lock = None
        gate = Path(self.policy.gateway)
        protected(gate.parent, True)
        if not gate.exists():
            gate.mkdir(mode=0o755)
            gate.chmod(0o755)
        self.gateway = protected(gate, True)
        if self.gateway.stat().st_mode & 0o777 != 0o755:
            raise ValueError('installed gate directory requires0755')
        self._claim_test_namespace(reopening=not new_local_test)
        self._finish_installed_claim(layout, new_local_test)

    def _finish_installed_claim(self, layout, new_local_test):
        try:
            self.begin_maintenance()
            if self.profile == 'local-installed-test':
                if str(self.save) != layout['save'] or regular_bytes(protected(self.control / '.installed-layout-disposable'), 100) != b'critter-lab-installed-layout-disposable\n':
                    raise ValueError('explicit fixed local installed-layout marker/save required')
            elif new_local_test:
                raise ValueError('production namespace initialization is unsupported')
            self._validate_existing_save()
            self._current_boot_id = regular_bytes(Path('/proc/sys/kernel/random/boot_id'), 100).decode().strip()
            if not re.fullmatch('[0-9a-f]{8}(?:-[0-9a-f]{4}){3}-[0-9a-f]{12}', self._current_boot_id):
                raise ValueError('invalid kernel boot identity')
            self._namespace(reopening=not new_local_test)
            if not new_local_test:
                self._reclaim_units()
            self._write_installed_namespace()
        except BaseException:
            self.close()
            raise

    def _validate_permanent_unit(self, unit):
        # A static inactive unit may be collected between GetUnit calls. Ask
        # systemctl for one coherent snapshot while its unit reference is held.
        result = self.command(['/usr/bin/systemctl', 'show', unit,
                               '--property=LoadState', '--property=Transient',
                               '--property=FragmentPath', '--property=DropInPaths'])
        expected_keys = {'LoadState', 'Transient', 'FragmentPath', 'DropInPaths'}
        properties = {}
        for line in result.stdout.decode().splitlines():
            name, separator, value = line.partition('=')
            if not separator or name not in expected_keys or name in properties:
                raise ValueError(unit + ': invalid permanent property snapshot')
            properties[name] = value
        if set(properties) != expected_keys:
            raise ValueError(unit + ': incomplete permanent property snapshot')
        if properties['LoadState'] == 'not-found':
            if properties != {'LoadState': 'not-found', 'Transient': 'no', 'FragmentPath': '', 'DropInPaths': ''}:
                raise ValueError(unit + ': inconsistent absent permanent snapshot')
            return
        expected = {'LoadState': 'loaded', 'Transient': 'no',
                    'FragmentPath': '/etc/systemd/system/' + unit, 'DropInPaths': ''}
        for name, value in expected.items():
            if properties[name] != value:
                raise ValueError(unit + ': permanent ' + name + ' mismatch')

    def _validate_existing_save(self):
        if not self.save.is_absolute() or self.save.resolve() != self.save or not re.fullmatch('[A-Za-z0-9_.-]{1,64}', self.save.name):
            raise ValueError('absolute nonsymlink external save required')
        for forbidden in (self.control, self.releases, self.tools):
            if self.save == forbidden or forbidden in self.save.parents:
                raise ValueError('save must be external to trusted tools/releases/control')
        protected(self.save.parent.parent, True)
        directory = self.save.parent.lstat()
        info = self.save.lstat()
        if (not stat.S_ISDIR(directory.st_mode) or directory.st_uid != self.config['runtime_uid']
                or directory.st_gid != self.config['runtime_gid'] or directory.st_mode & 0o777 != 0o700
                or not stat.S_ISREG(info.st_mode) or info.st_uid != self.config['runtime_uid']
                or info.st_gid != self.config['runtime_gid'] or info.st_mode & 0o777 != 0o600):
            raise ValueError('existing runtime-owned0700 directory/0600 regular save required')
        regular_bytes(self.save, 4096)

    def _installed_record(self):
        return {'schema': 2, 'profile': self.profile, 'root': str(self.root),
                'config_sha256': hashlib.sha256(json.dumps(self.config, sort_keys=True).encode()).hexdigest(),
                'units': [self.policy.unit(role) for role in ('runtime', 'health', 'check')],
                'namespace_id': self.namespace_id, 'boot_id': self._current_boot_id}

    def _namespace(self, reopening):
        path = self.control / 'namespace.json'
        if not reopening:
            if self.profile != 'local-installed-test' or path.exists() or path.is_symlink():
                raise ValueError('only fresh marked local installed-layout namespace may initialize')
            self.namespace_id = uuid.uuid4().hex
            return
        record = self._read_control(path)
        token = record.get('namespace_id') if isinstance(record, dict) else None
        boot = record.get('boot_id') if isinstance(record, dict) else None
        if not isinstance(token, str) or not re.fullmatch('[0-9a-f]{32}', token) or not isinstance(boot, str) or not re.fullmatch('[0-9a-f]{8}(?:-[0-9a-f]{4}){3}-[0-9a-f]{12}', boot):
            raise ValueError('invalid installed namespace identity')
        self.namespace_id = token
        expected = self._installed_record()
        if set(record) != set(expected) or type(record['schema']) is not int or any(record[key] != value for key, value in expected.items() if key != 'boot_id'):
            raise ValueError('installed protected namespace/configuration mismatch')

    def _write_installed_namespace(self):
        self._atomic_control(self.control / 'namespace.json', self._installed_record())
