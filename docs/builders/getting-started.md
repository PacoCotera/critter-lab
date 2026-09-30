# Run Beecho Lab locally

The current playable target is the [native three-device simulator](../../native/selected-lab/README.md):
Lab, combined Companion and Dock share one saved world in a C17 host process.
The browser transports physical-control events and displays native frames.
[Build coverage](../../BUILD.md) distinguishes that software from unfinished
firmware, hardware, cloud and mobile work.

## Current native simulator

Use Git, GCC, CMake 3.28 or later, Ninja and Python 3.12 on the established Linux
build host. The [native guide](../../native/README.md) records toolchains and
legacy MCU builds. Retrieve a clean committed source revision through Git; source,
executable and presenter must match. This does not require Node/browser-game
installation or an ESP-IDF Lab build.

```sh
git clone https://github.com/PacoCotera/critter-lab.git
cd critter-lab
cmake -S native/lab -B native/build/lab -G Ninja \
  -DCRITTER_BUILD_SELECTED_LAB=ON -DCMAKE_BUILD_TYPE=Release
cmake --build native/build/lab
ctest --test-dir native/build/lab --output-on-failure
python3 native/tests/test_selected_presenter.py native/build/lab/selected-lab/selected_lab
```

From the repository root, select an absolute writable save path outside release
bundles. The save and its `.kit` / `.kit.required` sidecars form one world.
Preserve and back them up together; see [recovery boundaries](../../specs/architecture.md#three-device-host-simulator).

```sh
export CRITTER_DEMO_BINARY="$PWD/native/build/lab/selected-lab/selected_lab"
export BEECHO_V1_SAVE="$HOME/beecho-saves/play-world"
mkdir -p "$(dirname "$BEECHO_V1_SAVE")"
python3 native/presenter/server.py
```

Open `http://127.0.0.1:4180`. Stop with Ctrl+C. The default presenter uses
`kit-serve`; legacy single-Lab `serve` remains a regression fixture. The older
`native/build/lab/critter_lab` domain CLI is a separate protocol, even though the
release bundle uses `bin/critter_lab` as the selected executable's packaged name.

Follow the [Pip play guide](../../native/selected-lab/V1.md). Only Companion starts
expeditions; Lab accepts returns and researches/creates. Device buttons drive
native focus and actions. External link switches and Reset sandbox are simulator
administration, not additional device controls. Reset keeps the matching world
and sidecars in a uniquely named backup; it has no browser restore endpoint.

For a changed connected journey, use the focused
`python3 native/tests/test_kit_presenter.py BINARY` check and the full timed
`python3 native/tests/test_v1_journey.py BINARY FRAME_DIRECTORY` journey as
appropriate. Reset behavior has
`python3 native/tests/test_sandbox_reset.py BINARY`. The target's
[guide](../../native/selected-lab/README.md) records current interfaces and limits.
Documentation-only changes need link/command inspection, not another game run.

## Historical browser and domain studies

These are separate experiments with separate persistence. They do not form an
alternative integrated game or a physical hardware simulator. Read each component
boundary before changing it. Node.js 22 or later and npm run the browser studies:

```sh
npm ci
npm start
```

Open `http://127.0.0.1:4173`. Dependencies are pinned in `package-lock.json`.

| Route | Historical study |
| --- | --- |
| `/` | [Breeding, research and share-card experiment](../../prototype/README.md) |
| `/lab/` | [Authored sample-to-founder fixture](../../prototype/lab/README.md) |
| `/transfer/` | [Transfer-status supplied observations](../../prototype/transfer/browser/README.md) |
| `/review.html` | [Display and idle studies](../../prototype/README.md) |

The [Pip genetic-content proof](../../prototype/genetics/README.md) has a
[generated report](../../prototype/genetics/report.md). Run
`node prototype/genetics/report.mjs` to regenerate it; its focused check is
`node --test prototype/tests/pip-genetics.test.mjs`. The report creates no living
individual.

The [headless expedition study](../../prototype/expedition/README.md) uses fixed
inputs and historical whole-pack/retained-partial arithmetic. Current accepted
inventory is whole indivisible items; old study values are not current gameplay.
Its guide owns demo commands, simulated observation comparisons and scoped tests.

Older breeding data lives in browser local storage; the Lab study uses separate
IndexedDB storage; transfer-status screens have no live transfer persistence.
Headless demos and share snapshots write to ignored `prototype/.data/` paths.
A changed browser/origin can show another collection. Do not delete unfamiliar
save data to repair an error; corrupt/unsupported saves are rejected.

Run `npm test` for changed host behavior, or the component's focused checks.
Optional Playwright/Chromium harnesses are separate from normal local play.

## Connection and access

The native presenter uses `CRITTER_DEMO_BIND` and `CRITTER_DEMO_PORT` for binding,
with loopback and port 4180 as defaults. Optional `CRITTER_DEMO_PASSWORD` enables
HTTP Basic access as username `lab`; use HTTPS for access beyond local inspection.
All visitors share the same simulator world. The existing
[CI bundle guide](../../native/UPDATER.md) records release provenance.

The historical Node server uses `CRITTER_HOST` / `CRITTER_PORT`, with loopback and
4173 as defaults. An explicitly configured trusted-LAN
`CRITTER_PUBLIC_ORIGIN` may support share-card inspection. Its share endpoints
have no player authentication; do not port-forward that study as a public game
service. Phone scanning and physical printing remain unvalidated.
