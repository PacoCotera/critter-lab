"""Strict disposable namespace provenance checks; no systemd unit launches."""
import copy
import hashlib
import json
from pathlib import Path
import sys
import unittest
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from updater_systemd import DisposableSystemdEnvironment, SystemdService, PREFIX


class RestartProvenanceTests(unittest.TestCase):
    def environment(self):
        env = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        env.root = Path('/var/lib/' + PREFIX + 'fixture')
        env.control = env.root / 'control'
        env.releases = env.root / 'releases'
        env.runtime = env.root / 'runtime'
        env.config = dict(root=str(env.root), runtime_uid=1001, runtime_gid=1001,
                          check_uid=1002, check_gid=1002, port=24080, health_port=24081,
                          timeout=2, disposable_systemd=True)
        env.namespace_id = '1' * 32
        env._lifetime_lock = 1
        env.service = SystemdService(env)
        env.release_path = lambda release: Path(release)
        env._validate_fragment = Mock()
        return env

    def fixture(self):
        env = self.environment()
        release = env.releases / ('a' * 40)
        command = env.service._launch_command(release)
        intent = dict(schema=1, namespace_id=env.namespace_id, role='runtime', sha=release.name,
                      command=command, scratch=None, arguments=[])
        props = dict(Transient=True, FragmentPath='/run/systemd/transient/' + PREFIX + 'runtime.service', DropInPaths=[], Description=PREFIX + env.namespace_id,
                     User='1001', Group='1001', KillMode='control-group', NoNewPrivileges=True,
                     ProtectSystem='strict', ProtectHome='yes', PrivateTmp=True, RestrictSUIDSGID=True,
                     UMask=0o077, MemoryMax=256 * 1024 * 1024, LimitFSIZE=8388608, LimitCPU=10,
                     ReadWritePaths=['/run/critter-lab-updater-test-state'],
                     Environment=[item.removeprefix('--setenv=') for item in command if item.startswith('--setenv=')],
                     ExecStart=[['/usr/bin/python3', command[command.index('/usr/bin/python3'):], False, 0, 0]],
                     BindPaths=[[str(env.runtime), '/run/critter-lab-updater-test-state', False, 16384]],
                     BindReadOnlyPaths=[[str(release), '/run/critter-lab-updater-test-release', False, 16384]],
                     ControlGroup='/system.slice/' + PREFIX + 'runtime.service', RuntimeMaxUSec=18446744073709551615)
        props.update(EnvironmentFiles=[], PassEnvironment=[], UnsetEnvironment=[], ExecStartPre=[],
                     ExecStartPost=[], ExecStop=[], ExecStopPost=[], ExecCondition=[], RootDirectory='',
                     RootImage='', DynamicUser=False, AmbientCapabilities=0, Type='exec', RemainAfterExit=False)
        return env, intent, props

    def test_fragment_path_requires_exact_fixed_unit(self):
        env = self.environment()
        with self.assertRaisesRegex(ValueError, 'FragmentPath'):
            DisposableSystemdEnvironment._validate_fragment(env, PREFIX + 'runtime.service',
                {'FragmentPath': '/run/systemd/transient/foreign.service'})

    def test_exact_provenance_accepted(self):
        env, intent, props = self.fixture()
        env._validate_unit(PREFIX + 'runtime.service', intent, props)

    def test_unit_property_mismatches_refused(self):
        for property, value in (('Transient', False), ('FragmentPath', '/etc/systemd/system/foreign.service'),
                                ('DropInPaths', ['/etc/systemd/system/override.conf']),
                                ('EnvironmentFiles', [['/private/environment', False]]),
                                ('ExecStartPre', [['/bin/sh', ['/bin/sh'], False]]),
                                ('Description', PREFIX + 'wrong'), ('User', '0'),
                                ('Group', '0'), ('NoNewPrivileges', False),
                                ('Environment', ['TOKEN=private']), ('ControlGroup', '/system.slice/other.service'),
                                ('ExecStart', [['/bin/sh', ['/bin/sh'], False]]),
                                ('BindPaths', [['/owner/save', '/run/critter-lab-updater-test-state', False, 16384]])):
            env, intent, props = self.fixture()
            props[property] = value
            with self.assertRaisesRegex(ValueError, property):
                env._validate_unit(PREFIX + 'runtime.service', intent, props)

    def test_intent_namespace_and_command_mismatches_refused(self):
        env, intent, props = self.fixture()
        intent['namespace_id'] = '2' * 32
        with self.assertRaises(ValueError):
            env._validate_unit(PREFIX + 'runtime.service', intent, props)
        env, intent, props = self.fixture()
        intent['command'][-1] = '/owner/code.py'
        with self.assertRaisesRegex(ValueError, 'command intent'):
            env._validate_unit(PREFIX + 'runtime.service', intent, props)

    def test_namespace_requires_exact_protected_config_record(self):
        env = self.environment()
        expected = dict(schema=1, root=str(env.root),
                        config_sha256=hashlib.sha256(json.dumps(env.config, sort_keys=True).encode()).hexdigest(),
                        units=[PREFIX + role + '.service' for role in ('runtime', 'health', 'check')],
                        namespace_id=env.namespace_id)
        env._read_control = Mock(return_value=expected)
        env._namespace(True)
        changed = dict(expected, config_sha256='0' * 64)
        env._read_control.return_value = changed
        with self.assertRaises(ValueError):
            env._namespace(True)
        env._read_control.side_effect = FileNotFoundError()
        with self.assertRaises(FileNotFoundError):
            env._namespace(True)

    def test_all_loaded_units_validate_before_any_stop(self):
        env = self.environment()
        env.command = Mock(return_value=type('Result', (), {'stdout': b'loaded'})())
        env._read_control = Mock(return_value={})
        env._unit_properties = Mock(return_value={})
        env._validate_unit = Mock(side_effect=[None, ValueError('foreign health unit')])
        env.service._drain = Mock()
        with self.assertRaises(ValueError):
            env._reclaim_units()
        env.service._drain.assert_not_called()

    def test_reopen_closes_gate_before_records_or_units(self):
        env = self.environment()
        order = []
        env.begin_maintenance = lambda: order.append('durable-close')
        env._namespace = lambda reopening: order.append('protected-record')
        env._reclaim_units = lambda: order.append('validate-and-drain')
        env._finish_claim(True)
        self.assertEqual(order, ['durable-close', 'protected-record', 'validate-and-drain'])

    def test_symlinked_namespace_record_refused(self):
        import tempfile
        env = self.environment()
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            target = directory / 'target'
            target.write_text('{}')
            link = directory / 'namespace.json'
            link.symlink_to(target)
            with self.assertRaises(ValueError):
                env._read_control(link)

    def test_corrupt_transaction_journal_does_not_accept(self):
        import tempfile
        from contextlib import nullcontext
        from updater_activation import Activation
        env = self.environment()
        env.lock = lambda: nullcontext()
        env.begin_maintenance = Mock()
        env.accept = Mock()
        env.service.stop = Mock()
        with tempfile.TemporaryDirectory() as temporary:
            env.control = Path(temporary)
            env.save = Path(temporary) / 'state'
            (env.control / 'journal.json').write_text('{broken')
            with patch('os.geteuid', return_value=0):
                activation = Activation.from_disposable_systemd(env)
            with self.assertRaises(ValueError):
                activation.recover()
        env.begin_maintenance.assert_called_once()
        env.accept.assert_not_called()

    def test_missing_units_permitted_without_intent_adoption(self):
        env = self.environment()
        env.command = Mock(return_value=type('Result', (), {'stdout': b'not-found'})())
        env._read_control = Mock()
        env.service._drain = Mock()
        env._reclaim_units()
        env._read_control.assert_not_called()
        env.service._drain.assert_not_called()


if __name__ == '__main__':
    unittest.main()
