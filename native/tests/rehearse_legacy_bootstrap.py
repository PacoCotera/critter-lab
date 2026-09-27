"""Authored local legacy-fixture bootstrap, never live capture or CI provenance.

The fixture source label mirrors de7d067; executable bytes come from the supplied
trusted test bundle and are NOT asserted to be that historical host executable.
"""
import argparse
from datetime import datetime
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import re
import signal
import stat
import subprocess
import sys
import tarfile
import tempfile
import urllib.request

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import package_staging
from updater_activation import Activation, write_json, sync_directory
from updater_environment import regular_bytes
from updater_systemd import DisposableSystemdEnvironment, PREFIX, protected
from rehearse_updater import free_port, fixture_bundle

LEGACY_SHA = 'de7d06720c93e34033b92e461d91396ff1faebb3'
INPUTS = ('critter_lab', 'presenter/server.py', 'presenter/index.html',
          'presenter/app.js', 'presenter/style.css', 'presenter/release.json')


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def legacy_bytes(root, name):
    path = root / name
    for component in (root, *path.parents):
        if component.is_symlink() or not stat.S_ISDIR(component.lstat().st_mode):
            raise ValueError('legacy fixture directory links rejected')
    return regular_bytes(path, 64 * 1024 * 1024 if name == 'critter_lab' else 2 * 1024 * 1024)


def historical_metadata(data):
    metadata = json.loads(data)
    if not isinstance(metadata, dict) or set(metadata) != {'commit', 'subject', 'committed_at', 'deployed_at'}:
        raise ValueError('complete supplied historical fixture metadata required')
    if metadata['commit'] != LEGACY_SHA:
        raise ValueError('explicit legacy fixture source label required')
    for name in ('committed_at', 'deployed_at'):
        if not isinstance(metadata[name], str) or datetime.fromisoformat(metadata[name].replace('Z', '+00:00')).tzinfo is None:
            raise ValueError('supplied historical times require timezone')
    return metadata


def fresh_bytes(path, data, mode=0o644):
    descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW | os.O_WRONLY, mode)
    try:
        os.fchmod(descriptor, mode)
        with os.fdopen(descriptor, 'wb', closefd=False) as output:
            output.write(data)
            output.flush()
            os.fsync(descriptor)
    finally:
        os.close(descriptor)
    sync_directory(path.parent)


def normalized_bundle(path, files, metadata):
    payload = {'bin/critter_lab': files['critter_lab']}
    payload.update({name: files[name] for name in INPUTS if name.startswith('presenter/') and name != 'presenter/release.json'})
    normalized = {**metadata, 'deployed_at': None}
    payload['presenter/release.json'] = json.dumps(normalized, sort_keys=True).encode()
    payload['MANIFEST.txt'] = package_staging.manifest({name: payload[name] for name in package_staging.PAYLOAD}, normalized)
    timestamp = int(datetime.fromisoformat(metadata['committed_at'].replace('Z', '+00:00')).timestamp())
    with path.open('xb') as raw, gzip.GzipFile(filename='', fileobj=raw, mode='wb', mtime=0) as compressed:
        with tarfile.open(fileobj=compressed, mode='w', format=tarfile.PAX_FORMAT) as archive:
            for name in sorted(payload):
                member = tarfile.TarInfo(package_staging.ROOT_NAME + '/' + name)
                member.size, member.mode, member.mtime = len(payload[name]), package_staging.MODES.get(name, 0o644), timestamp
                archive.addfile(member, io.BytesIO(payload[name]))
    with path.open('rb') as archive:
        os.fsync(archive.fileno())
    sync_directory(path.parent)
    package_staging.verify(path, LEGACY_SHA)


def bootstrap_fixture(legacy_root, expected_hashes, supplied_save, *, activation):
    """Prepare and accept an authored fixture; supplied state is never written.

    supplied_save has exact path/sha256 fields. Caller is a single-threaded test
    coordinator holding the disposable namespace, not an installed entrypoint.
    """
    environment = activation.environment
    if os.geteuid() != 0 or type(environment) is not DisposableSystemdEnvironment or not activation._systemd_test:
        raise ValueError('held root disposable test seam required')
    environment._require_ownership()
    legacy_root = Path(legacy_root)
    if legacy_root != environment.root / 'legacy-fixture' or legacy_root.resolve() != legacy_root:
        raise ValueError('fixed authored fixture layout required')
    if regular_bytes(legacy_root / '.local-legacy-fixture', 100) != b'critter-lab-local-legacy-fixture\n':
        raise ValueError('authored legacy fixture marker required')
    if (not isinstance(expected_hashes, dict) or set(expected_hashes) != set(INPUTS)
            or any(not isinstance(value, str) or not re.fullmatch('[0-9a-f]{64}', value) for value in expected_hashes.values())):
        raise ValueError('explicit complete fixture hashes required')
    if (not isinstance(supplied_save, dict) or set(supplied_save) != {'path', 'sha256'}
            or Path(supplied_save['path']) != activation.save
            or not isinstance(supplied_save['sha256'], str) or not re.fullmatch('[0-9a-f]{64}', supplied_save['sha256'])):
        raise ValueError('explicit existing fixture save identity required')
    with activation.lock():
        environment.begin_maintenance()
        activation.service.stop()
        if any(path.exists() or path.is_symlink() for path in (activation.pointer, activation.journal, activation.releases / LEGACY_SHA)):
            raise ValueError('bootstrap requires empty selection/journal and absent baseline')
        with activation.save_lock():
            before = regular_bytes(activation.save, 4096)
            if sha256(before) != supplied_save['sha256']:
                raise ValueError('supplied fixture save changed')
            files = {name: legacy_bytes(legacy_root, name) for name in INPUTS}
            if {name: sha256(data) for name, data in files.items()} != expected_hashes:
                raise ValueError('authored legacy input hashes changed')
            metadata = historical_metadata(files['presenter/release.json'])
            with tempfile.TemporaryDirectory(prefix='bootstrap-', dir=activation.control) as temporary:
                bundle = Path(temporary) / 'normalized-fixture.tar.gz'
                normalized_bundle(bundle, files, metadata)
                # A second bounded read detects fixture mutation during normalization.
                if any(sha256(legacy_bytes(legacy_root, name)) != expected_hashes[name] for name in INPUTS):
                    raise ValueError('legacy input changed during preparation')
                release = activation.admit(bundle, LEGACY_SHA, sha256(regular_bytes(bundle)))
                fresh_bytes(release / 'legacy-release.json', files['presenter/release.json'])
                origin = dict(schema=1, origin='local-legacy-fixture', reported_source_sha=LEGACY_SHA,
                              fixture_note='Authored layout copied from a supplied test bundle; not actual de7d067 host bytes or CI provenance.',
                              normalized_metadata_is_derivative=True,
                              fixture_input_sha256=dict(expected_hashes),
                              normalized_archive_sha256=sha256(regular_bytes(bundle)),
                              historical_metadata_sha256=expected_hashes['presenter/release.json'],
                              historical_deployed_at=metadata['deployed_at'],
                              supplied_save_sha256=supplied_save['sha256'])
                write_json(release / 'bootstrap-origin.json', origin)
                if regular_bytes(activation.save, 4096) != before:
                    raise ValueError('fixture save changed during preparation')
    # Reuse the sole activation transaction, including null-old failure behavior.
    activation.activate(LEGACY_SHA)
    if regular_bytes(activation.save, 4096) != before:
        raise ValueError('bootstrap changed supplied state')
    return origin


def child(config_path, mode):
    config = json.loads(regular_bytes(Path(config_path), 10000))
    with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
        activation = Activation.from_disposable_systemd(environment)
        if mode == 'recover':
            print(activation.recover())
            return
        if mode.startswith('kill-'):
            phase = mode.removeprefix('kill-')
            activation.transition_hook = lambda current: os.kill(os.getpid(), signal.SIGKILL) if current == phase else None
        if mode == 'failed-health':
            activation.service.health = lambda *args, **kwargs: (_ for _ in ()).throw(ValueError('synthetic initial health failure'))
        evidence = json.loads(regular_bytes(environment.control / 'fixture-input.json', 10000))
        bootstrap_fixture(environment.root / 'legacy-fixture', evidence['hashes'], evidence['save'], activation=activation)


def initialize(args):
    root = Path(args.root)
    gateway = Path('/run/critter-lab-updater-test-gateway')
    if os.geteuid() != 0 or root.parent != Path('/var/lib') or not root.name.startswith(PREFIX):
        raise ValueError('new explicit disposable root required')
    if root.exists() or gateway.exists():
        raise ValueError('refusing existing fixture paths')
    if min(args.runtime_uid, args.runtime_gid, args.check_uid, args.check_gid) <= 0 or args.runtime_uid == args.check_uid:
        raise ValueError('separate nonzero identities required')
    original = Path(args.bundle).resolve()
    history_bytes = regular_bytes(Path(args.legacy_metadata).resolve(), 10000)
    history = historical_metadata(history_bytes)
    with package_staging.bounded_tar(original) as source:
        original_metadata = json.load(source.extractfile(package_staging.ROOT_NAME + '/presenter/release.json'))
    package_staging.verify(original, original_metadata['commit'])
    root.mkdir(mode=0o700)
    gateway.mkdir(mode=0o755)
    gateway.chmod(0o755)
    (root / '.disposable-systemd').write_bytes(b'critter-lab-disposable-systemd\n')
    for name in ('releases', 'control', 'runtime', 'legacy-fixture'):
        (root / name).mkdir(mode=0o700)
    os.chown(root / 'runtime', args.runtime_uid, args.runtime_gid)
    legacy = root / 'legacy-fixture'
    (legacy / 'presenter').mkdir(mode=0o700)
    (legacy / '.local-legacy-fixture').write_bytes(b'critter-lab-local-legacy-fixture\n')
    with package_staging.bounded_tar(original) as source:
        for member in source:
            name = Path(member.name).relative_to(package_staging.ROOT_NAME).as_posix()
            target_name = 'critter_lab' if name == 'bin/critter_lab' else name
            if target_name in INPUTS and target_name != 'presenter/release.json':
                fresh_bytes(legacy / target_name, source.extractfile(member).read(), member.mode)
    fresh_bytes(legacy / 'presenter/release.json', history_bytes)
    for entry in (legacy, *legacy.rglob('*')):
        os.chown(entry, args.runtime_uid, args.runtime_gid)
    config = dict(root=str(root), port=free_port(), health_port=free_port(), timeout=3, disposable_systemd=True,
                  runtime_uid=args.runtime_uid, runtime_gid=args.runtime_gid,
                  check_uid=args.check_uid, check_gid=args.check_gid)
    with DisposableSystemdEnvironment(config) as environment:
        try:
            environment._atomic_control(environment.control / 'harness-config.json', config)
            activation = Activation.from_disposable_systemd(environment)
            # Author supplied noninitial state separately; bootstrap itself never seeds.
            seed_sha = original_metadata['commit']
            if seed_sha == LEGACY_SHA:
                raise ValueError('trusted seed bundle must differ from synthetic legacy label')
            activation.admit(original, seed_sha, sha256(regular_bytes(original)))
            with environment.scratch() as scratch:
                state = json.loads(environment.run_native(activation.release(seed_sha), scratch / 'state', 'status'))
                state = json.loads(environment.run_native(activation.release(seed_sha), scratch / 'state', 'command', 'review', state['revision'], 'legacy-seed-review'))
                environment.run_native(activation.release(seed_sha), scratch / 'state', 'command', 'back', state['revision'], 'legacy-seed-back')
                activation.save.write_bytes(regular_bytes(scratch / 'state', 4096))
            activation.save.chmod(0o600)
            os.chown(activation.save, args.runtime_uid, args.runtime_gid)
            environment._atomic_control(environment.control / 'fixture-input.json',
                {'hashes': {name: sha256(legacy_bytes(legacy, name)) for name in INPUTS},
                 'save': {'path': str(activation.save), 'sha256': sha256(regular_bytes(activation.save, 4096))}})
        finally:
            environment.begin_maintenance()
            environment._reclaim_units()
    return config, seed_sha


def reset_unaccepted_fixture(config, expected_digest):
    """Explicit operator test cleanup; never automatic transaction recovery."""
    with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
        activation = Activation.from_disposable_systemd(environment)
        activation.service.stop()
        if sha256(regular_bytes(activation.save, 4096)) != expected_digest:
            raise ValueError('test reset refused changed save')
        journal = activation.read_record(activation.journal)
        if (set(journal) != {'phase', 'old', 'candidate', 'save_digest'}
                or journal['phase'] not in ('prepared', 'switched')
                or journal['old'] is not None or journal['candidate'] != LEGACY_SHA
                or journal['save_digest'] != expected_digest):
            raise ValueError('test reset only permits unaccepted initial fixture journal')
        if activation.pointer.exists() and activation.read_record(activation.pointer) != {'sha': LEGACY_SHA}:
            raise ValueError('test reset refuses unrelated selection')
        release = environment.release_path(activation.releases / LEGACY_SHA)
        origin = json.loads(regular_bytes(protected(release / 'bootstrap-origin.json'), 10000))
        inputs = environment._read_control(environment.control / 'fixture-input.json')
        history = regular_bytes(release / 'legacy-release.json', 10000)
        metadata = historical_metadata(history)
        if (type(origin.get('schema')) is not int or origin['schema'] != 1
                or origin.get('origin') != 'local-legacy-fixture'
                or origin.get('reported_source_sha') != LEGACY_SHA
                or origin.get('normalized_metadata_is_derivative') is not True
                or origin.get('supplied_save_sha256') != expected_digest
                or origin.get('fixture_input_sha256') != inputs.get('hashes')
                or inputs.get('save') != {'path': str(activation.save), 'sha256': expected_digest}
                or origin.get('historical_metadata_sha256') != sha256(history)
                or origin.get('historical_deployed_at') != metadata['deployed_at']
                or origin.get('normalized_archive_sha256') != sha256(regular_bytes(release / 'original.tar.gz'))):
            raise ValueError('test reset refuses nonfixture baseline')
        activation.check_integrity(LEGACY_SHA)
        # Every identity and bound is checked before destructive test cleanup.
        for record in (activation.pointer, activation.journal):
            if record.exists():
                record.unlink()
        import shutil
        shutil.rmtree(release)
        sync_directory(activation.control)
        sync_directory(activation.releases)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bundle', nargs='?')
    parser.add_argument('--legacy-metadata')
    parser.add_argument('--root')
    for identity in ('runtime-uid', 'runtime-gid', 'check-uid', 'check-gid'):
        parser.add_argument('--' + identity, type=int)
    parser.add_argument('--child-config')
    parser.add_argument('--mode', choices=('recover', 'kill-switched', 'kill-accepted', 'failed-health'))
    args = parser.parse_args()
    if args.child_config:
        child(args.child_config, args.mode)
        return
    config, candidate_sha = initialize(args)
    root = Path(config['root'])
    evidence = json.loads(regular_bytes(root / 'control/fixture-input.json', 10000))
    before = regular_bytes(root / 'runtime/state', 4096)
    config_path = root / 'control/harness-config.json'
    def run(mode):
        return subprocess.run(['/usr/bin/python3', str(Path(__file__).resolve()), '--child-config', str(config_path), '--mode', mode], timeout=120)
    try:
        for mode in ('failed-health', 'kill-switched'):
            result = run(mode)
            if mode == 'kill-switched':
                assert result.returncode == -signal.SIGKILL
            else:
                assert result.returncode != 0
            with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
                activation = Activation.from_disposable_systemd(environment)
                try:
                    activation.recover()
                    raise AssertionError('initial unaccepted baseline recovered')
                except ValueError:
                    assert not environment.gate_open()
                    assert regular_bytes(activation.save, 4096) == before
            reset_unaccepted_fixture(config, evidence['save']['sha256'])
        result = run('kill-accepted')
        assert result.returncode == -signal.SIGKILL
        assert run('recover').returncode == 0
        with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
            activation = Activation.from_disposable_systemd(environment)
            assert activation.recover() == LEGACY_SHA
            release = activation.release(LEGACY_SHA)
            origin = activation.read_record(release / 'bootstrap-origin.json')
            assert origin['origin'] == 'local-legacy-fixture'
            assert regular_bytes(release / 'legacy-release.json', 10000) == legacy_bytes(root / 'legacy-fixture', 'presenter/release.json')
            for name in INPUTS:
                if name != 'presenter/release.json':
                    normalized_name = 'bin/critter_lab' if name == 'critter_lab' else name
                    assert sha256(regular_bytes(release / normalized_name)) == evidence['hashes'][name]
            assert regular_bytes(activation.save, 4096) == before
            baseline_receipt = regular_bytes(release / 'activation.json', 10000)
            activation.activate(candidate_sha)
            origin_url = 'http://127.0.0.1:' + str(config['port'])
            with urllib.request.urlopen(origin_url + '/api/status', timeout=3) as response:
                status = json.load(response)
            request = urllib.request.Request(origin_url + '/api/command', data=json.dumps(dict(
                name='review', revision=status['revision'], operation_id='legacy-candidate-play')).encode(),
                headers={'Content-Type': 'application/json', 'Origin': origin_url, 'X-Requested-With': 'CritterLab'})
            with urllib.request.urlopen(request, timeout=3) as response:
                assert response.status == 200
            played = regular_bytes(activation.save, 4096)
            assert played != before
            activation.activate(LEGACY_SHA, rollback=True)
            assert regular_bytes(activation.save, 4096) == played
            assert regular_bytes(release / 'activation.json', 10000) == baseline_receipt
            print(json.dumps({'origin': origin, 'bootstrap_receipt': json.loads(baseline_receipt),
                'synthetic_fixture_only': True, 'before_accepted_failed_closed': True,
                'after_accepted_fresh_process_recovered': True, 'rollback_preserved_play': True}, sort_keys=True))
    finally:
        with DisposableSystemdEnvironment.reopen_disposable_test(config) as environment:
            environment.service.stop()


if __name__ == '__main__':
    main()
