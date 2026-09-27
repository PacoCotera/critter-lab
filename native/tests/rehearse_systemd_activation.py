"""Root, single-process disposable transaction; fixture SHAs are NOT releases.

Requires explicit trusted executable bundle and NEW /var/lib/critter-lab-updater-test-*.
Does not support live/legacy installation, cross-process recovery or reboot.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
import urllib.request

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from updater_activation import Activation
from updater_environment import regular_bytes
from updater_systemd import DisposableSystemdEnvironment, PREFIX
from rehearse_updater import fixture_bundle, free_port
import package_staging


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bundle')
    parser.add_argument('--root', required=True)
    parser.add_argument('--runtime-uid', required=True, type=int)
    parser.add_argument('--runtime-gid', required=True, type=int)
    parser.add_argument('--check-uid', required=True, type=int)
    parser.add_argument('--check-gid', required=True, type=int)
    args = parser.parse_args()
    root = Path(args.root)
    gateway = Path('/run/critter-lab-updater-test-gateway')
    if os.geteuid() != 0 or root.parent != Path('/var/lib') or not root.name.startswith(PREFIX):
        raise ValueError('explicit new root test path required')
    if root.exists() or gateway.exists():
        raise ValueError('refusing existing disposable paths')
    if min(args.runtime_uid, args.runtime_gid, args.check_uid, args.check_gid) <= 0 or args.runtime_uid == args.check_uid:
        raise ValueError('separate nonzero test identities required')
    original = Path(args.bundle).resolve()
    with package_staging.bounded_tar(original) as archive:
        old = json.load(archive.extractfile(package_staging.ROOT_NAME + '/presenter/release.json'))['commit']
    package_staging.verify(original, old)
    root.mkdir(mode=0o700)
    gateway.mkdir(mode=0o755)
    gateway.chmod(0o755)
    (root / '.disposable-systemd').write_bytes(b'critter-lab-disposable-systemd\n')
    for name in ('releases', 'control', 'runtime'):
        (root / name).mkdir(mode=0o700)
    os.chown(root / 'runtime', args.runtime_uid, args.runtime_gid)
    config = dict(root=str(root), port=free_port(), health_port=free_port(), timeout=3,
                  disposable_systemd=True, runtime_uid=args.runtime_uid, runtime_gid=args.runtime_gid,
                  check_uid=args.check_uid, check_gid=args.check_gid)
    with DisposableSystemdEnvironment(config) as environment:
        activation = Activation.from_disposable_systemd(environment)
        next_sha, broken_sha = 'f' * 40, 'e' * 40
        if old in (next_sha, broken_sha):
            raise ValueError('fixture SHA collision')
        try:
            for sha, filename, broken in ((old, original, False), (next_sha, root / 'next-fixture.tar.gz', False),
                                          (broken_sha, root / 'broken-fixture.tar.gz', True)):
                if sha != old:
                    fixture_bundle(original, filename, sha, broken)
                activation.admit(filename, sha, hashlib.sha256(regular_bytes(filename)).hexdigest())
            with environment.scratch() as scratch:
                status = json.loads(environment.run_native(activation.release(old), scratch / 'state', 'status'))
                reviewed = json.loads(environment.run_native(activation.release(old), scratch / 'state',
                                     'command', 'review', status['revision'], 'seed-review'))
                environment.run_native(activation.release(old), scratch / 'state',
                                       'command', 'back', reviewed['revision'], 'seed-back')
                activation.save.write_bytes(regular_bytes(scratch / 'state', 4096))
            os.chown(activation.save, args.runtime_uid, args.runtime_gid)
            activation.save.chmod(0o600)
            initial = regular_bytes(activation.save, 4096)
            activation.activate(old)
            assert environment.gate_open() and regular_bytes(activation.save, 4096) == initial
            receipt = regular_bytes(activation.release(old) / 'activation.json', 10000)
            origin = 'http://127.0.0.1:' + str(config['port'])
            def play(action):
                with urllib.request.urlopen(origin + '/api/status', timeout=3) as response:
                    status = json.load(response)
                data = json.dumps(dict(name=action, revision=status['revision'], operation_id='rehearsal-' + str(status['revision']))).encode()
                request = urllib.request.Request(origin + '/api/command', data=data,
                    headers={'Content-Type': 'application/json', 'Origin': origin, 'X-Requested-With': 'CritterLab'})
                with urllib.request.urlopen(request, timeout=3) as response:
                    return json.load(response)
            play('review')
            played = regular_bytes(activation.save, 4096)
            assert played != initial
            activation.activate(next_sha)
            assert regular_bytes(activation.save, 4096) == played and environment.gate_open()
            play('back')
            played = regular_bytes(activation.save, 4096)
            activation.activate(old, rollback=True)
            assert regular_bytes(activation.save, 4096) == played
            assert regular_bytes(activation.release(old) / 'activation.json', 10000) == receipt
            try:
                activation.activate(broken_sha)
                raise AssertionError('broken fixture accepted')
            except ValueError:
                assert not environment.gate_open()
                assert regular_bytes(activation.save, 4096) == played
                assert activation.selected() == old
            activation.recover()
            assert environment.gate_open() and regular_bytes(activation.save, 4096) == played
            print(json.dumps({'fixture_only': True, 'initial': old, 'next_fixture': next_sha,
                              'rollback_save_preserved': True, 'failure_gate_closed': True,
                              'same_process_recovery': True, 'root': str(root)}, sort_keys=True))
        finally:
            environment.service.stop()
    # Keep marked roots and closed gate as evidence; never delete unknown paths.


if __name__ == '__main__':
    main()
