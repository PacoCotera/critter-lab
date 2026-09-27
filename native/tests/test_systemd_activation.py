"""Focused root test-seam and durable-write checks; no unit launches."""
import json
from contextlib import nullcontext
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from updater_activation import Activation, write_json
from updater_systemd import DisposableSystemdEnvironment


class SystemdActivationTests(unittest.TestCase):
    def environment(self):
        environment = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        environment._lifetime_lock = 1
        environment.root = Path('/fixed/test')
        environment.control = environment.root / 'control'
        environment.releases = environment.root / 'releases'
        environment.save = environment.root / 'runtime/state'
        environment.service = Mock()
        environment.lock = lambda: nullcontext()
        return environment

    def test_root_default_and_general_injection_remain_rejected(self):
        with patch('os.geteuid', return_value=0):
            with self.assertRaises(ValueError):
                Activation({})
            with self.assertRaises(ValueError):
                Activation.from_environment(self.environment())

    def test_explicit_seam_requires_exact_backend_root_and_ownership(self):
        environment = self.environment()
        with patch('os.geteuid', return_value=0):
            activation = Activation.from_disposable_systemd(environment)
            self.assertTrue(activation._systemd_test)
            with self.assertRaises(ValueError):
                Activation.from_disposable_systemd(Mock())
            environment._lifetime_lock = None
            with self.assertRaises(ValueError):
                Activation.from_disposable_systemd(environment)
        with patch('os.geteuid', return_value=1001):
            with self.assertRaises(ValueError):
                Activation.from_disposable_systemd(self.environment())

    def test_gate_acceptance_occurs_only_after_transaction_returns(self):
        environment = self.environment()
        environment.begin_maintenance = Mock()
        environment.accept = Mock()
        with patch('os.geteuid', return_value=0):
            activation = Activation.from_disposable_systemd(environment)
        activation._verify_systemd_acceptance = Mock()
        events = []
        environment.begin_maintenance.side_effect = lambda: events.append('close')
        environment.accept.side_effect = lambda sha: events.append(('accept', sha))
        activation._activate_locked_transaction = Mock(side_effect=lambda *args, **kwargs: events.append('durable-return') or 'a' * 40)
        activation.activate('a' * 40)
        self.assertEqual(events, ['close', 'durable-return', ('accept', 'a' * 40)])
        environment.accept.reset_mock()
        activation._activate_locked_transaction.side_effect = ValueError('failed')
        with self.assertRaises(ValueError):
            activation.activate('b' * 40)
        environment.accept.assert_not_called()

    def test_acceptance_remains_under_transaction_lock(self):
        from contextlib import contextmanager
        import threading
        environment = self.environment()
        environment.begin_maintenance = Mock()
        environment.accept = Mock()
        mutex = threading.Lock()
        @contextmanager
        def lock():
            if not mutex.acquire(blocking=False):
                raise BlockingIOError('transaction owned')
            try:
                yield
            finally:
                mutex.release()
        environment.lock = lock
        with patch('os.geteuid', return_value=0):
            activation = Activation.from_disposable_systemd(environment)
        verifying = threading.Event()
        finish = threading.Event()
        errors = []
        activation._activate_locked_transaction = Mock(return_value='a' * 40)
        def verify(sha):
            self.assertTrue(mutex.locked())
            verifying.set()
            if not finish.wait(2):
                raise AssertionError('test timeout')
        activation._verify_systemd_acceptance = verify
        environment.accept.side_effect = lambda sha: self.assertTrue(mutex.locked())
        def first():
            try:
                activation.activate('a' * 40)
            except BaseException as error:
                errors.append(error)
        thread = threading.Thread(target=first)
        thread.start()
        try:
            self.assertTrue(verifying.wait(2))
            with self.assertRaises(BlockingIOError):
                activation.activate('b' * 40)
            self.assertEqual(environment.begin_maintenance.call_count, 1)
        finally:
            finish.set()
            thread.join(2)
        self.assertFalse(thread.is_alive())
        self.assertEqual(errors, [])
        environment.accept.assert_called_once_with('a' * 40)

    def test_candidate_fifo_reader_rejects_without_waiting(self):
        from updater_environment import regular_bytes
        with tempfile.TemporaryDirectory() as temporary:
            fifo = Path(temporary) / 'state'
            os.mkfifo(fifo)
            with self.assertRaises(ValueError):
                regular_bytes(fifo, 4096)

    def test_candidate_scratch_symlink_is_not_read(self):
        from contextlib import contextmanager
        import struct
        activation = Activation.__new__(Activation)
        activation.check_integrity = Mock(return_value=Path('/fixed/release'))
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            victim = directory / 'victim'
            victim.write_bytes(b'CRITTER_DEMO 1\nrevision 0\nlast_id -\n')
            @contextmanager
            def scratch():
                yield directory
            activation.environment = Mock()
            activation.environment.scratch = scratch
            def native(release, save, *arguments):
                if arguments[0] == 'status':
                    return b'{"revision":0}'
                sizes = {'lab': (1024, 600), 'probe': (122, 250), 'companion': (368, 448)}
                if arguments[1] == 'companion':
                    save.unlink()
                    save.symlink_to(victim)
                return b'BM' + b'\0' * 16 + struct.pack('<ii', *sizes[arguments[1]])
            activation.native = native
            with self.assertRaises(OSError):
                activation.compatibility('a' * 40, victim.read_bytes())

    def test_root_admission_modes_ignore_restrictive_umask(self):
        import hashlib
        import io
        import tarfile
        import package_staging
        activation = Activation.__new__(Activation)
        activation._systemd_test = True
        activation.environment = Mock()
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            activation.control = root / 'control'
            activation.releases = root / 'releases'
            activation.control.mkdir(mode=0o700)
            activation.releases.mkdir(mode=0o700)
            archive = root / 'fixture.tar.gz'
            with tarfile.open(archive, 'w:gz') as output:
                for index in range(7):
                    name = 'bin/critter_lab' if index == 0 else 'presenter/file' + str(index)
                    member = tarfile.TarInfo(package_staging.ROOT_NAME + '/' + name)
                    member.size = 1
                    member.mode = 0o755 if index == 0 else 0o644
                    output.addfile(member, io.BytesIO(b'x'))
            def ready(candidate):
                self.assertEqual(candidate.stat().st_mode & 0o777, 0o755)
                self.assertEqual((candidate / 'bin').stat().st_mode & 0o777, 0o755)
                self.assertEqual((candidate / 'presenter').stat().st_mode & 0o777, 0o755)
                self.assertEqual((candidate / 'original.tar.gz').stat().st_mode & 0o777, 0o644)
                self.assertEqual((candidate / 'bin/critter_lab').stat().st_mode & 0o777, 0o755)
                self.assertEqual((candidate / 'presenter/file1').stat().st_mode & 0o777, 0o644)
                self.assertEqual(activation.control.stat().st_mode & 0o777, 0o700)
                self.assertEqual(activation.releases.stat().st_mode & 0o777, 0o700)
            activation.environment.admission_ready.side_effect = ready
            previous = os.umask(0o077)
            try:
                with patch('package_staging.verify'):
                    activation.admit(archive, 'a' * 40, hashlib.sha256(archive.read_bytes()).hexdigest())
            finally:
                os.umask(previous)
            activation.environment.admission_ready.assert_called_once()

    def test_stale_record_temporary_symlink_preserves_target(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            victim = directory / 'victim'
            victim.write_bytes(b'keep')
            (directory / 'record.new').symlink_to(victim)
            write_json(directory / 'record.json', {'schema': 1})
            self.assertEqual(victim.read_bytes(), b'keep')
            self.assertEqual(json.loads((directory / 'record.json').read_bytes()), {'schema': 1})
            self.assertEqual((directory / 'record.json').stat().st_mode & 0o777, 0o644)


if __name__ == '__main__':
    unittest.main()
