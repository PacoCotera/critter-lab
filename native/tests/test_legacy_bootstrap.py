"""Authored legacy-fixture integrity/normalization tests; no native launches."""
from contextlib import nullcontext
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from updater_activation import Activation
from updater_systemd import DisposableSystemdEnvironment
from rehearse_legacy_bootstrap import bootstrap_fixture, INPUTS, LEGACY_SHA, historical_metadata, reset_unaccepted_fixture
import package_staging


class LegacyBootstrapTests(unittest.TestCase):
    def fixture(self, root):
        legacy = root / 'legacy-fixture'
        legacy.mkdir()
        (legacy / 'presenter').mkdir()
        (legacy / '.local-legacy-fixture').write_bytes(b'critter-lab-local-legacy-fixture\n')
        history = dict(commit=LEGACY_SHA, subject='Authored synthetic legacy test fixture',
                       committed_at='2025-01-01T00:00:00+00:00', deployed_at='2025-01-02T00:00:00+00:00')
        for name in INPUTS:
            data = json.dumps(history).encode() if name == 'presenter/release.json' else ('authored bytes ' + name).encode()
            (legacy / name).write_bytes(data)
        env = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        env.root = root
        env.control = root / 'control'
        env.releases = root / 'releases'
        env.save = root / 'runtime/state'
        env.control.mkdir()
        env.releases.mkdir()
        env.save.parent.mkdir()
        env.save.write_bytes(b'CRITTER_DEMO 1\nrevision 7\nlast_id authored-retry\nlast_payload review:6\n')
        env._lifetime_lock = 1
        env.begin_maintenance = Mock()
        env.service = Mock()
        env.lock = lambda: nullcontext()
        env.save_lock = lambda: nullcontext()
        activation = Activation.__new__(Activation)
        activation._bind_environment(env)
        activation._systemd_test = True
        captured = {}
        def admit(bundle, sha, expected_digest):
            package_staging.verify(bundle, sha)
            with package_staging.bounded_tar(bundle) as archive:
                captured.update({Path(member.name).relative_to(package_staging.ROOT_NAME).as_posix():
                                 archive.extractfile(member).read() for member in archive})
            release = env.releases / sha
            release.mkdir()
            return release
        activation.admit = Mock(side_effect=admit)
        activation.activate = Mock()
        hashes = {name: hashlib.sha256((legacy / name).read_bytes()).hexdigest() for name in INPUTS}
        supplied = dict(path=str(env.save), sha256=hashlib.sha256(env.save.read_bytes()).hexdigest())
        return legacy, hashes, supplied, activation, captured

    def bootstrap(self, legacy, hashes, supplied, activation):
        with patch('os.geteuid', return_value=0):
            return bootstrap_fixture(legacy, hashes, supplied, activation=activation)

    def test_exact_code_and_history_preserved_with_truthful_origin(self):
        with tempfile.TemporaryDirectory() as temporary:
            legacy, hashes, supplied, activation, normalized = self.fixture(Path(temporary))
            before = activation.save.read_bytes()
            history = (legacy / 'presenter/release.json').read_bytes()
            origin = self.bootstrap(legacy, hashes, supplied, activation)
            self.assertEqual(normalized['bin/critter_lab'], (legacy / 'critter_lab').read_bytes())
            for name in INPUTS:
                if name.startswith('presenter/') and name != 'presenter/release.json':
                    self.assertEqual(normalized[name], (legacy / name).read_bytes())
            self.assertIsNone(json.loads(normalized['presenter/release.json'])['deployed_at'])
            release = activation.releases / LEGACY_SHA
            self.assertEqual((release / 'legacy-release.json').read_bytes(), history)
            self.assertEqual(origin['origin'], 'local-legacy-fixture')
            self.assertEqual(origin['historical_deployed_at'], '2025-01-02T00:00:00+00:00')
            self.assertFalse({'run_id', 'artifact_id', 'published_at'} & set(origin))
            self.assertEqual(activation.save.read_bytes(), before)
            activation.activate.assert_called_once_with(LEGACY_SHA)

    def test_changed_or_unknown_hashes_refuse_before_admission(self):
        for change in ('wrong', 'missing'):
            with tempfile.TemporaryDirectory() as temporary:
                legacy, hashes, supplied, activation, _ = self.fixture(Path(temporary))
                if change == 'wrong':
                    hashes['critter_lab'] = '0' * 64
                else:
                    del hashes['critter_lab']
                with self.assertRaises(ValueError):
                    self.bootstrap(legacy, hashes, supplied, activation)
                activation.admit.assert_not_called()

    def test_symlinked_code_and_parent_refused(self):
        for parent in (False, True):
            with tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                legacy, hashes, supplied, activation, _ = self.fixture(root)
                if parent:
                    (legacy / 'presenter').rename(legacy / 'original-presenter')
                    (legacy / 'presenter').symlink_to(legacy / 'original-presenter', target_is_directory=True)
                else:
                    (legacy / 'critter_lab').rename(legacy / 'original-binary')
                    (legacy / 'critter_lab').symlink_to(legacy / 'original-binary')
                with self.assertRaises((OSError, ValueError)):
                    self.bootstrap(legacy, hashes, supplied, activation)
                activation.admit.assert_not_called()

    def test_missing_metadata_or_changed_save_refused(self):
        for failure in ('metadata', 'save'):
            with tempfile.TemporaryDirectory() as temporary:
                legacy, hashes, supplied, activation, _ = self.fixture(Path(temporary))
                if failure == 'metadata':
                    (legacy / 'presenter/release.json').write_bytes(b'{"commit":"' + LEGACY_SHA.encode() + b'"}')
                    hashes['presenter/release.json'] = hashlib.sha256((legacy / 'presenter/release.json').read_bytes()).hexdigest()
                else:
                    activation.save.write_bytes(b'changed authored state')
                with self.assertRaises(ValueError):
                    self.bootstrap(legacy, hashes, supplied, activation)
                activation.admit.assert_not_called()

    def test_existing_selection_journal_or_baseline_refused(self):
        for name in ('current.json', 'journal.json', 'baseline'):
            with tempfile.TemporaryDirectory() as temporary:
                legacy, hashes, supplied, activation, _ = self.fixture(Path(temporary))
                if name == 'baseline':
                    (activation.releases / LEGACY_SHA).mkdir()
                else:
                    (activation.control / name).write_text('{}')
                with self.assertRaises(ValueError):
                    self.bootstrap(legacy, hashes, supplied, activation)
                activation.admit.assert_not_called()
                activation.activate.assert_not_called()

    def test_corrupt_reset_origin_preserves_selection_journal_and_baseline(self):
        with tempfile.TemporaryDirectory() as temporary:
            legacy, hashes, supplied, activation, _ = self.fixture(Path(temporary))
            environment = activation.environment
            release = activation.releases / LEGACY_SHA
            release.mkdir()
            (release / 'bootstrap-origin.json').write_bytes(b'{corrupt')
            environment.release_path = Mock(return_value=release)
            pointer = json.dumps({'sha': LEGACY_SHA}).encode()
            journal = json.dumps(dict(phase='switched', old=None, candidate=LEGACY_SHA,
                                      save_digest=supplied['sha256'])).encode()
            activation.pointer.write_bytes(pointer)
            activation.journal.write_bytes(journal)
            with patch('os.geteuid', return_value=0), patch(
                'rehearse_legacy_bootstrap.DisposableSystemdEnvironment.reopen_disposable_test',
                return_value=nullcontext(environment)), patch('rehearse_legacy_bootstrap.protected', side_effect=lambda path: path):
                with self.assertRaises(ValueError):
                    reset_unaccepted_fixture({}, supplied['sha256'])
            self.assertEqual(activation.pointer.read_bytes(), pointer)
            self.assertEqual(activation.journal.read_bytes(), journal)
            self.assertEqual((release / 'bootstrap-origin.json').read_bytes(), b'{corrupt')
            self.assertEqual(hashlib.sha256(activation.save.read_bytes()).hexdigest(), supplied['sha256'])

    def test_existing_transaction_rejects_incompatible_supplied_schema(self):
        with tempfile.TemporaryDirectory() as temporary:
            legacy, hashes, supplied, activation, _ = self.fixture(Path(temporary))
            before = b'CRITTER_DEMO 99\nrevision 7\nlast_id authored-retry\n'
            activation.save.write_bytes(before)  # Author test input before bootstrap.
            supplied['sha256'] = hashlib.sha256(before).hexdigest()
            release = activation.releases / LEGACY_SHA
            activation.check_integrity = Mock(return_value=release)
            activation.selected = Mock(return_value=None)
            activation.release = Mock(return_value=release)
            activation.native = Mock()
            activation.activate.side_effect = lambda sha: Activation._activate_locked_transaction(activation, sha)
            with self.assertRaisesRegex(ValueError, 'save schema'):
                self.bootstrap(legacy, hashes, supplied, activation)
            self.assertEqual(activation.save.read_bytes(), before)
            self.assertFalse(activation.pointer.exists())
            self.assertFalse(activation.journal.exists())
            activation.native.assert_not_called()

    def test_initial_compatibility_or_health_failure_preserves_supplied_save(self):
        for failure in ('incompatible supplied state', 'failed initial health'):
            with tempfile.TemporaryDirectory() as temporary:
                legacy, hashes, supplied, activation, _ = self.fixture(Path(temporary))
                before = activation.save.read_bytes()
                activation.activate.side_effect = ValueError(failure)
                with self.assertRaises(ValueError):
                    self.bootstrap(legacy, hashes, supplied, activation)
                self.assertEqual(activation.save.read_bytes(), before)
                self.assertFalse(activation.pointer.exists())
                self.assertFalse((activation.releases / LEGACY_SHA / 'activation.json').exists())
                activation.environment.begin_maintenance.assert_called_once()


if __name__ == '__main__':
    unittest.main()
