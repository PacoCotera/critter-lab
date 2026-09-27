"""Caller-owned disposable OS backend; no production privilege adapter."""
from contextlib import contextmanager
import fcntl
import os
from pathlib import Path
import signal
import stat
import subprocess
import tempfile

from updater_service import ProcessService

LIMIT = 64 * 1024 * 1024


def regular_bytes(path, limit=LIMIT):
    descriptor = os.open(path, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK)
    try:
        info = os.fstat(descriptor)
        if not stat.S_ISREG(info.st_mode) or info.st_size > limit:
            raise ValueError('regular bounded file required')
        with os.fdopen(descriptor, 'rb', closefd=False) as source:
            data = source.read(limit + 1)
        if len(data) > limit:
            raise ValueError('file exceeds bound')
        return data
    finally:
        os.close(descriptor)


class DisposableEnvironment:
    def __init__(self, config):
        if os.geteuid() == 0 or set(config) != {'root', 'port', 'health_port', 'timeout', 'disposable'}:
            raise ValueError('unprivileged disposable configuration required')
        if config['disposable'] is not True:
            raise ValueError('production adapter is not implemented')
        root = Path(config['root'])
        if not root.is_absolute() or root.resolve() != root:
            raise ValueError('absolute nonsymlink disposable root required')
        if regular_bytes(root / '.disposable', 100) != b'critter-lab-disposable\n':
            raise ValueError('disposable root marker missing')
        for parent in (root, *root.parents):
            if parent.is_symlink():
                raise ValueError('symlink parent rejected')
        if root.stat().st_uid != os.getuid() or root.stat().st_mode & 0o077:
            raise ValueError('disposable root must be private and caller owned')
        for key in ('port', 'health_port'):
            if type(config[key]) is not int or not 1024 <= config[key] <= 65535:
                raise ValueError('invalid loopback port')
        if config['port'] == config['health_port'] or type(config['timeout']) is not int or not 1 <= config['timeout'] <= 30:
            raise ValueError('invalid service bounds')
        self.root = root
        self.releases, self.control = root / 'releases', root / 'control'
        for directory in (self.releases, self.control):
            directory.mkdir(mode=0o700, exist_ok=True)
            if directory.is_symlink() or directory.stat().st_mode & 0o077:
                raise ValueError('unsafe control directory')
        self.save = root / 'state'
        self.service = ProcessService({**config, 'control': str(self.control), 'save': str(self.save)})

    @contextmanager
    def lock(self):
        descriptor = os.open(self.control / 'activation.lock', os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
        try:
            fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            yield
        finally:
            os.close(descriptor)

    @contextmanager
    def save_lock(self):
        descriptor = os.open(str(self.save) + '.lock', os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
        try:
            fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            yield
        finally:
            os.close(descriptor)

    def run_native(self, release, save, *arguments):
        import resource
        def limits():
            resource.setrlimit(resource.RLIMIT_FSIZE, (8 * 1024 * 1024, 8 * 1024 * 1024))
            resource.setrlimit(resource.RLIMIT_CPU, (10, 10))
            resource.setrlimit(resource.RLIMIT_AS, (256 * 1024 * 1024, 256 * 1024 * 1024))
        with tempfile.TemporaryFile() as output, tempfile.TemporaryFile() as errors:
            process = subprocess.Popen([str(release / 'bin/critter_lab'), '--save', str(save), *map(str, arguments)],
                                       stdout=output, stderr=errors, start_new_session=True,
                                       preexec_fn=limits, env={'PATH': '/usr/bin:/bin'})
            try:
                result = process.wait(timeout=10)
            finally:
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                process.wait()
            if result:
                raise ValueError('save compatibility check failed')
            output.seek(0)
            data = output.read(8 * 1024 * 1024 + 1)
            if len(data) > 8 * 1024 * 1024:
                raise ValueError('native output exceeds bound')
            return data

    @contextmanager
    def scratch(self):
        """Private disposable workspace for candidate-written compatibility copies."""
        with tempfile.TemporaryDirectory(dir=self.control) as temporary:
            yield Path(temporary)

