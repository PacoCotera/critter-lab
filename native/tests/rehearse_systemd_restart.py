"""Fresh-process SIGKILL recovery in an explicit disposable systemd namespace.

Fixture SHAs/wrappers are synthetic, not published releases. No reboot/live support.
"""
import argparse
import base64
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import tarfile
import tempfile
import time
import urllib.error
import urllib.request

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import package_staging
from updater_activation import Activation
from updater_environment import regular_bytes
from updater_systemd import DisposableSystemdEnvironment, PREFIX, protected
from rehearse_updater import fixture_bundle, free_port

PHASES = ('prepared', 'pointer', 'switched', 'receipt', 'accepted', 'journal-removed',
          'gate-opened', 'native-check', 'start-health')


def configuration(root):
    path = protected(root / 'control/harness-config.json')
    return json.loads(regular_bytes(path, 10000))


def child(root, phase, sha):
    config = configuration(root)
    with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
        activation = Activation.from_disposable_systemd(environment)
        activation.recover()
        def kill():
            os.kill(os.getpid(), signal.SIGKILL)
        activation.transition_hook = lambda transition: kill() if transition == phase else None
        if phase == 'journal-removed':
            activation._verify_systemd_acceptance = lambda selected: kill()
        if phase == 'gate-opened':
            accept = environment.accept
            def accepted(selected):
                accept(selected)
                kill()
            environment.accept = accepted
        if phase == 'start-health':
            environment.service.health = lambda *args, **kwargs: kill()
        if phase == 'native-check':
            original_popen = subprocess.Popen
            def launch(command, *args, **kwargs):
                process = original_popen(command, *args, **kwargs)
                if command[0] == '/usr/bin/systemd-run' and '--unit=' + PREFIX + 'check.service' in command:
                    # The synthetic native fixture persists a readiness marker only
                    # after starting a child inside the check cgroup.
                    deadline = time.monotonic() + 5
                    while time.monotonic() < deadline:
                        markers = list(root.glob('check-*/fixture-child-ready'))
                        if markers:
                            kill()
                        time.sleep(.02)
                    raise ValueError('synthetic check descendant did not become ready')
                return process
            subprocess.Popen = launch
        activation.activate(sha)
    raise AssertionError('SIGKILL phase was not reached')


def descendant_fixture(original, target, sha):
    """Embed unchanged trusted ELF in a clearly synthetic nonprivileged wrapper."""
    with package_staging.bounded_tar(original) as source:
        files = {Path(member.name).relative_to(package_staging.ROOT_NAME).as_posix():
                 source.extractfile(member).read() for member in source}
    encoded = base64.b64encode(files['bin/critter_lab']).decode()
    code = ("#!/usr/bin/python3\nimport base64,os,pathlib,subprocess,sys\n"
            "root=pathlib.Path(sys.argv[2]).parent\n"
            "tool=root/'fixture-original-native'\n"
            "tool.write_bytes(base64.b64decode(" + repr(encoded) + "))\n"
            "tool.chmod(0o700)\n"
            "result=subprocess.check_output([str(tool),*sys.argv[1:]])\n"
            "subprocess.Popen(['/usr/bin/sleep','30'])\n"
            "(root/'fixture-child-ready').write_text('synthetic child started\\n')\n"
            "sys.stdout.buffer.write(result)\nsys.stdout.flush()\n"
            "subprocess.run(['/usr/bin/sleep','30'])\n")
    files['bin/critter_lab'] = code.encode()
    metadata = json.loads(files['presenter/release.json'])
    metadata['commit'] = sha
    files['presenter/release.json'] = json.dumps(metadata).encode()
    files['MANIFEST.txt'] = package_staging.manifest({name: files[name] for name in package_staging.PAYLOAD}, metadata)
    from datetime import datetime
    timestamp = int(datetime.fromisoformat(metadata['committed_at']).timestamp())
    with target.open('wb') as raw, gzip.GzipFile(filename='', fileobj=raw, mode='wb', mtime=0) as compressed:
        with tarfile.open(fileobj=compressed, mode='w', format=tarfile.PAX_FORMAT) as archive:
            for name in sorted(files):
                member = tarfile.TarInfo(package_staging.ROOT_NAME + '/' + name)
                member.size, member.mode, member.mtime = len(files[name]), package_staging.MODES.get(name, 0o644), timestamp
                archive.addfile(member, io.BytesIO(files[name]))
    package_staging.verify(target, sha)


def gateway_request(port, route='/api/status', data=None):
    headers = {'Host': 'disposable.example.test', 'Origin': 'https://disposable.example.test',
               'Content-Type': 'application/json', 'X-Requested-With': 'CritterLab'}
    request = urllib.request.Request('http://127.0.0.1:' + str(port) + route, data=data, headers=headers)
    try:
        with urllib.request.urlopen(request, timeout=3) as response:
            return response.status, response.read(2_000_001)
    except urllib.error.HTTPError as error:
        return error.code, error.read(2000)


def play(port, action):
    code, body = gateway_request(port)
    if code != 200:
        raise ValueError('fixture gateway status failed')
    revision = json.loads(body)['revision']
    data = json.dumps(dict(name=action, revision=revision, operation_id='restart-play-' + str(revision))).encode()
    code, _ = gateway_request(port, '/api/command', data)
    if code != 200:
        raise ValueError('fixture gateway play failed')


def initialize(args):
    root = Path(args.root)
    gateway = Path('/run/critter-lab-updater-test-gateway')
    if os.geteuid() != 0 or root.parent != Path('/var/lib') or not root.name.startswith(PREFIX):
        raise ValueError('explicit root disposable fixture required')
    if root.exists() or gateway.exists():
        raise ValueError('refusing existing fixture paths')
    if min(args.runtime_uid, args.runtime_gid, args.check_uid, args.check_gid) <= 0 or args.runtime_uid == args.check_uid:
        raise ValueError('separate nonzero fixture identities required')
    original = Path(args.bundle).resolve()
    with package_staging.bounded_tar(original) as source:
        old = json.load(source.extractfile(package_staging.ROOT_NAME + '/presenter/release.json'))['commit']
    package_staging.verify(original, old)
    root.mkdir(mode=0o700)
    gateway.mkdir(mode=0o755)
    gateway.chmod(0o755)
    (root / '.disposable-systemd').write_bytes(b'critter-lab-disposable-systemd\n')
    for name in ('control', 'releases', 'runtime'):
        (root / name).mkdir(mode=0o700)
    os.chown(root / 'runtime', args.runtime_uid, args.runtime_gid)
    config = dict(root=str(root), port=free_port(), health_port=free_port(), timeout=3,
                  disposable_systemd=True, runtime_uid=args.runtime_uid, runtime_gid=args.runtime_gid,
                  check_uid=args.check_uid, check_gid=args.check_gid)
    with DisposableSystemdEnvironment(config) as environment:
        try:
            environment._atomic_control(root / 'control/harness-config.json', config)
            activation = Activation.from_disposable_systemd(environment)
            second = hashlib.sha256(b'accepted-second-fixture').hexdigest()[:40]
            for sha, bundle in ((old, original), (second, root / 'second-fixture.tar.gz')):
                if sha != old:
                    fixture_bundle(original, bundle, sha)
                activation.admit(bundle, sha, hashlib.sha256(regular_bytes(bundle)).hexdigest())
            with environment.scratch() as scratch:
                state = json.loads(environment.run_native(activation.release(old), scratch / 'state', 'status'))
                reviewed = json.loads(environment.run_native(activation.release(old), scratch / 'state', 'command', 'review', state['revision'], 'restart-seed-review'))
                environment.run_native(activation.release(old), scratch / 'state', 'command', 'back', reviewed['revision'], 'restart-seed-back')
                activation.save.write_bytes(regular_bytes(scratch / 'state', 4096))
            activation.save.chmod(0o600)
            os.chown(activation.save, args.runtime_uid, args.runtime_gid)
            activation.activate(old)
            activation.activate(second)
            activation.activate(old, rollback=True)
            for phase in PHASES:
                sha = hashlib.sha256(('crash-fixture-' + phase).encode()).hexdigest()[:40]
                bundle = root / (phase + '-fixture.tar.gz')
                if phase == 'native-check':
                    descendant_fixture(original, bundle, sha)
                else:
                    fixture_bundle(original, bundle, sha)
                activation.admit(bundle, sha, hashlib.sha256(regular_bytes(bundle)).hexdigest())
            environment.begin_maintenance()
        except BaseException:
            # Failed setup retains evidence but must never leave an accepted
            # fixture exposed. Validate all provenance before touching services.
            environment.begin_maintenance()
            environment._reclaim_units()
            raise
    return root, config, old


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bundle', nargs='?')
    parser.add_argument('--root', required=True)
    parser.add_argument('--runtime-uid', type=int)
    parser.add_argument('--runtime-gid', type=int)
    parser.add_argument('--check-uid', type=int)
    parser.add_argument('--check-gid', type=int)
    parser.add_argument('--child', choices=PHASES)
    parser.add_argument('--sha')
    args = parser.parse_args()
    root = Path(args.root)
    if args.child:
        child(root, args.child, args.sha)
        return
    root, config, old = initialize(args)
    gateway_port = free_port()
    tool_directory = Path(tempfile.mkdtemp(prefix=PREFIX + 'gateway-tool-'))
    tool_directory.chmod(0o755)
    gateway_tool = tool_directory / 'deployment_gateway.py'
    shutil.copyfile(Path(__file__).resolve().parents[1] / 'deployment_gateway.py', gateway_tool)
    gateway_tool.chmod(0o644)
    if gateway_tool.stat().st_uid != 0 or gateway_tool.stat().st_mode & 0o022:
        raise ValueError('trusted gateway fixture tool ownership required')
    gateway = subprocess.Popen(['/usr/bin/python3', str(gateway_tool), '--gate',
        '/run/critter-lab-updater-test-gateway/gate.json', '--origin-port', str(config['port']),
        '--listen-port', str(gateway_port), '--canonical-host', 'disposable.example.test', '--trusted-gate-uid', '0'],
        user=config['check_uid'], group=config['check_gid'], extra_groups=[],
        env={'PATH': '/usr/bin:/bin'}, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    evidence = []
    try:
        deadline = time.monotonic() + 3
        while True:
            try:
                if gateway_request(gateway_port)[0] == 503:
                    break
            except OSError:
                pass
            if time.monotonic() >= deadline:
                raise ValueError('unprivileged gateway fixture did not start closed')
            time.sleep(.05)
        for phase in PHASES:
            sha = hashlib.sha256(('crash-fixture-' + phase).encode()).hexdigest()[:40]
            with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
                assert gateway_request(gateway_port)[0] == 503
                activation = Activation.from_disposable_systemd(environment)
                activation.recover()
                if activation.selected() != old:
                    activation.activate(old, rollback=True)
                play(gateway_port, 'review')
                play(gateway_port, 'back')
                before = regular_bytes(activation.save, 4096)
                receipt = regular_bytes(activation.release(old) / 'activation.json', 10000)
            result = subprocess.run(['/usr/bin/python3', str(Path(__file__).resolve()), '--root', str(root),
                                     '--child', phase, '--sha', sha], timeout=120)
            if result.returncode != -signal.SIGKILL:
                raise ValueError('fixture coordinator was not killed at ' + phase)
            with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
                assert gateway_request(gateway_port)[0] == 503
                activation = Activation.from_disposable_systemd(environment)
                observed_journal = activation.read_record(activation.journal) if activation.journal.exists() else None
                selected = activation.recover()
                expected = sha if phase in ('accepted', 'journal-removed', 'gate-opened') else old
                assert selected == expected
                assert regular_bytes(activation.save, 4096) == before
                assert regular_bytes(activation.release(old) / 'activation.json', 10000) == receipt
                assert gateway_request(gateway_port)[0] == 200
                # A second fresh-process claimant fails before touching the gate.
                duplicate = subprocess.run(['/usr/bin/python3', '-c',
                    'import sys; from updater_systemd import DisposableSystemdEnvironment; '
                    'from pathlib import Path; import json; '
                    'DisposableSystemdEnvironment.reopen_disposable_test(json.loads(Path(sys.argv[1]).read_bytes()))',
                    str(root / 'control/harness-config.json')], cwd=Path(__file__).resolve().parents[1],
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=10)
                assert duplicate.returncode != 0 and environment.gate_open()
                evidence.append(dict(phase=phase, selected=selected, journal=observed_journal,
                                     save_sha256=hashlib.sha256(before).hexdigest(),
                                     gateway_closed_before_recovery=True, duplicate_refused=True))
                environment.begin_maintenance()
        print(json.dumps({'fixture_only': True, 'fresh_process_sigkill': evidence,
                          'root': str(root), 'reboot_tested': False}, sort_keys=True))
    finally:
        try:
            with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
                environment.service.stop()
        finally:
            gateway.terminate()
            gateway.wait(timeout=5)
    # Preserve marked roots, closed gate and trusted gateway tool as evidence.


if __name__ == '__main__':
    main()
