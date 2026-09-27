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

Actions now defines a publication job after all target checks, restricted to trusted main pushes. It creates a new exact-SHA tag and draft release, uploads only the tested bundle and delivery metadata, verifies the tag and rejects stale publication/overwrite. PR/manual runs do not publish. Hosted execution remains the publication acceptance gate.

The intended automatic chain is trusted main push → Actions checks → published release → signed webhook through the existing HTTP route → queued, verified activation. The separate webhook/production adapter is not installed by this increment. A webhook only wakes a worker; it never executes a command or touches player state. No inbound SSH runner access is required.

## Signed notification boundary

`deployment_hook.py` is a separate loopback sidecar for POST `/deployment/github`. It verifies the raw-body GitHub HMAC-SHA256 signature, expected release event/repository and staging tag before fsynced semantic queue insertion. A duplicate release is acknowledged without overwriting its queued identity. The handler has no native command, download or save interface.

Configure `CRITTER_DEPLOY_QUEUE` as an existing protected queue directory, `CRITTER_DEPLOY_SECRET_FILE` as an external file containing the same strong secret as the repository webhook, and optional `CRITTER_DEPLOY_HOOK_PORT` (default 4181). The existing tunnel can route this exact path separately from the gameplay presenter. The sidecar is not installed or exposed automatically by repository code.

`release_delivery.py` independently verifies public release/tag/asset/run metadata and downloads exact digested assets without an artifact credential. It rejects draft/incomplete releases, wrong tags, unsuccessful or unrelated CI, asset mutation and automatic downgrade from a supplied accepted revision. This is a verification function, not an installed activation worker. Publication-run completion may lag the webhook; a production queue worker must retry boundedly without touching the service meanwhile.

Run `python3 native/tests/test_delivery.py` for signature/metadata checks and Linux durable queue/HTTP checks. These use fixture GitHub metadata; they do not establish a configured external webhook or actual public-release download. GitHub does not automatically redeliver failed hooks; installation needs an operator redelivery or reconciliation procedure. See [signature validation](https://docs.github.com/en/webhooks/using-webhooks/validating-webhook-deliveries) and [failed deliveries](https://docs.github.com/en/webhooks/using-webhooks/handling-failed-webhook-deliveries).
## Disposable release preparation

`deployment_worker.py --queue /absolute/queue --output /absolute/output` prepares verified downloads only. Both directories must be private, caller-owned Linux directories containing the disposable marker `critter-lab-disposable\n`; execution as root is refused. The queue uses the signed receiver's existing JSON records. An output lock serializes preparation, and the worker publishes a readiness receipt only after the bundle is durable. Restart deduplication validates the receipt's complete delivery identity and stored bundle digest. Verification failures stop after three attempts; this command does not implement a scheduled retry service.

This worker does not activate releases, read game saves, control services or establish production readiness. A production adapter and host configuration require separate validation.
