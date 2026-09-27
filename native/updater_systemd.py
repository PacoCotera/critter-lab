"""Restricted building block for root-owned disposable systemd rehearsals only.

No production entrypoint: Activation's root guard remains authoritative.
Instances own fixed test units only after an absent-unit check under a lifetime
lock. Cross-process root recovery is deliberately unsupported.
"""
from contextlib import contextmanager
import fcntl
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

    def _unit(self, health=False):
        return PREFIX + ('health' if health else 'runtime') + '.service'

    def _command(self, unit, uid, gid, writable, executable):
        return ['/usr/bin/systemd-run', '--quiet', '--collect', '--unit=' + unit,
                '--service-type=exec', '--property=User=' + str(uid),
                '--property=Group=' + str(gid), '--property=KillMode=control-group',
                '--property=NoNewPrivileges=yes', '--property=ProtectSystem=strict',
                '--property=ProtectHome=yes', '--property=PrivateTmp=yes',
                '--property=RestrictSUIDSGID=yes', '--property=UMask=0077',
                '--property=MemoryMax=256M',
                '--property=LimitFSIZE=8388608', '--property=LimitCPU=10',
                '--property=BindPaths=' + str(writable) + ':/run/critter-lab-updater-test-state',
                '--property=ReadWritePaths=/run/critter-lab-updater-test-state', *executable]

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
        command = self._command(self._unit(health), self.config['runtime_uid'],
                                self.config['runtime_gid'], env.runtime,
                                ['/usr/bin/python3', '/run/critter-lab-updater-test-release/presenter/server.py'])
        command[2:2] = ['--property=BindReadOnlyPaths=' + str(release) + ':/run/critter-lab-updater-test-release']
        environment = {'CRITTER_DEMO_BIND': '127.0.0.1',
                       'CRITTER_DEMO_PORT': str(self.config['health_port' if health else 'port']),
                       'CRITTER_DEMO_BINARY': '/run/critter-lab-updater-test-release/bin/critter_lab',
                       'CRITTER_DEMO_SAVE': '/run/critter-lab-updater-test-state/state'}
        command[2:2] = ['--setenv=' + key + '=' + value for key, value in environment.items()]
        env.command(command)

    def health(self, sha, deployed_at, health=False):
        # Share the bounded loopback health contract, without constructing its
        # unprivileged process-management backend.
        ProcessService.health(self, sha, deployed_at, health)


class DisposableSystemdEnvironment:
    def __init__(self, config):
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
        self.gateway = protected('/run/critter-lab-updater-test-gateway', True)
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
        self._claim_test_namespace()
        try:
            self.begin_maintenance()
        except BaseException:
            self.close()
            raise

    def _claim_test_namespace(self):
        descriptor = os.open(self.gateway / 'environment.lock',
                             os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
        try:
            info = os.fstat(descriptor)
            if not stat.S_ISREG(info.st_mode) or info.st_uid != 0 or info.st_mode & 0o077:
                raise ValueError('unsafe test namespace lock')
            fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            for suffix in ('runtime', 'health', 'check'):
                unit = PREFIX + suffix + '.service'
                result = self.command(['/usr/bin/systemctl', 'show', unit,
                                       '--property=LoadState', '--value'])
                if result.stdout.strip() != b'not-found':
                    raise ValueError('test namespace contains an existing unit')
            self._lifetime_lock = descriptor
        except BaseException:
            os.close(descriptor)
            raise

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
        return subprocess.run(arguments, check=True, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, timeout=40, env={'PATH': '/usr/bin:/bin'})

    def admission_ready(self, release):
        """Verify privileged admission, including candidate namespace access."""
        release = protected(release, True)
        if release.stat().st_mode & 0o005 != 0o005:
            raise ValueError('release must be readable and traversable by isolated identities')
        for directory, dirs, files in os.walk(protected(release, True)):
            for name in dirs + files:
                entry = protected(Path(directory) / name)
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
        directory = Path(tempfile.mkdtemp(prefix='check-', dir=self.root))
        os.chown(directory, self.config['check_uid'], self.config['check_gid'])
        try:
            yield directory
        finally:
            shutil.rmtree(directory)

    def run_native(self, release, save, *arguments):
        self._require_ownership()
        release = self.release_path(release)
        save = Path(save)
        if save.parent.parent != self.root or not save.parent.name.startswith('check-') or save.parent.resolve() != save.parent:
            raise ValueError('private compatibility scratch required')
        info = save.parent.stat()
        if info.st_uid != self.config['check_uid'] or info.st_mode & 0o077:
            raise ValueError('unsafe check scratch')
        if save.exists():
            regular_bytes(save, 8 * 1024 * 1024)
            os.chown(save, self.config['check_uid'], self.config['check_gid'], follow_symlinks=False)
            os.chmod(save, 0o600, follow_symlinks=False)
        unit = PREFIX + 'check.service'
        command = self.service._command(unit, self.config['check_uid'], self.config['check_gid'], save.parent,
                                        ['/usr/bin/python3', '-c', 'import os,sys; os.execv(sys.argv[1],sys.argv[1:])',
                                         '/run/critter-lab-updater-test-release/bin/critter_lab', '--save',
                                         '/run/critter-lab-updater-test-state/' + save.name, *map(str, arguments)])
        command[2:2] = ['--property=BindReadOnlyPaths=' + str(release) + ':/run/critter-lab-updater-test-release']
        command[2:2] = ['--wait', '--pipe', '--property=RuntimeMaxSec=15']
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
