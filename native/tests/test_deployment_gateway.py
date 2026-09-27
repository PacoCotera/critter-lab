"""Real disposable loopback proxy checks; no host configuration."""
import http.client
import importlib.util
from unittest.mock import patch
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import sys
import tempfile
import threading
import time
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import deployment_gateway as gateway
CANONICAL = 'native.example.test'
spec = importlib.util.spec_from_file_location('presenter_fixture', Path(__file__).resolve().parents[1] / 'presenter/server.py')
presenter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(presenter)



class Upstream(BaseHTTPRequestHandler):
    def log_message(self, *arguments):
        pass

    def do_GET(self):
        self.server.hits += 1
        self.server.host = self.headers.get('Host')
        if self.server.slow:
            time.sleep(.5)
        data = self.server.payload
        self.send_response(201)
        self.send_header('Content-Type', 'application/octet-stream')
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Connection', 'X-Internal')
        self.send_header('X-Internal', 'hidden')
        for name in ('Content-Security-Policy', 'X-Content-Type-Options', 'Referrer-Policy', 'X-Frame-Options'):
            self.send_header(name, 'fixture-' + name)
        self.end_headers()
        try:
            self.wfile.write(data)
        except OSError:
            pass

    def do_POST(self):
        self.server.payload = self.rfile.read(int(self.headers.get('Content-Length', '0')))
        self.do_GET()


@unittest.skipUnless(os.name == 'posix', 'Linux no-follow gate required')
class Proxy(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.gate = Path(self.temporary.name) / 'gate.json'
        self.upstream = ThreadingHTTPServer(('127.0.0.1', 0), Upstream)
        self.upstream.hits, self.upstream.payload, self.upstream.slow = 0, b'bytes\x00\xff', False
        self.proxy = gateway.make_server(self.gate, self.upstream.server_port, 0, canonical_host=CANONICAL, trusted_uid=os.getuid(), timeout=.25)
        for server in (self.upstream, self.proxy):
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            self.addCleanup(server.server_close)
            self.addCleanup(server.shutdown)

    def opened(self):
        self.gate.write_text(json.dumps(dict(schema=1, approved=True, accepted_sha='a' * 40)))

    def request(self, method='GET', body=None, headers=None):
        connection = http.client.HTTPConnection('127.0.0.1', self.proxy.server_port, timeout=2)
        self.addCleanup(connection.close)
        request_headers = {'Host': CANONICAL}
        if method == 'POST':
            request_headers.update({'Origin': 'https://' + CANONICAL, 'X-Requested-With': 'CritterLab', 'Content-Type': 'application/json'})
        request_headers.update(headers or {})
        connection.request(method, '/api/command' if method == 'POST' else '/api/status', body, request_headers)
        response = connection.getresponse()
        return response.status, response.read(), dict(response.getheaders())

    def test_closed_and_malformed_never_contact_upstream(self):
        for data in (None, b'{}', b'broken', b'{"schema":true,"approved":true,"accepted_sha":"' + b'a' * 40 + b'"}'):
            if data is not None:
                self.gate.write_bytes(data)
            for method in ('GET', 'POST'):
                self.assertEqual(self.request(method, b'command')[0], 503)
        self.assertEqual(self.upstream.hits, 0)

    def test_open_preserves_bytes_status_type_and_fixed_host(self):
        self.opened()
        status, body, headers = self.request()
        self.assertEqual((status, body, headers['Content-Type']), (201, b'bytes\x00\xff', 'application/octet-stream'))
        self.assertNotIn('X-Internal', headers)
        for name in ('Content-Security-Policy', 'X-Content-Type-Options', 'Referrer-Policy', 'X-Frame-Options'):
            self.assertEqual(headers[name], 'fixture-' + name)
        self.assertEqual(self.upstream.host, CANONICAL)
        self.assertEqual(self.request('POST', b'command\x00')[0:2], (201, b'command\x00'))

    def test_gate_uid_and_writable_modes_fail_closed(self):
        self.opened()
        self.proxy.trusted_uid = os.getuid() + 1
        self.assertEqual(self.request('POST', b'command')[0], 503)
        self.proxy.trusted_uid = os.getuid()
        for mode in (0o620, 0o602):
            self.gate.chmod(mode)
            self.assertEqual(self.request('POST', b'command')[0], 503)
        self.assertEqual(self.upstream.hits, 0)

    def test_foreign_host_origin_never_contact_upstream(self):
        self.opened()
        for headers in ({'Host': 'foreign.example'}, {'Origin': 'https://foreign.example'},
                        {'Origin': 'http://' + CANONICAL}):
            self.assertEqual(self.request('POST', b'command', headers)[0], 403)
        self.assertEqual(self.upstream.hits, 0)

    def test_real_presenter_origin_checks_are_preserved(self):
        class Presenter(presenter.Handler):
            def native(self, arguments, image=False):
                self.server.commands += 1
                self.reply(200, {'arguments': arguments})
        runtime = ThreadingHTTPServer(('127.0.0.1', 0), Presenter)
        runtime.password, runtime.commands = '', 0
        threading.Thread(target=runtime.serve_forever, daemon=True).start()
        self.addCleanup(runtime.server_close)
        self.addCleanup(runtime.shutdown)
        self.proxy.origin_port = runtime.server_port
        self.opened()
        body = json.dumps(dict(name='review', revision=1, operation_id='fixture')).encode()
        status, _, headers = self.request('POST', body, {'X-Forwarded-Proto': 'http'})
        self.assertEqual(status, 200)
        self.assertEqual(runtime.commands, 1)
        self.assertIn('Content-Security-Policy', headers)
        self.assertEqual(self.request('POST', body, {'X-Requested-With': 'foreign'})[0], 403)
        self.assertEqual(runtime.commands, 1)
        self.gate.unlink()
        self.assertEqual(self.request('POST', body)[0], 503)
        self.assertEqual(runtime.commands, 1)

    def test_expired_connect_cannot_send_late_command(self):
        self.opened()
        original = http.client.HTTPConnection.connect
        def slow_connect(connection):
            if connection.port == self.upstream.server_port:
                time.sleep(.4)
            return original(connection)
        with patch.object(http.client.HTTPConnection, 'connect', slow_connect):
            try:
                self.request('POST', b'command')
            except (OSError, http.client.HTTPException):
                pass
            time.sleep(.2)
        self.assertEqual(self.upstream.hits, 0)

    def test_upstream_failure(self):
        self.opened()
        self.upstream.shutdown()
        self.upstream.server_close()
        self.assertEqual(self.request()[0], 502)

    def test_oversize(self):
        self.opened()
        self.assertEqual(self.request('POST', headers={'Content-Length': str(gateway.BODY_LIMIT + 1)})[0], 413)
        self.assertEqual(self.upstream.hits, 0)
        self.upstream.payload = b'x' * (gateway.OUTPUT_LIMIT + 1)
        self.assertEqual(self.request()[0], 502)

    def test_bounded_upstream_timeout(self):
        self.opened()
        self.upstream.slow = True
        start = time.monotonic()
        try:
            status = self.request()[0]
            self.assertEqual(status, 502)
        except (OSError, http.client.HTTPException):
            pass  # Deadline may close the client before error reply can be written.
        self.assertLess(time.monotonic() - start, 1)


if __name__ == '__main__':
    unittest.main()

