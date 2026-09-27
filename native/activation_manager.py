"""Recover-first installed-layout manager; no installer or release consumer.

Only accepted local recovery is implemented. Request application and captured
bootstrap deliberately fail closed pending their separately reviewed slices.
"""
import argparse
from pathlib import Path
import sys

from updater_activation import Activation
from updater_systemd import InstalledSystemdEnvironment, INSTALL_PROFILES


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('operation', choices=('recover', 'apply-request', 'bootstrap-captured'))
    parser.add_argument('--local-test-profile', action='store_true',
                        help='Use only the compiled, marked disposable installed-layout profile.')
    args = parser.parse_args()
    profile = 'local-installed-test' if args.local_test_profile else 'installed'
    layout = INSTALL_PROFILES[profile]
    if Path(__file__).resolve() != Path(layout['tools']) / 'activation_manager.py':
        raise ValueError('manager must execute from fixed verified installed tools')
    with InstalledSystemdEnvironment(layout['config']) as environment:
        if args.operation != 'recover':
            environment.begin_maintenance()
            raise NotImplementedError(args.operation + ' is not implemented; accepted recovery is the only supported operation')
        activation = Activation.from_installed_environment(environment)
        selected = activation.recover()
        print('accepted local recovery: ' + selected)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, TypeError, KeyError, NotImplementedError) as error:
        print('activation manager refused: ' + str(error), file=sys.stderr)
        raise SystemExit(1)
