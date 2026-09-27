"""Disposable Linux process/crash rehearsal. Fixture SHAs are not published releases."""
import argparse
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import signal
import socket
import subprocess
import sys
import tarfile
import tempfile
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import package_staging
from updater_activation import Activation, bounded_tar
import updater_activation
from updater_service import marked_groups, group_alive


def fixture_bundle(original, target, sha, broken=False):
    with tarfile.open(original, 'r:gz') as source:
        files = {Path(member.name).relative_to(package_staging.ROOT_NAME).as_posix():
                 source.extractfile(member).read() for member in source}
    metadata = json.loads(files['presenter/release.json'])
    metadata['commit'] = sha
    files['presenter/release.json'] = json.dumps(metadata).encode()
    if broken:
        files['presenter/server.py'] = b'raise RuntimeError("rehearsal startup failure")\n'
    files['MANIFEST.txt'] = package_staging.manifest(
        {name: files[name] for name in package_staging.PAYLOAD}, metadata)
    from datetime import datetime
    timestamp = int(datetime.fromisoformat(metadata['committed_at']).timestamp())
    with target.open('wb') as raw, gzip.GzipFile(filename='', fileobj=raw, mode='wb', mtime=0) as compressed:
        with tarfile.open(fileobj=compressed, mode='w', format=tarfile.PAX_FORMAT) as archive:
            for name in sorted(files):
                member = tarfile.TarInfo(package_staging.ROOT_NAME + '/' + name)
                member.size = len(files[name])
                member.mode = package_staging.MODES.get(name, 0o644)
                member.mtime = timestamp
                archive.addfile(member, io.BytesIO(files[name]))
    package_staging.verify(target, sha)


def free_port():
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0))
        return listener.getsockname()[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bundle')
    args = parser.parse_args()
    original = Path(args.bundle).resolve()
    with tarfile.open(original, 'r:gz') as source:
        old = json.load(source.extractfile(package_staging.ROOT_NAME + '/presenter/release.json'))['commit']
    candidate, broken = 'f' * 40, 'e' * 40
    with tempfile.TemporaryDirectory(prefix='critter-updater-') as temporary:
        root = Path(temporary)
        (root / '.disposable').write_bytes(b'critter-lab-disposable\n')
        config = dict(root=str(root), port=free_port(), health_port=free_port(), timeout=2, disposable=True)
        activation = Activation(config)
        try:
            for sha, bundle in ((old, original), (candidate, root / 'candidate.tar.gz'), (broken, root / 'broken.tar.gz')):
                if sha != old:
                    fixture_bundle(original, bundle, sha, broken=sha == broken)
                activation.admit(bundle, sha, hashlib.sha256(bundle.read_bytes()).hexdigest())
            binary = activation.release(old) / 'bin/critter_lab'
            def command(action):
                state = json.loads(subprocess.check_output([str(binary), '--save', str(activation.save), 'status']))
                return json.loads(subprocess.check_output([str(binary), '--save', str(activation.save),
                    'command', action, str(state['revision']), 'play-' + str(state['revision'])]))
            for action in ('review', 'load', 'start', 'advance', 'inspect', 'advance', 'receive', 'study_review', 'run'):
                command(action)
            before = activation.save.read_bytes()
            activation.activate(old)
            old_time = json.loads((activation.release(old) / 'activation.json').read_text())['deployed_at']
            old_receipt = (activation.release(old) / 'activation.json').read_bytes()
            activation.activate(candidate)
            assert activation.save.read_bytes() == before
            command('back')
            played = activation.save.read_bytes()
            activation.activate(old, rollback=True)
            assert activation.save.read_bytes() == played
            assert json.loads((activation.release(old) / 'activation.json').read_text())['deployed_at'] == old_time
            assert (activation.release(old) / 'activation.json').read_bytes() == old_receipt
            try:
                activation.activate(broken)
                raise AssertionError('broken startup accepted')
            except ValueError:
                assert activation.selected() == old
                assert activation.save.read_bytes() == played
            crash_results = {}
            for index, phase in enumerate(('prepared', 'pointer', 'switched', 'receipt', 'accepted')):
                crash_sha = str(index + 1) * 40
                crash_bundle = root / (crash_sha + '.tar.gz')
                fixture_bundle(original, crash_bundle, crash_sha)
                activation.admit(crash_bundle, crash_sha, hashlib.sha256(crash_bundle.read_bytes()).hexdigest())
                child = os.fork()
                if child == 0:
                    worker = Activation(config)
                    worker.transition_hook = lambda reached: os.kill(os.getpid(), signal.SIGKILL) if reached == phase else None
                    worker.activate(crash_sha)
                    os._exit(2)
                _, status = os.waitpid(child, 0)
                assert os.WIFSIGNALED(status) and os.WTERMSIG(status) == signal.SIGKILL
                fresh = Activation(config)
                recovered = fresh.recover()
                assert recovered == (crash_sha if phase == 'accepted' else old)
                assert fresh.save.read_bytes() == played
                crash_results[phase] = recovered
                fresh.service.stop()
                if recovered == crash_sha:
                    fresh.activate(old, rollback=True)
                activation = fresh
            with activation.lock():
                try:
                    with Activation(config).lock():
                        raise AssertionError('concurrent lock acquired')
                except BlockingIOError:
                    pass
            assert activation.save.read_bytes() == played
            # Bound hidden expansion before tarfile interprets any header.
            compressed = root / 'oversized.gz'
            compressed.write_bytes(gzip.compress(b'\0' * 2048))
            saved_limit = package_staging.ARCHIVE_LIMIT
            package_staging.ARCHIVE_LIMIT = 1024
            try:
                try:
                    bounded_tar(compressed)
                    raise AssertionError('expanded bytes accepted')
                except ValueError:
                    pass
            finally:
                package_staging.ARCHIVE_LIMIT = saved_limit
            # Mutable installed source fields must not become trusted metadata.
            metadata_path = activation.release(candidate) / 'presenter/release.json'
            metadata_bytes = metadata_path.read_bytes()
            metadata = json.loads(metadata_bytes)
            metadata['subject'] = 'tampered'
            metadata_path.write_text(json.dumps(metadata))
            try:
                activation.check_integrity(candidate)
                raise AssertionError('altered source metadata accepted')
            except ValueError:
                pass
            metadata_path.write_bytes(metadata_bytes)
            for bad in (None, b'corrupt save'):
                if bad is None:
                    activation.save.unlink()
                else:
                    activation.save.write_bytes(bad)
                try:
                    activation.activate(candidate, rollback=True)
                    raise AssertionError('invalid save accepted')
                except (ValueError, FileNotFoundError):
                    assert not activation.service.pid_file.exists()
                activation.save.write_bytes(played)
            activation.recover()
            pointer_bytes = activation.pointer.read_bytes()
            activation.pointer.write_bytes(b'corrupt selection')
            try:
                activation.activate(candidate, rollback=True)
                raise AssertionError('corrupt selection accepted')
            except ValueError:
                assert not activation.service.pid_file.exists()
            activation.pointer.write_bytes(pointer_bytes)
            # A TERM handler creates a detached marked descendant during shutdown.
            descendant_release = root / 'descendant-fixture'
            (descendant_release / 'presenter').mkdir(parents=True)
            (descendant_release / 'presenter/server.py').write_text(
                'import os,signal,subprocess,sys,time\n'
                'def terminate(*args):\n'
                ' subprocess.Popen([sys.executable,"-c","import time; time.sleep(120)"],start_new_session=True)\n'
                ' sys.exit(0)\n'
                'signal.signal(signal.SIGTERM,terminate)\n'
                'open(os.environ["CRITTER_DEMO_SAVE"]+".ready","w").write("ready")\n'
                'while True: time.sleep(.05)\n')
            activation.service.start(descendant_release)
            readiness = Path(str(activation.save) + '.ready')
            deadline = time.monotonic() + 2
            while not readiness.exists():
                assert time.monotonic() < deadline
                time.sleep(.05)
            reservation = json.loads(activation.service.pid_file.read_text())
            activation.service.stop()
            assert not marked_groups(reservation['token'])
            print(json.dumps(dict(activation=True, rollback_preserves_play=True, failed_startup_rolled_back=True,
                                  concurrency_excluded=True, crash_recovery=crash_results,
                                  detached_descendants_stopped=True, malformed_state_stops_service=True,
                                  source_metadata_tampering_rejected=True, hidden_expansion_bounded=True,
                                  save_sha256=hashlib.sha256(played).hexdigest()), indent=2))
        finally:
            activation.service.stop()


if __name__ == '__main__':
    main()
