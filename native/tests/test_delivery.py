"""Signed queue and independently verified public release tests; fixture metadata only."""
import gzip
import hashlib
import hmac
from http.server import HTTPServer
import io
import json
import os
import socket
from pathlib import Path
import sys
import tarfile
import tempfile
import threading
import time
import unittest
import uuid
import urllib.request
import urllib.error

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import deployment_hook
import package_staging
import release_delivery
from test_updater import Metadata, SHA


def bundle_fixture():
    metadata = dict(commit=SHA, subject='Fixture', committed_at='2026-09-26T00:00:00+00:00', deployed_at=None)
    files = {name: b'fixture' for name in package_staging.PAYLOAD}
    files['presenter/release.json'] = json.dumps(metadata).encode()
    files['MANIFEST.txt'] = package_staging.manifest({name: files[name] for name in package_staging.PAYLOAD}, metadata)
    from datetime import datetime
    timestamp = int(datetime.fromisoformat(metadata['committed_at']).timestamp())
    output = io.BytesIO()
    with gzip.GzipFile(filename='', fileobj=output, mode='wb', mtime=0) as compressed:
        with tarfile.open(fileobj=compressed, mode='w', format=tarfile.PAX_FORMAT) as archive:
            for name in sorted(files):
                member = tarfile.TarInfo(package_staging.ROOT_NAME + '/' + name)
                member.size, member.mode, member.mtime = len(files[name]), package_staging.MODES.get(name, 0o644), timestamp
                archive.addfile(member, io.BytesIO(files[name]))
    return output.getvalue()


class ReleaseFixture(Metadata):
    def __init__(self):
        super().__init__()
        bundle = bundle_fixture()
        delivery = dict(schema=1, repository=release_delivery.REPOSITORY, sha=SHA,
                        workflow=release_delivery.WORKFLOW, run_id=10, attempt=2,
                        bundle=f'critter-lab-staging-{SHA}.tar.gz', bundle_sha256=hashlib.sha256(bundle).hexdigest(),
                        save_format='CRITTER_DEMO 1')
        self.data = {1: bundle, 2: json.dumps(delivery).encode()}
        self.release = dict(id=30, draft=False, tag_name='staging-' + SHA, assets=[
            dict(id=index, name=delivery['bundle'] if index == 1 else 'delivery.json',
                 size=len(data), state='uploaded', digest='sha256:' + hashlib.sha256(data).hexdigest())
            for index, data in self.data.items()])
        self.ref = dict(object=dict(type='commit', sha=SHA))
        self.direction = dict(status='ahead', merge_base_commit=dict(sha='0' * 40))

    def metadata(self, path):
        if path == '/releases/30':
            return self.release
        if path.startswith('/git/ref/'):
            return self.ref
        if path.startswith('/compare/') and not path.endswith('...main'):
            return self.direction
        return super().metadata(path)

    def read(self, url, limit, accept):
        return self.data[int(url.rsplit('/', 1)[1])]


class DeliveryChecks(unittest.TestCase):
    def signed(self):
        secret = b'TEST_ONLY_WEBHOOK_SECRET_32_BYTES_LONG'
        body = json.dumps(dict(action='published', repository=dict(full_name=deployment_hook.REPOSITORY,
                     id=deployment_hook.REPOSITORY_ID), release=dict(id=30, draft=False, tag_name='staging-' + SHA))).encode()
        signature = 'sha256=' + hmac.new(secret, body, hashlib.sha256).hexdigest()
        return secret, body, signature, str(uuid.uuid4())

    def test_signature_body_and_event_rejection(self):
        secret, body, signature, delivery = self.signed()
        record = deployment_hook.notification(secret, body, signature, 'release', delivery)
        self.assertEqual(record['sha'], SHA)
        for arguments in ((secret, body + b' ', signature, 'release', delivery),
                          (secret, body, signature, 'workflow_run', delivery),
                          (secret, body, signature, 'release', '../escape')):
            with self.assertRaises(ValueError):
                deployment_hook.notification(*arguments)

    @unittest.skipUnless(os.name == 'posix', 'directory fsync requires Linux')
    def test_durable_semantic_deduplication(self):
        secret, body, signature, delivery = self.signed()
        record = deployment_hook.notification(secret, body, signature, 'release', delivery)
        with tempfile.TemporaryDirectory() as temporary:
            deployment_hook.enqueue(temporary, record)
            deployment_hook.enqueue(temporary, {**record, 'delivery': str(uuid.uuid4())})
            self.assertEqual(len(list(Path(temporary).glob('*.json'))), 1)
            with self.assertRaises(ValueError):
                deployment_hook.enqueue(temporary, {**record, 'release_id': 31})

    def test_verified_public_release(self):
        delivery, bundle = release_delivery.verified_release(ReleaseFixture(), 30, SHA)
        self.assertEqual(delivery['bundle_sha256'], hashlib.sha256(bundle).hexdigest())

    @unittest.skipUnless(os.name == 'posix', 'real durable HTTP queue requires Linux')
    def test_http_notification_only_enqueues(self):
        secret, body, signature, delivery = self.signed()
        with tempfile.TemporaryDirectory() as temporary:
            server = HTTPServer(('127.0.0.1', 0), deployment_hook.Handler)
            server.secret, server.queue = secret, Path(temporary)
            server.request_deadline = .2
            thread = threading.Thread(target=server.serve_forever)
            thread.start()
            try:
                # Incomplete headers are closed before they can block a valid delivery.
                with socket.create_connection(('127.0.0.1', server.server_port), timeout=2) as slow:
                    slow.sendall(b'POST /deployment/github HTTP/1.1\r\nHost: localhost\r\n')
                    time.sleep(.3)
                    self.assertEqual(slow.recv(1), b'')
                url = f'http://127.0.0.1:{server.server_port}/deployment/github'
                headers = {'Content-Type': 'application/json', 'X-GitHub-Event': 'release',
                           'X-GitHub-Delivery': delivery, 'X-Hub-Signature-256': signature}
                with urllib.request.urlopen(urllib.request.Request(url, body, headers), timeout=2) as response:
                    self.assertEqual(response.status, 202)
                self.assertEqual(json.loads(next(Path(temporary).glob('*.json')).read_text())['release_id'], 30)
                headers['X-Hub-Signature-256'] = 'sha256=' + '0' * 64
                with self.assertRaises(urllib.error.HTTPError) as failed:
                    urllib.request.urlopen(urllib.request.Request(url, body, headers), timeout=2)
                self.assertEqual(failed.exception.code, 400)
            finally:
                server.shutdown()
                thread.join()
                server.server_close()

    def test_wrong_tag_run_and_asset_rejected(self):
        client = ReleaseFixture()
        client.ref['object']['sha'] = 'd' * 40
        with self.assertRaises(ValueError):
            release_delivery.verified_release(client, 30, SHA)

    def test_automatic_direction_is_forward_only(self):
        accepted = '0' * 40
        release_delivery.verified_release(ReleaseFixture(), 30, SHA, accepted)
        for status, base in (('identical', accepted), ('behind', accepted), ('diverged', accepted), ('ahead', 'd' * 40)):
            client = ReleaseFixture()
            client.direction = dict(status=status, merge_base_commit=dict(sha=base))
            with self.subTest(status=status), self.assertRaises(ValueError):
                release_delivery.verified_release(client, 30, SHA, accepted)
        client = ReleaseFixture()
        client.run['event'] = 'pull_request'
        with self.assertRaises(ValueError):
            release_delivery.verified_release(client, 30, SHA)
        client = ReleaseFixture()
        client.data[1] += b'tampering'
        with self.assertRaises(ValueError):
            release_delivery.verified_release(client, 30, SHA)

    def test_hidden_expansion_bound_is_portable(self):
        previous = package_staging.ARCHIVE_LIMIT
        package_staging.ARCHIVE_LIMIT = 1024
        try:
            with tempfile.TemporaryDirectory() as temporary:
                path = Path(temporary) / 'expanded.gz'
                path.write_bytes(gzip.compress(b'\0' * 2048))
                with self.assertRaises(ValueError):
                    package_staging.bounded_tar(path)
        finally:
            package_staging.ARCHIVE_LIMIT = previous


if __name__ == '__main__':
    unittest.main()
