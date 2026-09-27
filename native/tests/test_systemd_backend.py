"""No systemd launches: command construction and fail-closed controls."""
import os
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from updater_systemd import DisposableSystemdEnvironment, SystemdService, PREFIX, protected


class SystemdBackendTests(unittest.TestCase):
    def config(self):
        return dict(root='/root/' + PREFIX + 'fixture', port=24080, health_port=24081,
                    timeout=2, disposable_systemd=True, runtime_uid=1001,
                    runtime_gid=1001, check_uid=1002, check_gid=1002)

    def test_zero_and_boolean_identities_rejected(self):
        for key in ('runtime_uid', 'runtime_gid', 'check_uid', 'check_gid'):
            for value in (0, True, -1):
                config = self.config()
                config[key] = value
                with patch('os.geteuid', return_value=0):
                    with self.assertRaises(ValueError):
                        DisposableSystemdEnvironment(config)

    def test_nonroot_and_extra_keys_rejected(self):
        with patch('os.geteuid', return_value=123):
            with self.assertRaises(ValueError):
                DisposableSystemdEnvironment(self.config())
        config = self.config()
        config['unit'] = 'owner.service'
        with patch('os.geteuid', return_value=0):
            with self.assertRaises(ValueError):
                DisposableSystemdEnvironment(config)

    def test_fixed_sandbox_command(self):
        env = type('Environment', (), {'config': self.config()})()
        service = SystemdService(env)
        command = service._command(service._unit(), 1001, 1001, Path('/root/test/state'), ['/usr/bin/python3', '/run/fixed.py'])
        self.assertEqual(command[0], '/usr/bin/systemd-run')
        for property in ('User=1001', 'Group=1001', 'KillMode=control-group', 'NoNewPrivileges=yes', 'ProtectSystem=strict'):
            self.assertIn('--property=' + property, command)
        self.assertEqual(service._unit(), PREFIX + 'runtime.service')
        self.assertEqual(service._unit(True), PREFIX + 'health.service')

    def test_native_uses_fixed_host_python_exec_shim(self):
        import tempfile
        env = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        env.config = self.config()
        env.config['check_uid'] = os.getuid()
        env._lifetime_lock = 1
        env.release_path = lambda release: Path(release)
        env.service = SystemdService(env)
        with tempfile.TemporaryDirectory() as temporary:
            env.root = Path(temporary)
            scratch = env.root / 'check-fixture'
            scratch.mkdir(mode=0o700)
            with patch('updater_systemd.subprocess.Popen', side_effect=RuntimeError('capture')) as launch, patch.object(env.service, '_drain'):
                with self.assertRaisesRegex(RuntimeError, 'capture'):
                    env.run_native(Path('/fixed/release'), scratch / 'state', 'status')
            command = launch.call_args.args[0]
            shim = command.index('/usr/bin/python3')
            self.assertEqual(command[shim:], ['/usr/bin/python3', '-c',
                'import os,sys; os.execv(sys.argv[1],sys.argv[1:])',
                '/run/critter-lab-updater-test-release/bin/critter_lab', '--save',
                '/run/critter-lab-updater-test-state/state', 'status'])
            self.assertIn('--property=User=' + str(os.getuid()), command)

    def test_malformed_gate_fails_closed(self):
        env = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        env.gateway = Path('/root/test/gateway')
        for data in (b'{}', b'[]', b'null', b'{"schema":1,"approved":true,"accepted_sha":"bad"}', b'bad'):
            with patch('updater_systemd.protected', side_effect=lambda path: path), patch('updater_systemd.regular_bytes', return_value=data):
                self.assertFalse(env.gate_open())

    def test_stop_closes_gate_first(self):
        events = []
        env = type('Environment', (), {'config': self.config(), 'begin_maintenance': lambda self: events.append('closed'), '_require_ownership': lambda self: None})()
        service = SystemdService(env)
        with patch.object(service, '_drain', side_effect=lambda unit: events.append(unit)):
            service.stop()
        self.assertEqual(events, ['closed', PREFIX + 'runtime.service', PREFIX + 'health.service'])

    def test_protected_rejects_writable_and_symlink_paths(self):
        import tempfile
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            directory.chmod(0o777)
            with self.assertRaises(ValueError):
                protected(directory, True)
            link = directory / 'link'
            link.symlink_to(directory)
            with self.assertRaises(ValueError):
                protected(link, True)

    def test_gate_exact_schema(self):
        env = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        env.gateway = Path('/root/test/gateway')
        data = ('{"schema":1,"approved":true,"accepted_sha":"' + 'a' * 40 + '"}').encode()
        with patch('updater_systemd.protected', side_effect=lambda path: path), patch('updater_systemd.regular_bytes', return_value=data):
            self.assertTrue(env.gate_open())

    def test_not_found_unit_is_quiescent(self):
        calls = []
        def command(arguments):
            calls.append(arguments)
            return type('Result', (), {'stdout': b'not-found\n'})()
        env = type('Environment', (), {'config': self.config(), 'command': staticmethod(command)})()
        SystemdService(env)._drain(PREFIX + 'runtime.service')
        self.assertEqual(len(calls), 1)

    def test_unknown_load_state_rejected(self):
        for state in (b'', b'error', b'masked', b'notfound'):
            env = type('Environment', (), {'config': self.config(), 'command': staticmethod(lambda arguments: type('Result', (), {'stdout': state})())})()
            with self.assertRaises(ValueError):
                SystemdService(env)._drain(PREFIX + 'runtime.service')

    def test_loaded_stop_failure_propagates(self):
        import subprocess
        responses = [type('Result', (), {'stdout': b'loaded'})(),
                     type('Result', (), {'stdout': b'/system.slice/' + (PREFIX + 'runtime.service').encode()})(),
                     subprocess.CalledProcessError(1, 'systemctl')]
        env = type('Environment', (), {'config': self.config()})()
        env.command = unittest.mock.Mock(side_effect=responses)
        with self.assertRaises(subprocess.CalledProcessError):
            SystemdService(env)._drain(PREFIX + 'runtime.service')

    def test_foreign_unit_refuses_namespace(self):
        import tempfile
        env = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        env._lifetime_lock = None
        env.command = unittest.mock.Mock(return_value=type('Result', (), {'stdout': b'loaded'})())
        with tempfile.TemporaryDirectory() as temporary:
            env.gateway = Path(temporary)
            with patch('os.fstat', return_value=type('Info', (), {'st_mode': 0o100600, 'st_uid': 0})()):
                with self.assertRaises(ValueError):
                    env._claim_test_namespace()
        self.assertIsNone(env._lifetime_lock)

    def test_second_namespace_instance_refused(self):
        import tempfile
        environments = [DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment) for _ in range(2)]
        with tempfile.TemporaryDirectory() as temporary:
            for env in environments:
                env.gateway = Path(temporary)
                env._lifetime_lock = None
                env.command = unittest.mock.Mock(return_value=type('Result', (), {'stdout': b'not-found'})())
            with patch('os.fstat', return_value=type('Info', (), {'st_mode': 0o100600, 'st_uid': 0})()):
                environments[0]._claim_test_namespace()
                try:
                    with self.assertRaises(BlockingIOError):
                        environments[1]._claim_test_namespace()
                finally:
                    environments[0].close()
            with self.assertRaises(ValueError):
                environments[0]._require_ownership()

    def test_stale_gate_new_symlink_is_not_followed(self):
        import tempfile
        env = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        with tempfile.TemporaryDirectory() as temporary:
            env.gateway = Path(temporary)
            victim = env.gateway / 'victim'
            victim.write_bytes(b'preserve')
            (env.gateway / 'gate.new').symlink_to(victim)
            # Privilege identity is mocked only for this filesystem regression.
            actual_fstat = os.fstat
            def root_info(descriptor):
                info = actual_fstat(descriptor)
                return type('Info', (), {'st_mode': info.st_mode, 'st_uid': 0})()
            with patch('os.fstat', side_effect=root_info):
                env._gate_record(False, None)
            self.assertEqual(victim.read_bytes(), b'preserve')
            self.assertEqual((env.gateway / 'gate.json').stat().st_mode & 0o777, 0o644)

    def test_accept_rejects_non_sha(self):
        env = DisposableSystemdEnvironment.__new__(DisposableSystemdEnvironment)
        for sha in ('abc', '../runtime', None):
            with self.assertRaises(ValueError):
                env.accept(sha)


if __name__ == '__main__':
    unittest.main()
