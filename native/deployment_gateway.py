"""Read-only maintenance gate and bounded fixed-loopback proxy building block."""
import argparse
import http.client
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import re
import socket
import stat
import threading
from urllib.parse import urlsplit

BODY_LIMIT = 1024 * 1024
OUTPUT_LIMIT = 2 * 1024 * 1024
TIMEOUT = 5
HOP_HEADERS = {'connection', 'keep-alive', 'proxy-authenticate', 'proxy-authorization',
               'te', 'trailer', 'transfer-encoding', 'upgrade'}
REQUEST_HEADERS = {'content-type', 'accept', 'authorization', 'origin', 'x-requested-with'}
RESPONSE_HEADERS = {'content-type', 'cache-control', 'www-authenticate',
                    'content-security-policy', 'x-content-type-options',
                    'referrer-policy', 'x-frame-options'}


def gate_open(path, trusted_uid):
    if os.name != 'posix':
        return False
    try:
        descriptor = os.open(path, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK)
        try:
            info = os.fstat(descriptor)
            if (not stat.S_ISREG(info.st_mode) or info.st_size > 1024
                    or info.st_uid != trusted_uid or info.st_mode & 0o022):
                return False
            with os.fdopen(descriptor, 'rb', closefd=False) as source:
                data = source.read(1025)
            if len(data) > 1024:
                return False
            gate = json.loads(data)
            return (isinstance(gate, dict) and set(gate) == {'schema', 'approved', 'accepted_sha'}
                    and type(gate['schema']) is int and gate['schema'] == 1
                    and gate['approved'] is True and isinstance(gate['accepted_sha'], str)
                    and re.fullmatch('[0-9a-f]{40}', gate['accepted_sha']) is not None)
        finally:
            os.close(descriptor)
    except (OSError, ValueError, TypeError):
        return False


def filtered_headers(headers, allowed):
    connection_names = set()
    for value in headers.get_all('Connection', []):
        connection_names.update(name.strip().lower() for name in value.split(','))
    return {name: value for name, value in headers.items()
            if name.lower() in allowed and name.lower() not in HOP_HEADERS | connection_names}


class Handler(BaseHTTPRequestHandler):
    def setup(self):
        super().setup()
        self.connection.settimeout(self.server.timeout_seconds)
        self.upstream = None
        self.upstream_socket = None
        self.expired = threading.Event()
        self.deadline_lock = threading.Lock()
        def expire():
            with self.deadline_lock:
                self.expired.set()
            for connection in (self.connection, self.upstream_socket):
                if connection is not None:
                    try:
                        connection.shutdown(socket.SHUT_RDWR)
                    except OSError:
                        pass
        self.deadline = threading.Timer(self.server.timeout_seconds, expire)
        self.deadline.start()

    def finish(self):
        self.deadline.cancel()
        super().finish()

    def log_message(self, *arguments):
        pass

    def reply(self, status, data=b'', headers=None):
        self.send_response(status)
        for name, value in (headers or {}).items():
            self.send_header(name, value)
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Connection', 'close')
        self.end_headers()
        self.wfile.write(data)
        self.close_connection = True

    def proxy(self):
        try:
            if not gate_open(self.server.gate, self.server.trusted_uid):
                self.reply(503)
                return
            if (self.headers.get_all('Host', []) != [self.server.canonical_host]
                    or (self.command == 'POST' and self.headers.get_all('Origin', [])
                        != ['https://' + self.server.canonical_host])):
                self.reply(403)
                return
            target = urlsplit(self.path)
            if (target.scheme or target.netloc or target.fragment or not self.path.startswith('/')
                    or self.path.startswith('//') or len(self.path) > 2048):
                self.reply(400)
                return
            allowed = {'/', '/index.html', '/app.js', '/style.css', '/api/release', '/api/status', '/api/frame'}
            if ((self.command == 'GET' and target.path not in allowed)
                    or (self.command == 'POST' and target.path != '/api/command')):
                self.reply(404)
                return
            lengths = self.headers.get_all('Content-Length', [])
            if self.headers.get('Transfer-Encoding') or len(lengths) > 1:
                self.reply(400)
                return
            size = int(lengths[0]) if lengths else 0
            if size < 0 or size > BODY_LIMIT:
                self.reply(413)
                return
            body = self.rfile.read(size)
            if len(body) != size:
                self.reply(400)
                return
            # Recheck after bounded body read: maintenance may begin while reading.
            if not gate_open(self.server.gate, self.server.trusted_uid):
                self.reply(503)
                return
            if self.expired.is_set():
                return
            headers = filtered_headers(self.headers, REQUEST_HEADERS)
            headers['Host'] = self.server.canonical_host
            headers['X-Forwarded-Proto'] = 'https'
            self.upstream = http.client.HTTPConnection('127.0.0.1', self.server.origin_port,
                                                       timeout=self.server.timeout_seconds)
            self.upstream.connect()
            with self.deadline_lock:
                if self.expired.is_set():
                    return
                self.upstream_socket = self.upstream.sock
            # The timer can interrupt sends as well as response reads. Never reconnect
            # automatically after expiration: the published socket stays attached.
            if self.expired.is_set():
                return
            self.upstream.request(self.command, self.path, body=body, headers=headers)
            response = self.upstream.getresponse()
            size_header = response.getheader('Content-Length')
            if size_header is not None and (not size_header.isdecimal() or int(size_header) > OUTPUT_LIMIT):
                self.reply(502)
                return
            data = response.read(OUTPUT_LIMIT + 1)
            if len(data) > OUTPUT_LIMIT:
                self.reply(502)
                return
            self.reply(response.status, data, filtered_headers(response.headers, RESPONSE_HEADERS))
        except (OSError, ValueError, http.client.HTTPException):
            try:
                self.reply(502)
            except OSError:
                pass
        finally:
            if self.upstream is not None:
                self.upstream.close()

    do_GET = proxy
    do_POST = proxy


def make_server(gate, origin_port, listen_port, *, canonical_host, trusted_uid, timeout=TIMEOUT):
    if (not isinstance(canonical_host, str) or len(canonical_host) > 253
            or not re.fullmatch(r'[a-z0-9](?:[a-z0-9.-]*[a-z0-9])?', canonical_host)
            or '..' in canonical_host or type(trusted_uid) is not int or trusted_uid < 0):
        raise ValueError('explicit canonical hostname and trusted gate UID required')
    gate = Path(gate)
    if not gate.is_absolute() or gate.resolve() != gate:
        raise ValueError('fixed absolute real gate path required')
    for port in (origin_port, listen_port):
        if type(port) is not int or not 0 <= port <= 65535:
            raise ValueError('invalid loopback port')
    if origin_port == 0 or origin_port == listen_port or not 0 < timeout <= TIMEOUT:
        raise ValueError('invalid gateway bounds')
    server = ThreadingHTTPServer(('127.0.0.1', listen_port), Handler)
    server.gate, server.origin_port, server.timeout_seconds = gate, origin_port, timeout
    server.canonical_host, server.trusted_uid = canonical_host, trusted_uid
    return server


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--gate', required=True)
    parser.add_argument('--origin-port', type=int, required=True)
    parser.add_argument('--listen-port', type=int, required=True)
    parser.add_argument('--canonical-host', required=True)
    parser.add_argument('--trusted-gate-uid', type=int, required=True)
    args = parser.parse_args()
    with make_server(args.gate, args.origin_port, args.listen_port,
                     canonical_host=args.canonical_host, trusted_uid=args.trusted_gate_uid) as server:
        server.serve_forever()


if __name__ == '__main__':
    main()

