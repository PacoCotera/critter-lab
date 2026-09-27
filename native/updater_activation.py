"""Disposable Linux activation transaction; never a production privilege adapter."""
from datetime import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import tarfile
import tempfile
import uuid
from zoneinfo import ZoneInfo

import package_staging
from updater_environment import DisposableEnvironment, regular_bytes

LIMIT = 64 * 1024 * 1024


def sync_directory(path):
    descriptor = os.open(path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(descriptor)
    finally:
        os.close(descriptor)


def write_json(path, value):
    temporary = path.parent / ('.record-' + uuid.uuid4().hex)
    descriptor = os.open(temporary, os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW | os.O_WRONLY, 0o600)
    try:
        os.fchmod(descriptor, 0o644)
        with os.fdopen(descriptor, 'w', closefd=False) as output:
            json.dump(value, output, sort_keys=True)
            output.flush()
            os.fsync(descriptor)
        os.replace(temporary, path)
        sync_directory(path.parent)
    finally:
        os.close(descriptor)
        if os.path.exists(temporary):
            os.unlink(temporary)


def digest(path):
    return hashlib.sha256(regular_bytes(path)).hexdigest()


def bounded_tar(path):
    return package_staging.bounded_tar(path)


class Activation:
    def __init__(self, config):
        self._bind_environment(DisposableEnvironment(config))

    @classmethod
    def from_environment(cls, environment):
        """Inject a trusted backend without duplicating the activation transaction."""
        if os.name != 'posix' or os.geteuid() == 0:
            raise ValueError('unprivileged Linux activation required')
        activation = cls.__new__(cls)
        activation._bind_environment(environment)
        return activation

    @classmethod
    def from_disposable_systemd(cls, environment):
        """Root test seam; no production or cross-process recovery admission."""
        from updater_systemd import DisposableSystemdEnvironment
        if os.name != 'posix' or os.geteuid() != 0 or type(environment) is not DisposableSystemdEnvironment:
            raise ValueError('exact root disposable systemd environment required')
        environment._require_ownership()
        activation = cls.__new__(cls)
        activation._bind_environment(environment)
        activation._systemd_test = True
        return activation

    def _bind_environment(self, environment):
        self._systemd_test = False
        self.environment = environment
        self.root = environment.root
        self.releases = environment.releases
        self.control = environment.control
        self.save = environment.save
        self.service = environment.service
        self.pointer = self.control / 'current.json'
        self.journal = self.control / 'journal.json'

    def lock(self):
        return self.environment.lock()

    def save_lock(self):
        return self.environment.save_lock()

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
            if self._systemd_test:
                # Only freshly extracted root-owned candidate directories become
                # readable inside the fixed isolated namespace. Control stays private.
                (candidate / 'original.tar.gz').chmod(0o644)
                with (candidate / 'original.tar.gz').open('rb') as original:
                    os.fsync(original.fileno())
                for directory in (candidate, *(entry for entry in candidate.rglob('*') if entry.is_dir())):
                    directory.chmod(0o755)
                    sync_directory(directory)
                self.environment.admission_ready(candidate)
            os.rename(candidate, destination)
            sync_directory(self.releases)
        return destination

    def check_integrity(self, sha):
        path = self.release(sha)
        if self._systemd_test:
            self.environment.admission_ready(path)
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
        return self.environment.run_native(release, save, *arguments)

    def compatibility(self, sha, snapshot):
        release = self.check_integrity(sha)
        if not snapshot.startswith(b'CRITTER_DEMO 1\n'):
            raise ValueError('save schema outside approved scope')
        with self.environment.scratch() as temporary:
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
            if regular_bytes(copy, 4096) != snapshot:
                raise ValueError('compatibility check rewrote save')
        return status

    def written_compatibility(self, candidate, previous):
        if previous is None:
            return
        release = self.check_integrity(candidate)
        with self.environment.scratch() as temporary:
            copy = Path(temporary) / 'state'
            for index, action in enumerate(('review', 'load', 'start', 'advance', 'inspect',
                                            'advance', 'receive', 'study_review', 'run')):
                state = json.loads(self.native(release, copy, 'status'))
                self.native(release, copy, 'command', action, state['revision'], f'compat-{index}')
                self.compatibility(previous, regular_bytes(copy, 4096))

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

    def _complete_systemd_test(self, operation, locked_operation, *arguments, **options):
        if not self._systemd_test:
            return operation(*arguments, **options)
        with self.lock():
            self.environment.begin_maintenance()
            result = locked_operation(*arguments, **options)
            self._verify_systemd_acceptance(result)
            self.environment.accept(result)
            return result

    def _verify_systemd_acceptance(self, sha):
        if self.journal.exists() or self.selected() != sha:
            raise ValueError('test acceptance is not durably reconciled')
        receipt = self.read_record(self.check_integrity(sha) / 'activation.json')
        accepted = datetime.fromisoformat(receipt.get('accepted_at', ''))
        if accepted.tzinfo is None:
            raise ValueError('test acceptance receipt lacks timezone')

    def recover(self):
        return self._complete_systemd_test(self._recover, self._recover_locked_transaction)

    def _recover(self):
        with self.lock():
            return self._recover_locked_transaction()

    def _recover_locked_transaction(self):
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
        return self._complete_systemd_test(self._activate, self._activate_locked_transaction, sha, rollback=rollback)

    def _activate(self, sha, rollback=False):
        with self.lock():
            return self._activate_locked_transaction(sha, rollback=rollback)

    def _activate_locked_transaction(self, sha, rollback=False):
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

