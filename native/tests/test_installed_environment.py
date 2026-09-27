"""Installed-layout guards and fixed policy checks; no service launches."""
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
from updater_systemd import (InstalledSystemdEnvironment, DisposableSystemdEnvironment,
                            SystemdService, INSTALLED_POLICY, LOCAL_INSTALLED_POLICY,
                            INSTALL_PROFILES, TOOL_FILES)
import activation_manager


class InstalledEnvironmentTests(unittest.TestCase):
    def config(self):
        return dict(schema=1, profile='local-installed-test', save=INSTALL_PROFILES['local-installed-test']['save'],
                    runtime_uid=1001, runtime_gid=1001, check_uid=1002, check_gid=1002,
                    port=24080, health_port=24081, timeout=3,
                    tools_sha256={name: 'a' * 64 for name in TOOL_FILES},
                    unit_sha256={name: 'b' * 64 for name in INSTALL_PROFILES['local-installed-test']['permanent_units']})

    def test_config_path_and_nonroot_refused(self):
        with patch('os.geteuid', return_value=0):
            with self.assertRaises(ValueError):
                InstalledSystemdEnvironment('/tmp/arbitrary.json')
        with patch('os.geteuid', return_value=1001):
            with self.assertRaises(ValueError):
                InstalledSystemdEnvironment(INSTALL_PROFILES['installed']['config'])

    def test_bad_identities_and_extra_config_fields_refused(self):
        for field, value in (('runtime_uid', 0), ('runtime_gid', True), ('check_uid', 1001), ('unit_name', 'foreign.service')):
            config = self.config()
            config[field] = value
            with patch('os.geteuid', return_value=0), patch('updater_systemd.protected', side_effect=lambda path, *args: Path(path)), patch('updater_systemd.regular_bytes', return_value=json.dumps(config).encode()):
                with self.assertRaises(ValueError):
                    InstalledSystemdEnvironment(INSTALL_PROFILES['local-installed-test']['config'])

    def test_fixed_policy_has_no_arbitrary_unit_or_runtime_path(self):
        environment = InstalledSystemdEnvironment.__new__(InstalledSystemdEnvironment)
        environment.policy = LOCAL_INSTALLED_POLICY
        environment.config = self.config()
        environment.runtime = Path('/var/lib/critter-lab-installed-test-owner')
        environment.save = environment.runtime / 'state'
        environment.namespace_id = '1' * 32
        service = SystemdService(environment)
        command = service._launch_command(Path('/opt/critter-lab-installed-test/releases/' + 'a' * 40))
        self.assertIn('--unit=critter-lab-installed-test-managed-runtime.service', command)
        self.assertIn('--setenv=CRITTER_DEMO_BIND=127.0.0.1', command)
        self.assertIn('--setenv=CRITTER_DEMO_SAVE=' + LOCAL_INSTALLED_POLICY.state_mount + '/state', command)
        self.assertEqual(INSTALLED_POLICY.unit('runtime'), 'critterlab-native-managed-runtime.service')
        with self.assertRaises(ValueError):
            environment.policy.unit('owner-service')

    def save_fixture(self, root):
        environment = InstalledSystemdEnvironment.__new__(InstalledSystemdEnvironment)
        environment.control, environment.releases, environment.tools = root / 'control', root / 'releases', root / 'tools'
        environment.config = {'runtime_uid': os.getuid(), 'runtime_gid': os.getgid()}
        directory = root / 'external-owner'
        directory.mkdir(mode=0o700)
        directory.chmod(0o700)
        environment.save = directory / 'state'
        environment.save.write_bytes(b'explicit already-present fixture bytes')
        environment.save.chmod(0o600)
        return environment

    def test_external_save_is_validated_without_creation_or_write(self):
        with tempfile.TemporaryDirectory() as temporary:
            environment = self.save_fixture(Path(temporary))
            before = environment.save.read_bytes()
            with patch('updater_systemd.protected', side_effect=lambda path, *args: Path(path)):
                environment._validate_existing_save()
            self.assertEqual(environment.save.read_bytes(), before)
            environment.save.unlink()
            with patch('updater_systemd.protected', side_effect=lambda path, *args: Path(path)):
                with self.assertRaises(FileNotFoundError):
                    environment._validate_existing_save()
            self.assertFalse(environment.save.exists())

    def test_preopen_gate_closes_before_missing_save_validation(self):
        import fcntl
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            environment = self.save_fixture(root)
            environment.profile = 'installed'
            environment.gateway = root / 'gateway'
            environment.gateway.mkdir()
            (environment.gateway / 'gate.json').write_text(json.dumps(
                {'schema': 1, 'approved': True, 'accepted_sha': 'a' * 40}))
            descriptor = os.open(environment.gateway / 'environment.lock', os.O_CREAT | os.O_RDWR, 0o600)
            fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            environment._lifetime_lock = descriptor
            environment.save.unlink()
            actual_fstat = os.fstat
            def root_info(open_descriptor):
                info = actual_fstat(open_descriptor)
                return type('Info', (), {'st_mode': info.st_mode, 'st_uid': 0})()
            with patch('updater_systemd.protected', side_effect=lambda path, *args: Path(path)), patch('os.fstat', side_effect=root_info):
                with self.assertRaises(FileNotFoundError):
                    environment._finish_installed_claim({}, False)
            record = json.loads((environment.gateway / 'gate.json').read_bytes())
            self.assertFalse(record['approved'])
            self.assertIsNone(record['accepted_sha'])
            self.assertIsNone(environment._lifetime_lock)
            self.assertFalse(environment.save.exists())

    def test_save_symlink_modes_and_trusted_tree_paths_refused(self):
        for failure in ('symlink', 'mode', 'control'):
            with tempfile.TemporaryDirectory() as temporary:
                environment = self.save_fixture(Path(temporary))
                if failure == 'symlink':
                    target = environment.save.with_name('target')
                    environment.save.rename(target)
                    environment.save.symlink_to(target)
                elif failure == 'mode':
                    environment.save.chmod(0o644)
                else:
                    environment.control = environment.save.parent
                with patch('updater_systemd.protected', side_effect=lambda path, *args: Path(path)):
                    with self.assertRaises(ValueError):
                        environment._validate_existing_save()

    def test_boot_id_change_preserves_namespace_identity(self):
        environment = InstalledSystemdEnvironment.__new__(InstalledSystemdEnvironment)
        environment.profile = 'local-installed-test'
        environment.policy = LOCAL_INSTALLED_POLICY
        environment.root = environment.control = Path('/var/lib/critter-lab-installed-test/control')
        environment.config = self.config()
        environment.namespace_id = 'a' * 32
        environment._current_boot_id = '11111111-1111-1111-1111-111111111111'
        record = environment._installed_record()
        record['boot_id'] = '22222222-2222-2222-2222-222222222222'
        environment._read_control = Mock(return_value=record)
        environment._namespace(True)
        self.assertEqual(environment.namespace_id, 'a' * 32)
        environment._atomic_control = Mock()
        environment._write_installed_namespace()
        self.assertEqual(environment._atomic_control.call_args.args[1]['boot_id'], environment._current_boot_id)

    def test_permanent_units_are_not_transient_adoptions(self):
        environment = InstalledSystemdEnvironment.__new__(InstalledSystemdEnvironment)
        environment.command = Mock(return_value=type('Result', (), {'stdout': b'LoadState=loaded\nTransient=yes\nFragmentPath=/etc/systemd/system/critter-lab-installed-test-activation.service\nDropInPaths=\n'})())
        environment._unit_object_path = Mock(return_value='/org/freedesktop/systemd1/unit/fixed')
        environment._bus_property = Mock(side_effect=[True])
        with self.assertRaisesRegex(ValueError, 'permanent Transient'):
            environment._validate_permanent_unit('critter-lab-installed-test-activation.service')

    def test_inactive_static_unit_uses_one_snapshot_without_getunit_race(self):
        environment = InstalledSystemdEnvironment.__new__(InstalledSystemdEnvironment)
        unit = 'critter-lab-installed-test-activation.service'
        snapshot = ('LoadState=loaded\nTransient=no\nFragmentPath=/etc/systemd/system/' + unit + '\nDropInPaths=\n').encode()
        environment.command = Mock(return_value=type('Result', (), {'stdout': snapshot})())
        environment._unit_object_path = Mock(side_effect=ValueError('unit already collected'))
        environment._validate_permanent_unit(unit)
        environment.command.assert_called_once_with(['/usr/bin/systemctl', 'show', unit,
            '--property=LoadState', '--property=Transient', '--property=FragmentPath', '--property=DropInPaths'])
        environment._unit_object_path.assert_not_called()
        for invalid in (snapshot + b'Transient=no\n', snapshot.replace(b'DropInPaths=\n', b''),
                        snapshot.replace(b'DropInPaths=\n', b'DropInPaths=/etc/override.conf\n')):
            environment.command.return_value.stdout = invalid
            with self.assertRaises(ValueError):
                environment._validate_permanent_unit(unit)

    def test_exact_installed_activation_seam(self):
        environment = InstalledSystemdEnvironment.__new__(InstalledSystemdEnvironment)
        environment._lifetime_lock = 1
        environment.tools = Path(__file__).resolve().parents[1]
        environment.root = Path('/fixed/control')
        environment.control = environment.root
        environment.releases = Path('/fixed/releases')
        environment.save = Path('/fixed/external/state')
        environment.service = Mock()
        with patch('os.geteuid', return_value=0):
            self.assertTrue(Activation.from_installed_environment(environment)._systemd_test)
            with self.assertRaises(ValueError):
                Activation.from_environment(environment)
            with self.assertRaises(ValueError):
                Activation.from_installed_environment(DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment))
            environment._lifetime_lock = None
            with self.assertRaises(ValueError):
                Activation.from_installed_environment(environment)

    def test_unsupported_manager_operation_closes_gate(self):
        environment = Mock()
        context = Mock()
        context.__enter__ = Mock(return_value=environment)
        context.__exit__ = Mock(return_value=False)
        with patch('sys.argv', ['activation_manager.py', 'apply-request', '--local-test-profile']), patch(
                'activation_manager.__file__', INSTALL_PROFILES['local-installed-test']['tools'] + '/activation_manager.py'), patch(
                'activation_manager.InstalledSystemdEnvironment', return_value=context):
            with self.assertRaises(NotImplementedError):
                activation_manager.main()
        environment.begin_maintenance.assert_called_once()


if __name__ == '__main__':
    unittest.main()
