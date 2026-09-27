"""Explicit fresh installed-layout LOCAL TEST preparation/recovery only.

Writes only compiled test-profile paths after refusing their existence. Never
run against an owner save. Source save must be in a marked disposable fixture.
Does not install/enable permanent units or exercise requests/captured bootstrap.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from updater_environment import regular_bytes
from updater_systemd import INSTALL_PROFILES, LOCAL_INSTALLED_POLICY, PREFIX, TOOL_FILES, protected
import package_staging

LAYOUT = INSTALL_PROFILES['local-installed-test']


def fresh_file(path, data, mode=0o644):
    descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW | os.O_WRONLY, mode)
    try:
        os.fchmod(descriptor, mode)
        with os.fdopen(descriptor, 'wb', closefd=False) as output:
            output.write(data)
            output.flush()
            os.fsync(descriptor)
    finally:
        os.close(descriptor)


def trusted_helper(code, *arguments):
    fixed_import = "import sys; sys.path.insert(0, '/usr/local/lib/critter-lab-installed-test'); "
    return subprocess.run(['/usr/bin/python3', '-c', fixed_import + code, *map(str, arguments)], check=True, timeout=120)


def manager():
    return subprocess.run(['/usr/bin/python3', LAYOUT['tools'] + '/activation_manager.py',
                           'recover', '--local-test-profile'], check=True, timeout=120)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bundle')
    parser.add_argument('--save-source', required=True)
    parser.add_argument('--prepare-local-test', action='store_true', required=True)
    for identity in ('runtime-uid', 'runtime-gid', 'check-uid', 'check-gid'):
        parser.add_argument('--' + identity, required=True, type=int)
    args = parser.parse_args()
    if os.geteuid() != 0 or min(args.runtime_uid, args.runtime_gid, args.check_uid, args.check_gid) <= 0 or args.runtime_uid == args.check_uid:
        raise ValueError('root explicit local test and separate nonzero identities required')
    source = Path(args.save_source)
    fixture_root = source.parent.parent
    if (not source.is_absolute() or source.resolve() != source or source.name != 'state'
            or source.parent.name != 'runtime' or not fixture_root.name.startswith(PREFIX)
            or regular_bytes(fixture_root / '.disposable-systemd', 100) != b'critter-lab-disposable-systemd\n'):
        raise ValueError('save source must be existing marked disposable fixture state')
    save_bytes = regular_bytes(source, 4096)
    original = Path(args.bundle).resolve()
    bundle_bytes = regular_bytes(original)
    with package_staging.bounded_tar(original) as archive:
        sha = json.load(archive.extractfile(package_staging.ROOT_NAME + '/presenter/release.json'))['commit']
    package_staging.verify(original, sha)
    paths = [Path('/etc/critter-lab-installed-test'), Path(LAYOUT['tools']),
             Path('/opt/critter-lab-installed-test'), Path('/var/lib/critter-lab-installed-test'),
             Path(LAYOUT['save']).parent, Path(LOCAL_INSTALLED_POLICY.gateway)]
    fragments = [Path('/etc/systemd/system') / name for name in LAYOUT['permanent_units']]
    if any(path.exists() or path.is_symlink() for path in paths + fragments):
        raise ValueError('refusing existing installed-layout test paths/fragments')
    for role in ('runtime', 'health', 'check'):
        unit = LOCAL_INSTALLED_POLICY.unit(role)
        state = subprocess.check_output(['/usr/bin/systemctl', 'show', unit, '--property=LoadState', '--value']).strip()
        if state != b'not-found':
            raise ValueError('refusing existing test runtime namespace')
    for path in paths[:-1]:
        protected(path.parent, True)
        path.mkdir(mode=0o755 if str(path) in (LAYOUT['tools'], '/etc/critter-lab-installed-test', '/opt/critter-lab-installed-test') else 0o700)
    tools = Path(LAYOUT['tools'])
    tools.chmod(0o755)
    control = Path(LAYOUT['control'])
    control.mkdir(mode=0o700)
    (control / 'checks').mkdir(mode=0o700)
    releases = Path(LAYOUT['releases'])
    releases.mkdir(mode=0o700)
    fresh_file(control / '.installed-layout-disposable', b'critter-lab-installed-layout-disposable\n', 0o600)
    save = Path(LAYOUT['save'])
    os.chown(save.parent, args.runtime_uid, args.runtime_gid)
    save.parent.chmod(0o700)
    fresh_file(save, save_bytes, 0o600)
    os.chown(save, args.runtime_uid, args.runtime_gid)
    source_tools = Path(__file__).resolve().parents[1]
    for name in TOOL_FILES:
        fresh_file(tools / name, regular_bytes(source_tools / name, 2 * 1024 * 1024))
    runtime_port, health_port, gateway_port = 24080, 24081, 24082
    # Caller must ensure these isolated loopback ports are available; no production ports.
    manager_template = regular_bytes(source_tools / 'deployment/critterlab-native-activation.service', 65536).decode()
    manager_template = manager_template.replace('critterlab-native-gateway.service', LAYOUT['permanent_units'][1]).replace(
        '/usr/local/lib/critterlab-native-updater', LAYOUT['tools']).replace('activation_manager.py recover', 'activation_manager.py recover --local-test-profile')
    gateway_template = regular_bytes(source_tools / 'deployment/critterlab-native-gateway.service', 65536).decode()
    for old, new in (
        ('/usr/local/lib/critterlab-native-updater', LAYOUT['tools']),
        ('/run/critterlab-native-gateway', LOCAL_INSTALLED_POLICY.gateway),
        ('REPLACE_GATEWAY_NONZERO_UID', str(args.check_uid)), ('REPLACE_GATEWAY_NONZERO_GID', str(args.check_gid)),
        ('REPLACE_RUNTIME_LOOPBACK_PORT', str(runtime_port)), ('REPLACE_GATEWAY_LOOPBACK_PORT', str(gateway_port)),
        ('REPLACE_PUBLIC_HOST', 'disposable.example.test')):
        gateway_template = gateway_template.replace(old, new)
    for path, data in zip(fragments, (manager_template, gateway_template)):
        fresh_file(path, data.encode())
    config = dict(schema=1, profile='local-installed-test', save=str(save),
        runtime_uid=args.runtime_uid, runtime_gid=args.runtime_gid, check_uid=args.check_uid, check_gid=args.check_gid,
        port=runtime_port, health_port=health_port, timeout=3,
        tools_sha256={name: hashlib.sha256(regular_bytes(tools / name)).hexdigest() for name in TOOL_FILES},
        unit_sha256={path.name: hashlib.sha256(regular_bytes(path, 65536)).hexdigest() for path in fragments})
    fresh_file(Path(LAYOUT['config']), json.dumps(config, sort_keys=True).encode(), 0o600)
    captured_bundle = control / 'fixture-bundle.tar.gz'
    fresh_file(captured_bundle, bundle_bytes, 0o600)
    subprocess.run(['/usr/bin/systemd-analyze', 'verify', *map(str, fragments)], check=True, timeout=30)
    try:
        trusted_helper("from updater_systemd import InstalledSystemdEnvironment; from updater_activation import Activation; "
            "from updater_environment import regular_bytes; import hashlib; from pathlib import Path; "
            "env=InstalledSystemdEnvironment.initialize_local_test('/etc/critter-lab-installed-test/config.json'); "
            "act=Activation.from_installed_environment(env); "
            "act.admit(Path(sys.argv[1]),sys.argv[2],hashlib.sha256(regular_bytes(Path(sys.argv[1]))).hexdigest()); "
            "act.activate(sys.argv[2]); env.service.stop(); env.close()", captured_bundle, sha)
        assert regular_bytes(save, 4096) == save_bytes
        manager()
        assert regular_bytes(save, 4096) == save_bytes
        trusted_helper("from updater_systemd import InstalledSystemdEnvironment; "
            "env=InstalledSystemdEnvironment('/etc/critter-lab-installed-test/config.json'); "
            "env.service.stop(); record=env._installed_record(); "
            "record['boot_id']='00000000-0000-0000-0000-000000000000'; "
            "env._atomic_control(env.control/'namespace.json',record); env.close()")
        deadline = time.monotonic() + 3
        while True:
            states = [subprocess.check_output(['/usr/bin/systemctl', 'show', LOCAL_INSTALLED_POLICY.unit(role),
                       '--property=LoadState', '--value']).strip() for role in ('runtime', 'health', 'check')]
            if states == [b'not-found'] * 3:
                break
            if time.monotonic() >= deadline:
                raise ValueError('owned transient units did not disappear')
            time.sleep(.05)
        gate = protected(LOCAL_INSTALLED_POLICY.gateway, True)
        if {entry.name for entry in gate.iterdir()} != {'gate.json', 'environment.lock'}:
            raise ValueError('unexpected test gate files; refusing removal')
        for name in ('gate.json', 'environment.lock'):
            protected(gate / name).unlink()
        gate.rmdir()
        manager()
        assert regular_bytes(save, 4096) == save_bytes
        print(json.dumps({'local_installed_layout_only': True, 'accepted_sha': sha,
            'external_save_sha256': hashlib.sha256(save_bytes).hexdigest(), 'network_independent_recover': True,
            'run_gate_and_transients_absent_recover': True, 'simulated_boot_id_change': True,
            'real_reboot_tested': False, 'requests_and_captured_bootstrap_implemented': False}, sort_keys=True))
    finally:
        # A helper may die after starting a matched unit but before returning.
        # Reopen validates ALL provenance before draining any loaded unit.
        trusted_helper("from updater_systemd import InstalledSystemdEnvironment; "
            "env=InstalledSystemdEnvironment('/etc/critter-lab-installed-test/config.json'); "
            "env.service.stop(); env.close()")
    # Preserve explicitly marked test paths/fragments and closed gate as evidence.


if __name__ == '__main__':
    main()
