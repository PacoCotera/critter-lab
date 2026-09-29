# Development sequence

Read the [electronics-first reference](../../specs/devices.md#electronics-first-v1-reference-specification) before new device work. Validate the connected game through existing host/native tools; do not infer complete firmware from MCU fixture builds. New physical profiles remain proposals; the mobile fallback shares domain contracts and records. The commands below run existing experiments, not the proposed full family.

# Run the local experiments

For builders and developers. These instructions run the existing host software; there is no complete kit to assemble or production service to deploy yet. See [build coverage](../../BUILD.md).

For native Lab/MCU development, use the [native build guide](../../native/README.md). The browser experiments below are separate historical studies, not hardware simulators.

## Install and start

You need Git, Node.js 22 or later with npm, and a current browser. No hardware, account or private tooling is required.

```sh
git clone https://github.com/PacoCotera/critter-lab.git
cd critter-lab
npm ci
npm start
```

Open `http://127.0.0.1:4173`. Stop the server with Ctrl+C. There is no compilation step; dependencies are pinned in `package-lock.json`.

| Route | What to expect | Details |
| --- | --- | --- |
| `/` | Older breeding, research and share-card experiment | [Prototype](../../prototype/README.md) |
| `/lab/` | Authored sample-to-founder pixel fixture | [Lab](../../prototype/lab/README.md) |
| `/transfer/` | Transfer-status screens using supplied observations | [Transfer browser study](../../prototype/transfer/browser/README.md) |
| `/review.html` | Color/monochrome/six-color and idle studies | [Prototype](../../prototype/README.md) |

These experiments are separate. An action in one does not form an integrated game progression across the others.

## Pip genetic-content proof

The approved bounded [Pip proof](../../prototype/genetics/README.md) runs separately from these older browser experiments. Its [generated report](../../prototype/genetics/report.md) connects inherited baseline and allele contributions, partial research knowledge and one compatible child. Run `node prototype/genetics/report.mjs` to regenerate it and `node --test prototype/tests/pip-genetics.test.mjs` for its acceptance checks. It creates no living individual and is not wired into the demo.

## Saved data

The older breeding experiment uses browser local storage. The Lab uses a separate IndexedDB database and preserves prior isolated runs. Transfer-status screens have no live transfer or persistence. Published share snapshots are local JSON files in ignored `prototype/.data/`; headless demos write to the new directory you supply. Browser data belongs to that browser and origin, not a cloud account. Changing ports or browsers can therefore show a different local collection.

Use a demo's explicit reset/new-run controls when needed. Do not delete an unfamiliar data directory to fix an error. Corrupt or unsupported saves are rejected rather than automatically replaced.

## Checks and exports

Run the host suite when changing behavior:

```sh
npm test
```

For documentation-only changes, inspect links and commands instead. Component READMEs list focused checks. Optional browser harnesses need Playwright and Chromium installed separately; they are not needed to play these local experiments or run the normal host suite.

| Command from repository root | Output |
| --- | --- |
| `node prototype/lab/export.mjs` | Lab PNG frames in `prototype/lab/artifacts/` |
| `node prototype/pixel/export.mjs` | Static pixel studies in `design/reviews/pixel-01/` |
| `node prototype/transfer/demo.mjs <new-directory>` | Two saved replicas and interrupted-transfer trace |
| `node prototype/compatibility/demo.mjs <new-directory>` | Four byte-preserving saved-record copies |

Replace `<new-directory>` with an unused path. Exports can update generated artifacts; inspect Git changes before committing them.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| `node` or `npm` not found | Install Node.js with npm and reopen the terminal; check `node --version` and `npm --version` |
| Missing package | Run `npm ci` at the repository root |
| Port already in use | Stop your earlier server, or set `CRITTER_PORT` to an unused port |
| Browser cannot connect | Keep the server running and use its printed URL; check the port |
| A finding never arrives | The Lab is an authored fixture; use its external supply-finding control |
| QR will not open on a phone | Loopback is local to each device; the optional LAN setup is below |

Alternate port in PowerShell: `$env:CRITTER_PORT='4174'` followed by `npm start`. In a POSIX shell: `CRITTER_PORT=4174 npm start`. The environment override lasts for the shell/session as defined by your shell.

## Optional local-network inspection

Only for a trusted LAN: set `CRITTER_HOST` to `0.0.0.0` and `CRITTER_PUBLIC_ORIGIN` to `http://<computer-LAN-IP>:4173`, then restart the server. Use that same origin on both devices when creating and opening share cards. The configured origin must contain only scheme, hostname and optional port. Firewall access may be required; the app does not configure it.

The prototype has writable share endpoints and no player authentication. Do not expose it as a public game server or port-forward it. Phone scanning and physical printing remain unvalidated; a browser preview is not printer calibration. No cloud deployment or firmware-flashing instructions exist yet.
