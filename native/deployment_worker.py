"""Disposable preparation only: no activation, saves or production readiness."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import uuid
from release_delivery import FIELDS, verified_release
from updater import GitHubClient, REPOSITORY, WORKFLOW


def read(directory, name, limit=16384):
    descriptor = os.open(name, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK, dir_fd=directory)
    try:
        if not stat.S_ISREG(os.fstat(descriptor).st_mode) or os.fstat(descriptor).st_size > limit:
            raise ValueError('invalid bounded file')
        with os.fdopen(descriptor, 'rb', closefd=False) as source:
            data = source.read(limit + 1)
        if len(data) > limit:
            raise ValueError('oversized file')
        return data
    finally:
        os.close(descriptor)


def write(directory, name, data):
    temporary = '.pending-' + uuid.uuid4().hex
    descriptor = os.open(temporary, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600, dir_fd=directory)
    try:
        with os.fdopen(descriptor, 'wb') as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        os.rename(temporary, name, src_dir_fd=directory, dst_dir_fd=directory)
        os.fsync(directory)
    finally:
        try:
            os.unlink(temporary, dir_fd=directory)
        except FileNotFoundError:
            pass


def directory(path):
    path = Path(path)
    if not path.is_absolute() or path.resolve() != path or path.is_symlink():
        raise ValueError('absolute real directory required')
    descriptor = os.open(path, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
    try:
        info = os.fstat(descriptor)
        if info.st_uid != os.getuid() or info.st_mode & 0o077:
            raise ValueError('private caller-owned directory required')
        if read(descriptor, '.disposable') != b'critter-lab-disposable\n':
            raise ValueError('disposable marker required')
        return descriptor
    except Exception:
        os.close(descriptor)
        raise


def validate_receipt(receipt, record):
    expected_fields = {'schema', 'status', 'sha', 'release_id', 'bundle_sha256', 'delivery'}
    if (not isinstance(receipt, dict) or set(receipt) != expected_fields
            or type(receipt['schema']) is not int or receipt['schema'] != 1
            or receipt['status'] != 'prepared' or receipt['sha'] != record['sha']
            or type(receipt['release_id']) is not int or receipt['release_id'] != record['release_id']
            or not isinstance(receipt['bundle_sha256'], str)
            or not re.fullmatch('[0-9a-f]{64}', receipt['bundle_sha256'])):
        raise ValueError('invalid prepared receipt')
    delivery = receipt['delivery']
    if (not isinstance(delivery, dict) or set(delivery) != FIELDS
            or type(delivery['schema']) is not int or delivery['schema'] != 1
            or delivery['repository'] != REPOSITORY or delivery['workflow'] != WORKFLOW
            or delivery['sha'] != record['sha']
            or delivery['bundle'] != 'critter-lab-staging-' + record['sha'] + '.tar.gz'
            or delivery['bundle_sha256'] != receipt['bundle_sha256']
            or delivery['save_format'] != 'CRITTER_DEMO 1'
            or type(delivery['run_id']) is not int or delivery['run_id'] <= 0
            or type(delivery['attempt']) is not int or delivery['attempt'] <= 0):
        raise ValueError('invalid saved delivery metadata')

def prepare_one(queue, output, name, client):
    record = json.loads(read(queue, name))
    if (not isinstance(record, dict) or set(record) != {'schema', 'sha', 'release_id', 'delivery'}
            or type(record['schema']) is not int or record['schema'] != 1
            or not isinstance(record['sha'], str) or not re.fullmatch('[0-9a-f]{40}', record['sha'])
            or name != record['sha'] + '.json'
            or type(record['release_id']) is not int or record['release_id'] <= 0
            or not isinstance(record['delivery'], str)
            or str(uuid.UUID(record['delivery'])) != record['delivery']):
        raise ValueError('invalid queue identity')
    sha = record['sha']
    receipt_name = sha + '.prepared.json'
    try:
        receipt = json.loads(read(output, receipt_name))
    except FileNotFoundError:
        receipt = None
    if receipt is not None:
        validate_receipt(receipt, record)
        bundle = read(output, sha + '.tar.gz', 64 * 1024 * 1024)
        if hashlib.sha256(bundle).hexdigest() != receipt.get('bundle_sha256'):
            raise ValueError('damaged prepared bundle')
        return 'duplicate'
    failure_name = sha + '.failure.json'
    try:
        failure = json.loads(read(output, failure_name))
        attempts = failure['attempts']
        if (set(failure) != {'sha', 'release_id', 'attempts', 'status'}
                or failure['sha'] != sha or failure['release_id'] != record['release_id']
                or type(attempts) is not int or not 1 <= attempts <= 3
                or failure['status'] != 'verification-failed'):
            raise ValueError('invalid retry state')
    except FileNotFoundError:
        attempts = 0
    if attempts == 3:
        return 'exhausted'
    try:
        delivery, bundle = verified_release(client, record['release_id'], sha)
    except (ValueError, TypeError, KeyError, AttributeError, OSError):
        failure = dict(sha=sha, release_id=record['release_id'], attempts=attempts + 1,
                       status='verification-failed')
        write(output, failure_name, json.dumps(failure, sort_keys=True).encode())
        return 'retry' if attempts + 1 < 3 else 'exhausted'
    write(output, sha + '.tar.gz', bundle)
    receipt = dict(schema=1, status='prepared', sha=sha, release_id=record['release_id'],
                   bundle_sha256=hashlib.sha256(bundle).hexdigest(), delivery=delivery)
    # Receipt publication commits readiness; a lone bundle is never ready.
    write(output, receipt_name, json.dumps(receipt, sort_keys=True).encode())
    return 'prepared'


def prepare(queue_path, output_path, client=None):
    if os.name != 'posix' or os.geteuid() == 0:
        raise ValueError('unprivileged Linux execution required')
    import fcntl
    queue = directory(queue_path)
    try:
        output = directory(output_path)
        try:
            queue_info, output_info = os.fstat(queue), os.fstat(output)
            if (queue_info.st_dev, queue_info.st_ino) == (output_info.st_dev, output_info.st_ino):
                raise ValueError('distinct directories required')
            lock = os.open('.prepare.lock', os.O_RDWR | os.O_CREAT | os.O_NOFOLLOW | os.O_NONBLOCK,
                           0o600, dir_fd=output)
            try:
                if not stat.S_ISREG(os.fstat(lock).st_mode):
                    raise ValueError('invalid lock')
                fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
                client = client or GitHubClient(None, public=True)
                names = sorted(name for name in os.listdir(queue) if name.endswith('.json'))
                if len(names) > 100:
                    raise ValueError('batch limit exceeded')
                results = {}
                for name in names:
                    try:
                        results[name] = prepare_one(queue, output, name, client)
                    except (ValueError, TypeError, KeyError, AttributeError, OSError):
                        results[name] = 'invalid'
                return results
            finally:
                os.close(lock)
        finally:
            os.close(output)
    finally:
        os.close(queue)


def main():
    parser = argparse.ArgumentParser(description='Prepare only: disposable downloads, no activation or production readiness.')
    parser.add_argument('--queue', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    results = prepare(args.queue, args.output)
    print(json.dumps(results, sort_keys=True))
    return 0 if all(value in ('prepared', 'duplicate') for value in results.values()) else 1


if __name__ == '__main__':
    raise SystemExit(main())



