"""Disposable preparation fixtures; no host or service changes."""
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch
import uuid
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import deployment_worker as worker
from test_delivery import ReleaseFixture, SHA


class Portable(unittest.TestCase):
    def test_non_linux_refused(self):
        with patch.object(worker.os, 'name', 'nt'):
            with self.assertRaises(ValueError):
                worker.prepare('/queue', '/output')


@unittest.skipUnless(os.name == 'posix' and os.geteuid() != 0, 'unprivileged Linux required')
class Preparation(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.queue = Path(self.temporary.name) / 'queue'
        self.output = Path(self.temporary.name) / 'output'
        for path in (self.queue, self.output):
            path.mkdir(mode=0o700)
            (path / '.disposable').write_bytes(b'critter-lab-disposable\n')
        self.name = SHA + '.json'
        self.record = dict(schema=1, sha=SHA, release_id=30, delivery=str(uuid.uuid4()))
        (self.queue / self.name).write_text(json.dumps(self.record))
        self.client = ReleaseFixture()

    def run_worker(self):
        return worker.prepare(self.queue, self.output, self.client)[self.name]

    def test_restart_and_semantic_dedup(self):
        self.assertEqual(self.run_worker(), 'prepared')
        self.record['delivery'] = str(uuid.uuid4())
        (self.queue / self.name).write_text(json.dumps(self.record))
        with patch.object(worker, 'verified_release', side_effect=AssertionError('redownload')):
            self.assertEqual(self.run_worker(), 'duplicate')
        receipt = json.loads((self.output / (SHA + '.prepared.json')).read_text())
        self.assertEqual(receipt['bundle_sha256'], hashlib.sha256(self.client.data[1]).hexdigest())

    def test_corrupt_and_symlink_queue(self):
        path = self.queue / self.name
        path.write_text('{}')
        self.assertEqual(self.run_worker(), 'invalid')
        path.unlink()
        path.symlink_to(self.output / '.disposable')
        self.assertEqual(self.run_worker(), 'invalid')
        self.assertFalse((self.output / (SHA + '.prepared.json')).exists())

    def test_digest_failure_bounded_retry(self):
        self.client.data[1] += b'corruption'
        self.assertEqual(self.run_worker(), 'retry')
        self.assertEqual(self.run_worker(), 'retry')
        self.assertEqual(self.run_worker(), 'exhausted')
        with patch.object(worker, 'verified_release', side_effect=AssertionError('retry exhausted')):
            self.assertEqual(self.run_worker(), 'exhausted')
        self.assertFalse((self.output / (SHA + '.prepared.json')).exists())
        self.assertFalse((self.output / (SHA + '.tar.gz')).exists())

    def test_receipt_is_atomic_readiness_commit(self):
        original = worker.write
        def fail_receipt(directory, name, data):
            if name.endswith('.prepared.json'):
                raise OSError('fixture crash before readiness')
            return original(directory, name, data)
        with patch.object(worker, 'write', side_effect=fail_receipt):
            self.assertEqual(self.run_worker(), 'invalid')
        self.assertFalse((self.output / (SHA + '.prepared.json')).exists())
        self.assertEqual(self.run_worker(), 'prepared')
        self.assertFalse(list(self.output.glob('.pending-*')))


    def test_damaged_receipts_rejected(self):
        self.assertEqual(self.run_worker(), 'prepared')
        path = self.output / (SHA + '.prepared.json')
        original = json.loads(path.read_text())
        changes = [
            ('schema', True), ('release_id', True), ('bundle_sha256', 'invalid'),
            ('delivery', {}), ('extra', 'unexpected'),
        ]
        for field, value in changes:
            with self.subTest(field=field):
                damaged = json.loads(json.dumps(original))
                damaged[field] = value
                path.write_text(json.dumps(damaged))
                self.assertEqual(self.run_worker(), 'invalid')
        for field, value in [('schema', True), ('sha', '0' * 40),
                             ('repository', 'other/repo'), ('workflow', 'other.yml'),
                             ('bundle_sha256', '0' * 64), ('run_id', True), ('attempt', 0)]:
            with self.subTest(delivery_field=field):
                damaged = json.loads(json.dumps(original))
                damaged['delivery'][field] = value
                path.write_text(json.dumps(damaged))
                self.assertEqual(self.run_worker(), 'invalid')
        damaged = json.loads(json.dumps(original))
        del damaged['delivery']
        path.write_text(json.dumps(damaged))
        self.assertEqual(self.run_worker(), 'invalid')

    def test_held_flock_excludes_worker(self):
        import fcntl
        with (self.output / '.prepare.lock').open('w') as lock:
            fcntl.flock(lock.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
            with patch.object(worker, 'verified_release', side_effect=AssertionError('lock bypass')):
                with self.assertRaises(BlockingIOError):
                    self.run_worker()
            self.assertFalse((self.output / (SHA + '.prepared.json')).exists())
        self.assertEqual(self.run_worker(), 'prepared')

if __name__ == '__main__':
    unittest.main()

