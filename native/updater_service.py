"""Unprivileged disposable process adapter; production service isolation is deferred."""
import json
import os
from pathlib import Path
import signal
import subprocess
import time
import sys
import urllib.request
import uuid


def durable_record(path, record):
    temporary = path.with_suffix('.new')
    with temporary.open('w') as output:
        json.dump(record, output)
        output.flush()
        os.fsync(output.fileno())
    os.replace(temporary, path)
    descriptor = os.open(path.parent, os.O_DIRECTORY)
    os.fsync(descriptor)
    os.close(descriptor)


def marked_groups(token):
    groups = set()
    expected = ('CRITTER_UPDATER_SESSION=' + token).encode()
    for directory in Path('/proc').iterdir():
        if not directory.name.isdecimal():
            continue
        try:
            if directory.stat().st_uid != os.getuid():
                continue
            fields = (directory / 'stat').read_text().split(') ')[1].split()
            if fields[0] != 'Z' and expected in (directory / 'environ').read_bytes().split(b'\0'):
                groups.add(int(fields[2]))
        except (FileNotFoundError, ProcessLookupError, PermissionError):
            continue
    return groups


def group_alive(group):
    for directory in Path('/proc').iterdir():
        if not directory.name.isdecimal():
            continue
        try:
            fields = (directory / 'stat').read_text().split(') ')[1].split()
            if int(fields[2]) == group and fields[0] != 'Z':
                return True
        except (FileNotFoundError, ProcessLookupError, PermissionError):
            continue
    return False


class ProcessService:
    def __init__(self, config):
        if os.name != 'posix' or os.geteuid() == 0:
            raise ValueError('disposable adapter requires unprivileged Linux')
        self.config = config
        self.pid_file = Path(config['control']) / 'service.json'
        self.children = []

    def stop(self):
        if not self.pid_file.exists():
            return
        record = json.loads(self.pid_file.read_text())
        token = record.get('token')
        if not isinstance(token, str) or len(token) != 32:
            raise ValueError('unknown service reservation')
        groups, terminated = set(), set()
        graceful = time.monotonic() + self.config['timeout']
        deadline = graceful + self.config['timeout']
        while True:
            groups.update(marked_groups(token))
            if os.getpgrp() in groups:
                raise ValueError('service group overlaps activator')
            active = {group for group in groups if group_alive(group)}
            if not active:
                break
            for group in active:
                if group not in terminated or time.monotonic() >= graceful:
                    try:
                        os.killpg(group, signal.SIGKILL if time.monotonic() >= graceful else signal.SIGTERM)
                    except ProcessLookupError:
                        pass
                    terminated.add(group)
            if time.monotonic() >= deadline:
                raise ValueError('service descendants failed to quiesce')
            time.sleep(.05)
        for child in self.children:
            if child.poll() is not None:
                child.wait()
        self.pid_file.unlink()
        descriptor = os.open(self.pid_file.parent, os.O_DIRECTORY)
        os.fsync(descriptor)
        os.close(descriptor)

    def start(self, release, health=False):
        self.stop()
        token = uuid.uuid4().hex
        # Reservation is durable before spawn; recovery scans inherited session markers.
        durable_record(self.pid_file, {'token': token})
        environment = {'PATH': '/usr/bin:/bin', 'CRITTER_UPDATER_SESSION': token,
                       'CRITTER_DEMO_BIND': '127.0.0.1',
                       'CRITTER_DEMO_PORT': str(self.config['health_port'] if health else self.config['port']),
                       'CRITTER_DEMO_BINARY': str(release / 'bin/critter_lab'),
                       'CRITTER_DEMO_SAVE': self.config['save']}
        process = subprocess.Popen([sys.executable, str(release / 'presenter/server.py')],
                                   env=environment, start_new_session=True,
                                   stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                                   stderr=subprocess.DEVNULL)
        self.children.append(process)
        return process.pid

    def health(self, sha, deployed_at, health=False):
        port = self.config['health_port'] if health else self.config['port']
        deadline = time.monotonic() + self.config['timeout']
        while True:
            try:
                for route in ('/', '/app.js', '/style.css', '/api/release'):
                    with urllib.request.urlopen(f'http://127.0.0.1:{port}{route}', timeout=1) as response:
                        data = response.read(2_000_001)
                        if not data or len(data) > 2_000_000:
                            raise ValueError('invalid health response')
                        if route == '/api/release':
                            release = json.loads(data)
                            if release['commit'] != sha or release['deployed_at'] != deployed_at:
                                raise ValueError('release health mismatch')
                return
            except (OSError, ValueError):
                if time.monotonic() >= deadline:
                    raise ValueError('presenter health failed') from None
                time.sleep(.05)
