"""Signed release notifications become durable work, never HTTP-executed commands."""
import hashlib
import hmac
from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import os
from pathlib import Path
import re
import socket
import tempfile
import threading
import uuid

from updater import REPOSITORY

BODY_LIMIT = 256 * 1024
REPOSITORY_ID = 1388343071


def notification(secret, body, signature, event, delivery):
    if not isinstance(secret, bytes) or len(secret) < 32:
        raise ValueError('webhook secret must contain at least 32 bytes')
    if len(body) > BODY_LIMIT:
        raise ValueError('body exceeds limit')
    expected = 'sha256=' + hmac.new(secret, body, hashlib.sha256).hexdigest()
    if not isinstance(signature, str) or not hmac.compare_digest(signature, expected):
        raise ValueError('invalid signature')
    if len(body) > BODY_LIMIT or event != 'release':
        raise ValueError('unsupported event or body size')
    if str(uuid.UUID(delivery)) != delivery:
        raise ValueError('invalid delivery identity')
    payload = json.loads(body)
    repository = payload.get('repository', {})
    release = payload.get('release', {})
    tag = release.get('tag_name', '')
    if (payload.get('action') != 'published' or repository.get('full_name') != REPOSITORY
            or repository.get('id') != REPOSITORY_ID or release.get('draft') is not False
            or not isinstance(tag, str) or not re.fullmatch('staging-[0-9a-f]{40}', tag)
            or type(release.get('id')) is not int or release['id'] <= 0):
        raise ValueError('unrelated release notification')
    return dict(schema=1, sha=tag[8:], release_id=release['id'], delivery=delivery)


def enqueue(directory, record):
    if os.name != 'posix':
        raise ValueError('durable deployment queue requires Linux')
    directory = Path(directory)
    if directory.is_symlink() or not directory.is_dir():
        raise ValueError('queue must be an existing real directory')
    target = directory / (record['sha'] + '.json')
    descriptor, temporary = tempfile.mkstemp(prefix='.pending-', dir=directory)
    try:
        with os.fdopen(descriptor, 'w') as output:
            json.dump(record, output, sort_keys=True)
            output.flush()
            os.fsync(output.fileno())
        try:
            os.link(temporary, target)
        except FileExistsError:
            existing = json.loads(target.read_text())
            if existing.get('sha') != record['sha'] or existing.get('release_id') != record['release_id']:
                raise ValueError('conflicting release identity')
        descriptor = os.open(directory, os.O_RDONLY)
        try:
            os.fsync(descriptor)
        finally:
            os.close(descriptor)
    finally:
        Path(temporary).unlink(missing_ok=True)


class Handler(BaseHTTPRequestHandler):
    def setup(self):
        super().setup()
        self.connection.settimeout(5)
        def expire():
            try:
                self.connection.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
        # Starts before header parsing; a dripping client cannot reset this deadline.
        self.deadline = threading.Timer(getattr(self.server, 'request_deadline', 5), expire)
        self.deadline.start()

    def finish(self):
        self.deadline.cancel()
        super().finish()

    def log_message(self, *arguments):
        # No delivery bodies, URL query strings or credentials in request logs.
        pass

    def reply(self, code):
        try:
            self.send_response(code)
            self.send_header('Content-Length', '0')
            self.end_headers()
        except OSError:
            # The deadline may already have closed an incomplete request.
            pass

    def do_POST(self):
        if self.path != '/deployment/github':
            self.reply(404)
            return
        try:
            size = int(self.headers.get('Content-Length', '0'))
            if not 1 <= size <= BODY_LIMIT or self.headers.get_content_type() != 'application/json':
                raise ValueError('invalid request size/type')
            if self.headers.get('Transfer-Encoding'):
                raise ValueError('chunked body unsupported')
            body = self.rfile.read(size)
            if len(body) != size:
                raise ValueError('incomplete body')
            record = notification(self.server.secret, body, self.headers.get('X-Hub-Signature-256'),
                                  self.headers.get('X-GitHub-Event'), self.headers.get('X-GitHub-Delivery'))
            enqueue(self.server.queue, record)
        except (ValueError, TypeError, AttributeError, OSError):
            self.reply(400)
            return
        self.reply(202)


def main():
    queue = Path(os.environ['CRITTER_DEPLOY_QUEUE']).resolve(strict=True)
    secret = Path(os.environ['CRITTER_DEPLOY_SECRET_FILE']).read_bytes().strip()
    if len(secret) < 32:
        raise ValueError('invalid webhook secret')
    server = HTTPServer(('127.0.0.1', int(os.environ.get('CRITTER_DEPLOY_HOOK_PORT', '4181'))), Handler)
    server.secret, server.queue = secret, queue
    server.serve_forever()


if __name__ == '__main__':
    main()
