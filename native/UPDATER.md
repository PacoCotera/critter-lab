# CI release delivery

GitHub Actions builds and tests all native targets and publishes the tested Linux Lab bundle. Publication identifies the exact source revision, successful main workflow and asset digests. `release_delivery.py` verifies this provenance and `package_staging.py` checks the bounded package contents; `updater.py` supplies shared HTTPS/provenance helpers.

Staging uses the existing Ubuntu presenter service. Private operational tooling downloads a verified release before changing the running service, switches the release path, restarts and checks health and reported revision. Failure restores the previous release. Persistent game state remains separate from executable releases. Initial staging state may be reset when a fresh demo is explicitly requested.

There is no custom activation manager, maintenance gateway, webhook queue or systemd runtime generator. Host identities, routing, installation scripts and credentials stay outside the public product repository. A service restart causes a brief interruption. Save-format changes require a separate compatibility decision before deployment.