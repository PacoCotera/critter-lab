# Recover-first installed-layout source proposal

These templates are review inputs, not an installer or installation-ready setup.
Substitute reviewed numeric identities/ports/hostname and compute exact installed
file hashes. The example configuration intentionally fails validation unchanged.
No credentials or owner-machine values are supplied here.

Runtime and gateway bind loopback. Public hostname traffic must pass through the
fail-closed gateway. Direct LAN access is not required. Runtime, health and check
units are generated only by the fixed adapter and have no independent boot
installation or dependency back to the manager. The manager handles local
accepted-state recovery without network verification. Gate absence is closed.

The external save must already exist as a runtime-owned0600 regular file under a
runtime-owned0700 directory. Recovery never creates or replaces it. Releases,
control, checks, installed tools, configuration and permanent fragments must have
protected root ownership. A persistent namespace must already be established by
a separately reviewed setup/bootstrap operation; production initialization is
not implemented. Only the explicit marked local-test initializer can create a
new test namespace.

Only `recover` is implemented. `apply-request` and `bootstrap-captured` fail
closed; no request loader, capture, timer, hook/worker wiring, install command or
live migration is included. Existing hook and worker sources are unchanged.
Permanent manager/gateway fragments have separate fixed-path hash checks;
loaded runtime/health/check units retain exact transient D-Bus provenance checks.
Permanent drop-ins/configuration and target-version behavior still require
independent installation review. Passing local tests is not installation approval.
