# Native release delivery

This increment implements manual GitHub artifact retrieval and a **disposable Linux activation rehearsal**. It does not install a production updater or configure the owner route. Never point this adapter at an existing player save or live service.

`updater.py` owns promotion validation, HTTPS retrieval and ZIP selection; `updater_activation.py` owns Linux admission, save compatibility, selection and recovery; `updater_service.py` owns the disposable process adapter. Install the reviewed tools together outside candidate trees. Candidate code never supplies the admission verifier or runs elevated.

## Promotion and configuration

A promotion JSON has exactly `schema: 1`, `repository: "PacoCotera/critter-lab"`, `workflow: ".github/workflows/native-build.yml"`, `sha` (full lowercase SHA), positive integer `run_id`, `attempt`, `artifact_id`, `artifact_name: "native-lab-<SHA>"`, and lowercase 64-character `archive_sha256`/`bundle_sha256`. It contains no paths, commands or credentials. The operator selects the exact release and digests.

Fetch and activation independently verify successful main-push provenance, exact repository/workflow/SHA/current attempt, artifact association and creation time within that attempt, expiry/digest and continued main ancestry. PR/manual runs are rejected. HTTPS credentials stay on the API origin; redirected requests exclude them. ZIP and complete expanded tar bytes are bounded before parsing hidden tar headers.

Supply an external read-only GitHub credential through `CRITTER_UPDATER_TOKEN`, never command arguments, JSON or logs. See GitHub's [artifact API](https://docs.github.com/en/rest/actions/artifacts), [workflow-run API](https://docs.github.com/en/rest/actions/workflow-runs) and [commit comparison API](https://docs.github.com/en/rest/commits/commits#compare-two-commits). Production credential storage/rotation is not configured here.

Create a new caller-owned disposable directory, mode `0700`, with `.disposable` containing exactly `critter-lab-disposable` followed by a newline. Configuration has exactly `root` (absolute real path), distinct unused loopback `port`/`health_port` above 1023, integer `timeout` (1–30 seconds), and `disposable: true`. State is fixed at `<root>/state`; releases, incoming and control files stay inside that directory. Seed state with a trusted native executable first. Missing/corrupt saves fail closed. Root execution is rejected.

```sh
python3 native/updater.py fetch --config disposable.json --promotion promotion.json
python3 native/updater.py activate --config disposable.json --promotion promotion.json
python3 native/updater.py rollback --config disposable.json --sha RETAINED_ACCEPTED_SHA
python3 native/updater.py recover --config disposable.json
```

The original CI archive/manifest remain unchanged. Injected `release.json` preserves verified source fields and has separately hashed activation metadata. Actual activation time uses `America/Mexico_City`; acceptance time is distinct. Previously activated releases require explicit retained-release selection; rollback preserves their activation time.

The adapter stops marked process groups before acquiring the save lock. Copied-save checks preserve revision, exact retry result and native frames; representative candidate-written saves remain readable by the old binary. No updater check restores a backup or writes the configured save. Durable unaccepted transitions recover a compatible old release; accepted transitions recover the candidate. Corrupt/unknown state or incompatible saves leave service stopped for intervention.

## Verification and production limits

```sh
python3 native/tests/test_updater.py
python3 native/tests/rehearse_updater.py PATH_TO_VERIFIED_STAGING_BUNDLE
```

Rehearsal SHAs are explicit test identities derived from a supplied bundle, not source provenance. Checks use real Linux processes/locks and SIGKILL recovery, not reboot or physical power loss.

The disposable adapter shares an unprivileged account with candidate code. Session markers track cooperating descendants, including detached groups; deliberately marker-stripping hostile code is outside this adapter's protection. It is not a security sandbox. Private loopback has no unrelated callers; it is not a production maintenance gate. Production installation must separate downloader/activator/runtime permissions, quiesce the full service cgroup, gate owner traffic until durable acceptance and recover before normal startup. Passing this rehearsal does not establish production deployment readiness.

Actions defines a publication job after all target checks, restricted to trusted main pushes. It creates a new exact-SHA tag and draft release, uploads only the tested bundle and delivery metadata, verifies the tag and rejects stale publication/overwrite. PR/manual runs do not publish. [The first main publication completed successfully](https://github.com/PacoCotera/critter-lab/actions/runs/36292656424); an external webhook delivery and live activation remain separate acceptance gates.

The intended automatic chain is trusted main push → Actions checks → published release → signed webhook through the existing HTTP route → queued, verified activation. The separate webhook/production adapter is not installed by this increment. A webhook only wakes a worker; it never executes a command or touches player state. No inbound SSH runner access is required.

## Signed notification boundary

`deployment_hook.py` is a separate loopback sidecar for POST `/deployment/github`. It verifies the raw-body GitHub HMAC-SHA256 signature, expected release event/repository and staging tag before fsynced semantic queue insertion. A duplicate release is acknowledged without overwriting its queued identity. The handler has no native command, download or save interface.

Configure `CRITTER_DEPLOY_QUEUE` as an existing protected queue directory, `CRITTER_DEPLOY_SECRET_FILE` as an external file containing the same strong secret as the repository webhook, and optional `CRITTER_DEPLOY_HOOK_PORT` (default 4181). The existing tunnel can route this exact path separately from the gameplay presenter. The sidecar is not installed or exposed automatically by repository code.

`release_delivery.py` independently verifies public release/tag/asset/run metadata and downloads exact digested assets without an artifact credential. It rejects draft/incomplete releases, wrong tags, unsuccessful or unrelated CI, asset mutation and automatic downgrade from a supplied accepted revision. This is a verification function, not an installed activation worker. Publication-run completion may lag the webhook; a production queue worker must retry boundedly without touching the service meanwhile.

Run `python3 native/tests/test_delivery.py` for signature/metadata checks and Linux durable queue/HTTP checks. These use fixture GitHub metadata; they do not establish a configured external webhook or actual public-release download. GitHub does not automatically redeliver failed hooks; installation needs an operator redelivery or reconciliation procedure. See [signature validation](https://docs.github.com/en/webhooks/using-webhooks/validating-webhook-deliveries) and [failed deliveries](https://docs.github.com/en/webhooks/using-webhooks/handling-failed-webhook-deliveries).
## Disposable release preparation

The activation transaction delegates paths, locks, compatibility scratch directories, service operations and native execution to `DisposableEnvironment`. `Activation(config)` keeps the disposable guards; `Activation.from_environment(environment)` supports explicit backend injection while still refusing root. This boundary does not provide a production environment or privileged adapter.

`deployment_worker.py --queue /absolute/queue --output /absolute/output` prepares verified downloads only. Both directories must be private, caller-owned Linux directories containing the disposable marker `critter-lab-disposable\n`; execution as root is refused. The queue uses the signed receiver's existing JSON records. An output lock serializes preparation, and the worker publishes a readiness receipt only after the bundle is durable. Restart deduplication validates the receipt's complete delivery identity and stored bundle digest. Verification failures stop after three attempts; this command does not implement a scheduled retry service.

This worker does not activate releases, read game saves, control services or establish production readiness. A production adapter and host configuration require separate validation.

## Maintenance gateway building block

Run `python3 native/deployment_gateway.py --gate /absolute/trusted/gate.json --origin-port 4180 --listen-port 4182 --canonical-host demo.example --trusted-gate-uid 0` only against a separately configured local runtime. The gateway binds loopback, never writes the gate, and returns 503 without contacting the runtime when the gate is missing or invalid. An open gate has exactly `schema: 1`, `approved: true`, and `accepted_sha` containing a full lowercase source SHA. The gate must be readable, owned by the configured trusted UID and not group/world writable; installation must also protect every parent directory and keep the gateway outside replaceable release code.

This gateway expects an HTTPS tunnel with a fixed canonical hostname. It rejects other Host values and foreign POST origins, preserves Origin, and supplies fixed canonical Host and HTTPS protocol to the presenter. Existing authentication, JSON/request-marker checks and browser protection headers remain active. It is a bounded proxy building block, not an installed maintenance/acceptance manager: the trusted adapter still must close the gate, drain runtime requests and open it only after durable acceptance and health. `native/tests/test_deployment_gateway.py` exercises real loopback transport and the presenter's same-origin policy; it does not establish live routing or boot recovery.

## Disposable systemd backend

`updater_systemd.py` provides a root-owned **local test building block**, with no production entrypoint. Default and general injected Activation entrypoints retain their root guards. It requires a new private marked root, explicitly nonzero runtime/check identities, protected admitted files, the fixed `critter-lab-updater-test-` unit namespace and a separately readable protected test gate directory. A lifetime lock excludes another environment; initialization refuses existing units. Explicit close releases ownership and prevents further operations. Fresh-process recovery is available only through the explicit disposable reopen seam described below.

Compatibility commands execute through a fixed interpreter/execv shim after systemd sets the check identity and mounts the release read-only. Presenter execution uses the runtime identity; state is the only writable runtime bind. The backend closes the shared maintenance gate before stopping services, verifies cgroup quiescence, and opens it only through an explicit acceptance call. The exact `Activation.from_disposable_systemd(environment)` test seam integrates the existing transaction under one activation lock through gate acceptance. It requires the held disposable environment. Another process must use the explicit verified reopen seam; boot recovery is not established.

Run `python3 -m unittest discover -s native/tests -p test_systemd_backend.py` for configuration/command checks without launching units. Local real-systemd rehearsal additionally exercised native status/play, presenter health, observed nonzero runtime UID, gate lifecycle and unchanged fixture state. It did not establish production boot/recovery, a legacy baseline migration or live deployment. Root test fixtures must refuse preexisting paths/units and preserve failed evidence.

Run python3 -m unittest discover -s native/tests -p test_systemd_activation.py for transaction guards, acceptance exclusion, FIFO rejection and restrictive-umask admission. The explicit root-only native/tests/rehearse_systemd_activation.py harness requires a trusted bundle, new marked fixture root and separate nonzero runtime/check identities. Its local systemd rehearsal passed update, play-preserving rollback, failed-update gate closure and same-process recovery under umask077. This evidence does not establish cross-process recovery, reboot recovery, legacy migration or production installation.



## Disposable restart recovery

`DisposableSystemdEnvironment.reopen_disposable_test(config)` reacquires the protected test namespace under an exclusive lifetime lock and closes the gate before inspecting services. It checks the durable namespace/configuration and launch intents, exact transient unit files, identities, commands, environment, mounts and sandbox properties before stopping any loaded unit. Unknown or mismatched services remain untouched and access remains gated. Recovery reuses `Activation.recover()`.

Run `python3 -m unittest discover -s native/tests -p test_systemd_restart.py` for ownership and validation guards. The root-only `native/tests/rehearse_systemd_restart.py` harness requires a trusted bundle, new marked root and separate nonzero identities. A local systemd rehearsal passed nine fresh-process SIGKILL boundaries, save/receipt preservation, surviving check descendants, real gateway maintenance/recovery responses and duplicate reopener exclusion. Synthetic bundles and native wrappers are labeled fixtures, not published releases.

This is explicit restart recovery: a crash after the gate opens leaves it open until a reopener closes it. No automatic supervisor, reboot/power-loss proof, legacy bootstrap or production installation is provided. Setup-failure cleanup was independently source reviewed after the successful rehearsal; that branch was not exercised by the successful run.

## Authored legacy-layout fixture

`native/tests/rehearse_legacy_bootstrap.py` is a root disposable test harness, not a live migration tool. It accepts a trusted test bundle and explicitly authored historical metadata, preserves exact executable/presenter assets through normalization, retains historical metadata separately, and records `local-legacy-fixture` provenance. The fixture SHA mirrors an older layout; its code bytes are not asserted to be that historical installation or its CI artifact.

Run `python3 -m unittest discover -s native/tests -p test_legacy_bootstrap.py` for eight integrity/guard checks. The actual local fixture rehearsal passed initial health failure staying gated, SIGKILL before acceptance, fresh-process recovery after acceptance, and later update/play/rollback preserving progress. Bootstrap itself never writes the supplied state; historical time remains separate from actual acceptance time.

Reset of a failed unaccepted fixture is explicit operator test cleanup, validates all evidence before removal, and is not automatic recovery. This evidence does not authorize live capture, establish actual historical executable compatibility or provide production bootstrap/installation.

## Recover-only installed-layout adapter

`activation_manager.py recover` uses the fixed installed profile, protected configuration/tool hashes, permanent fragment hashes and exact transient provenance. `Activation.from_installed_environment()` is an explicit exact-class root seam; default and general disposable root guards remain. Recovery requires an already existing runtime-owned0600 save under a0700 directory and never creates or replaces it. Runtime and gateway bind loopback; public traffic must pass through the gateway.

The placeholder configuration and manager/gateway templates in `native/deployment/` are review inputs, not an installer. A persistent namespace must already exist; only the explicitly marked local test profile can initialize one. `apply-request` and `bootstrap-captured` fail closed and are not implemented. No hook/worker/timer wiring is included.

Run `python3 -m unittest discover -s native/tests -p test_installed_environment.py` for eleven focused guards. `native/tests/rehearse_installed_recovery.py` refuses existing fixed test paths and accepts save input only from a marked disposable fixture. Local template verification, network-independent recovery, unchanged external fixture save and recovery after absent gate/transients with simulated boot identity passed. An initial static-unit lookup race failed before runtime creation; a single bounded permanent-property snapshot corrected it and the fresh fixture passed.

This evidence excludes actual boot startup, public gateway routing, real reboot/power loss, target-host version behavior, request application, captured bootstrap and live installation. Existing disposable entrypoints and transaction remain supported.
