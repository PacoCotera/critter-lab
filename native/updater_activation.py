"""Disposable Linux activation transaction; never a production privilege adapter."""
from contextlib import contextmanager
from datetime import datetime
import fcntl
import gzip
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import stat
import struct
import subprocess
import tarfile
import tempfile
from zoneinfo import ZoneInfo

import package_staging
from updater_service import ProcessService

LIMIT = 64 * 1024 * 1024


def sync_directory(path):
    descriptor = os.open(path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(descriptor)
    finally:
        os.close(descriptor)


def write_json(path, value):
    temporary = path.with_suffix('.new')
    with temporary.open('w') as output:
        json.dump(value, output, sort_keys=True)
        output.flush()
        os.fsync(output.fileno())
    os.replace(temporary, path)
    sync_directory(path.parent)


def regular_bytes(path, limit=LIMIT):
    descriptor = os.open(path, os.O_RDONLY | os.O_NOFOLLOW)
    try:
        info = os.fstat(descriptor)
        if not stat.S_ISREG(info.st_mode) or info.st_size > limit:
            raise ValueError('regular bounded file required')
        with os.fdopen(descriptor, 'rb', closefd=False) as source:
            data = source.read(limit + 1)
        if len(data) > limit:
            raise ValueError('file exceeds bound')
        return data
    finally:
        os.close(descriptor)


def digest(path):
    return hashlib.sha256(regular_bytes(path)).hexdigest()


def bounded_tar(path):
    # Bound all expanded bytes before tarfile consumes hidden PAX/GNU headers.
    import io
    compressed = regular_bytes(path)
    with gzip.GzipFile(fileobj=io.BytesIO(compressed)) as source:
        expanded = source.read(LIMIT + 1)
    if len(expanded) > LIMIT:
        raise ValueError('expanded archive exceeds bound')
    return tarfile.open(fileobj=io.BytesIO(expanded), mode='r:')


class Activation:
    def __init__(self, config):
        if os.geteuid() == 0 or set(config) != {'root', 'port', 'health_port', 'timeout', 'disposable'}:
            raise ValueError('unprivileged disposable configuration required')
        if config['disposable'] is not True:
            raise ValueError('production adapter is not implemented')
        root = Path(config['root'])
        if not root.is_absolute() or root.resolve() != root:
            raise ValueError('absolute nonsymlink disposable root required')
        if regular_bytes(root / '.disposable', 100) != b'critter-lab-disposable\n':
            raise ValueError('disposable root marker missing')
        for parent in (root, *root.parents):
            if parent.is_symlink():
                raise ValueError('symlink parent rejected')
        if root.stat().st_uid != os.getuid() or root.stat().st_mode & 0o077:
            raise ValueError('disposable root must be private and caller owned')
        for key in ('port', 'health_port'):
            if type(config[key]) is not int or not 1024 <= config[key] <= 65535:
                raise ValueError('invalid loopback port')
        if config['port'] == config['health_port'] or type(config['timeout']) is not int or not 1 <= config['timeout'] <= 30:
            raise ValueError('invalid service bounds')
        self.root = root
        self.releases, self.control = root / 'releases', root / 'control'
        for directory in (self.releases, self.control):
            directory.mkdir(mode=0o700, exist_ok=True)
            if directory.is_symlink() or directory.stat().st_mode & 0o077:
                raise ValueError('unsafe control directory')
        self.save = root / 'state'
        self.pointer, self.journal = self.control / 'current.json', self.control / 'journal.json'
        self.service = ProcessService({**config, 'control': str(self.control), 'save': str(self.save)})

    @contextmanager
    def lock(self):
        descriptor = os.open(self.control / 'activation.lock', os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
        try:
            fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            yield
        finally:
            os.close(descriptor)

    @contextmanager
    def save_lock(self):
        descriptor = os.open(str(self.save) + '.lock', os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
        try:
            fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            yield
        finally:
            os.close(descriptor)

    def read_record(self, path):
        return json.loads(regular_bytes(path, 10000))

    def release(self, sha):
        import re
        if not isinstance(sha, str) or not re.fullmatch('[0-9a-f]{40}', sha):
            raise ValueError('invalid selected SHA')
        path = self.releases / sha
        if path.is_symlink() or not path.is_dir():
            raise ValueError('selected release missing')
        return path

    def admit(self, bundle, sha, expected_digest):
        data = regular_bytes(bundle)
        if hashlib.sha256(data).hexdigest() != expected_digest:
            raise ValueError('incoming bundle changed before admission')
        with tempfile.TemporaryDirectory(dir=self.control) as temporary:
            archive = Path(temporary) / 'bundle.tar.gz'
            archive.write_bytes(data)
            with bounded_tar(archive) as source:
                count, total = 0, 0
                for member in source:
                    count += 1
                    total += member.size
                    if count > 7 or total > LIMIT or member.size < 0:
                        raise ValueError('bundle exceeds member bounds')
                if count != 7:
                    raise ValueError('wrong bundle member count')
            package_staging.verify(archive, sha)
            candidate = Path(temporary) / 'release'
            candidate.mkdir()
            with bounded_tar(archive) as source:
                for member in source:
                    name = Path(member.name).relative_to(package_staging.ROOT_NAME)
                    target = candidate / name
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with target.open('xb') as output:
                        output.write(source.extractfile(member).read())
                        output.flush()
                        os.fsync(output.fileno())
                    target.chmod(member.mode)
            shutil.copyfile(archive, candidate / 'original.tar.gz')
            with (candidate / 'original.tar.gz').open('rb') as original:
                os.fsync(original.fileno())
            write_json(candidate / 'admission.json', {'sha': sha, 'bundle_sha256': hashlib.sha256(data).hexdigest()})
            for directory in sorted((p for p in candidate.rglob('*') if p.is_dir()), reverse=True):
                sync_directory(directory)
            sync_directory(candidate)
            destination = self.releases / sha
            if destination.exists():
                if digest(destination / 'original.tar.gz') != expected_digest:
                    raise ValueError('different release already admitted')
                return self.check_integrity(sha)
            os.rename(candidate, destination)
            sync_directory(self.releases)
        return destination

    def check_integrity(self, sha):
        path = self.release(sha)
        admission = self.read_record(path / 'admission.json')
        if admission != {'sha': sha, 'bundle_sha256': digest(path / 'original.tar.gz')}:
            raise ValueError('admitted archive changed')
        package_staging.verify(path / 'original.tar.gz', sha)
        with bounded_tar(path / 'original.tar.gz') as source:
            for member in source:
                name = Path(member.name).relative_to(package_staging.ROOT_NAME)
                if name.as_posix() == 'presenter/release.json':
                    original = json.loads(source.extractfile(member).read())
                    installed = self.read_record(path / name)
                    if set(installed) != set(original) or any(installed[key] != original[key]
                            for key in original if key != 'deployed_at'):
                        raise ValueError('derived source metadata changed')
                    if installed['deployed_at'] is not None:
                        timestamp = datetime.fromisoformat(installed['deployed_at'])
                        if timestamp.tzinfo is None:
                            raise ValueError('activation timestamp lacks timezone')
                        receipt = self.read_record(path / 'activation.json')
                        if receipt.get('metadata_sha256') != digest(path / name):
                            raise ValueError('derived metadata receipt mismatch')
                    continue
                if regular_bytes(path / name) != source.extractfile(member).read():
                    raise ValueError('installed payload changed')
        return path

    def native(self, release, save, *arguments):
        import resource
        def limits():
            resource.setrlimit(resource.RLIMIT_FSIZE, (8 * 1024 * 1024, 8 * 1024 * 1024))
            resource.setrlimit(resource.RLIMIT_CPU, (10, 10))
            resource.setrlimit(resource.RLIMIT_AS, (256 * 1024 * 1024, 256 * 1024 * 1024))
        with tempfile.TemporaryFile() as output, tempfile.TemporaryFile() as errors:
            process = subprocess.Popen([str(release / 'bin/critter_lab'), '--save', str(save), *map(str, arguments)],
                                       stdout=output, stderr=errors, start_new_session=True,
                                       preexec_fn=limits, env={'PATH': '/usr/bin:/bin'})
            try:
                result = process.wait(timeout=10)
            finally:
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                process.wait()
            if result:
                raise ValueError('save compatibility check failed')
            output.seek(0)
            data = output.read(8 * 1024 * 1024 + 1)
            if len(data) > 8 * 1024 * 1024:
                raise ValueError('native output exceeds bound')
            return data

    def compatibility(self, sha, snapshot):
        release = self.check_integrity(sha)
        if not snapshot.startswith(b'CRITTER_DEMO 1\n'):
            raise ValueError('save schema outside approved scope')
        with tempfile.TemporaryDirectory(dir=self.control) as temporary:
            copy = Path(temporary) / 'state'
            copy.write_bytes(snapshot)
            status = json.loads(self.native(release, copy, 'status'))
            fields = dict(line.split(' ', 1) for line in snapshot.decode().splitlines()[1:])
            if status['revision'] != int(fields['revision']):
                raise ValueError('loaded save revision differs')
            if fields['last_id'] != '-':
                action, revision = fields['last_payload'].rsplit(':', 1)
                retried = json.loads(self.native(release, copy, 'command', action, revision, fields['last_id']))
                if retried != status:
                    raise ValueError('retained retry result differs')
            for device, width, height in (('lab', 1024, 600), ('probe', 122, 250), ('companion', 368, 448)):
                frame = self.native(release, copy, 'frame', device, status['revision'])
                if frame[:2] != b'BM' or struct.unpack_from('<ii', frame, 18) != (width, height):
                    raise ValueError('frame profile mismatch')
            if copy.read_bytes() != snapshot:
                raise ValueError('compatibility check rewrote save')
        return status

    def written_compatibility(self, candidate, previous):
        if previous is None:
            return
        release = self.check_integrity(candidate)
        with tempfile.TemporaryDirectory(dir=self.control) as temporary:
            copy = Path(temporary) / 'state'
            for index, action in enumerate(('review', 'load', 'start', 'advance', 'inspect',
                                            'advance', 'receive', 'study_review', 'run')):
                state = json.loads(self.native(release, copy, 'status'))
                self.native(release, copy, 'command', action, state['revision'], f'compat-{index}')
                self.compatibility(previous, copy.read_bytes())

    def selected(self, validate=True):
        if not self.pointer.exists():
            return None
        record = self.read_record(self.pointer)
        if set(record) != {'sha'}:
            raise ValueError('unknown selection')
        self.release(record['sha'])
        if validate:
            self.check_integrity(record['sha'])
        return record['sha']

    def start_verified(self, sha):
        release = self.check_integrity(sha)
        receipt = self.read_record(release / 'activation.json')
        metadata = self.read_record(release / 'presenter/release.json')
        if (receipt.get('sha') != sha or receipt.get('metadata_sha256') != digest(release / 'presenter/release.json')
                or metadata.get('commit') != sha or metadata.get('deployed_at') != receipt.get('deployed_at')):
            raise ValueError('activation metadata mismatch')
        self.service.start(release, health=True)
        self.service.health(sha, receipt['deployed_at'], health=True)
        self.service.stop()
        self.service.start(release)
        self.service.health(sha, receipt['deployed_at'])

    def transition_hook(self, phase):
        # Rehearsal subclasses inject a real process kill here. CLI has no hook option.
        pass

    def recover_locked(self):
        self.service.stop()
        journal = self.read_record(self.journal)
        if set(journal) != {'phase', 'old', 'candidate', 'save_digest'}:
            raise ValueError('unknown recovery journal')
        phase = journal['phase']
        if phase not in ('prepared', 'switched', 'accepted'):
            raise ValueError('unknown recovery phase')
        selected = self.selected(validate=False)
        if selected not in (journal['old'], journal['candidate']):
            raise ValueError('selection outside recovery journal')
        target = journal['candidate'] if phase == 'accepted' else journal['old']
        if target is None:
            raise ValueError('no verified rollback baseline; service remains stopped')
        with self.save_lock():
            snapshot = regular_bytes(self.save, 4096)
            self.compatibility(target, snapshot)
            write_json(self.pointer, {'sha': target})
            self.start_verified(target)
            if digest(self.save) != hashlib.sha256(snapshot).hexdigest():
                raise ValueError('recovery changed live save')
        self.journal.unlink()
        sync_directory(self.control)
        return target

    def recover(self):
        with self.lock():
            try:
                if self.journal.exists():
                    return self.recover_locked()
                self.service.stop()
                selected = self.selected()
                if selected is None:
                    raise ValueError('no selected release')
                with self.save_lock():
                    self.compatibility(selected, regular_bytes(self.save, 4096))
                    self.start_verified(selected)
                return selected
            except Exception:
                self.service.stop()
                raise

    def activate(self, sha, rollback=False):
        with self.lock():
            self.service.stop()
            if self.journal.exists():
                raise ValueError('recover unfinished transaction first')
            old = self.selected()
            if old == sha:
                raise ValueError('release already selected')
            self.check_integrity(sha)
            if not rollback and (self.release(sha) / 'activation.json').exists():
                raise ValueError('retained activation requires explicit rollback selection')
            if rollback and not self.read_record(self.release(sha) / 'activation.json').get('accepted_at'):
                raise ValueError('rollback target was never accepted')
            try:
                with self.save_lock():
                    snapshot = regular_bytes(self.save, 4096)
                    before = hashlib.sha256(snapshot).hexdigest()
                    self.compatibility(sha, snapshot)
                    self.written_compatibility(sha, old)
                    journal = {'phase': 'prepared', 'old': old, 'candidate': sha, 'save_digest': before}
                    write_json(self.journal, journal)
                    self.transition_hook('prepared')
                    release = self.release(sha)
                    if not rollback:
                        with bounded_tar(release / 'original.tar.gz') as original:
                            metadata = json.load(original.extractfile(package_staging.ROOT_NAME + '/presenter/release.json'))
                        metadata['deployed_at'] = datetime.now(ZoneInfo('America/Mexico_City')).isoformat()
                        write_json(release / 'presenter/release.json', metadata)
                        write_json(release / 'activation.json', {'sha': sha, 'deployed_at': metadata['deployed_at'],
                                   'metadata_sha256': digest(release / 'presenter/release.json')})
                    write_json(self.pointer, {'sha': sha})
                    self.transition_hook('pointer')
                    journal['phase'] = 'switched'
                    write_json(self.journal, journal)
                    self.transition_hook('switched')
                    self.start_verified(sha)
                    if digest(self.save) != before:
                        raise ValueError('activation changed live save')
                    receipt = self.read_record(release / 'activation.json')
                    if not rollback:
                        receipt['accepted_at'] = datetime.now(ZoneInfo('America/Mexico_City')).isoformat()
                        write_json(release / 'activation.json', receipt)
                    self.transition_hook('receipt')
                    journal['phase'] = 'accepted'
                    write_json(self.journal, journal)
                    self.transition_hook('accepted')
                self.journal.unlink()
                sync_directory(self.control)
                return sha
            except Exception:
                self.service.stop()
                if self.journal.exists():
                    try:
                        self.recover_locked()
                    except Exception:
                        self.service.stop()
                elif old is not None:
                    try:
                        with self.save_lock():
                            self.compatibility(old, regular_bytes(self.save, 4096))
                            self.start_verified(old)
                    except Exception:
                        self.service.stop()
                raise
