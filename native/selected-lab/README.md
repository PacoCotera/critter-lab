# Native Beecho Lab V1

The current [playable loop and limits](V1.md) are authoritative for this target.
Native C17 owns the game, saved world, focus and input authorization. LVGL
composes migrated device screens into native frames; remaining manual families
are tracked in [architecture coverage](../../specs/architecture.md#current-migration-coverage-and-target-evidence).
The browser transports fixed simulated hardware button events
and displays native BMP frames. It does not implement the game or screen layout.

## Platform

The selected Lab reference is **Raspberry Pi4 Model B**, with the Waveshare 7inch
HDMI LCD (H), 1024×600. The existing build is Linux **x86-64 host simulation** using
GCC, CMake and Ninja. It is not an ESP32/ESP-IDF executable, Pi emulation or a
verified ARM build. Physical HDMI/input integration, board boot and performance
remain untested. Companion and Caddy target ESP32/ESP-IDF. The shared current Dock
UI has a headless ESP-IDF compile proof; current Companion UI compilation remains open.

Develop locally, commit and push, then fetch the exact clean revision through Git
on the established VM. Never copy loose source to bypass version control.

```sh
cmake -S native/lab -B native/build/lab -G Ninja -DCRITTER_BUILD_SELECTED_LAB=ON -DCMAKE_BUILD_TYPE=Release
cmake --build native/build/lab
ctest --test-dir native/build/lab --output-on-failure
python3 native/tests/test_selected_presenter.py native/build/lab/selected-lab/selected_lab
python3 native/tests/test_v1_journey.py native/build/lab/selected-lab/selected_lab native/build/lab/journey
```

The legacy `critter_lab` executable remains a separate earlier fixture. CI packages
the tested **selected_lab** executable and matching production presenter through
the established release path. Its packaged filename may be `critter_lab`; that
filename does not make it the legacy fixture.

## Physical-control contract

The current panel follows the [family reference](../../design/lab-controls/combined-family-materials.png):
directional cross at left, four labeled workspace keys, Back then Confirm at right.
No knob or clickable screen targets. [Experience specification](../../specs/experience.md#simulated-console-controls--accepted)
owns action mapping and safety. Native input requires a fresh press/release against
the visible ready revision; cancellation, overlapping presses and suspension cannot
carry an armed action into another workspace. Right is read-only navigation.

The line protocol accepts `status`, `frame REVISION`, `ready REVISION`,
`cancel REVISION`, `suspend REVISION`, `resume REVISION`, and button phases such as
`up-down REVISION` / `up-up REVISION`. The ten button names are up, down, left,
right, research, critters, library, habitat, back and confirm. Legacy rotation is
not accepted. Frame responses contain byte count followed by BMP bytes; Python
and JavaScript do not decide focus, destinations or gameplay consequences.

## Artwork and rendering

`render.c` emits RGB888 scanlines and 24-bit BMP frames. Existing licensed font
sources, extracted Gemini UI assets and [Pip artwork](../../design/v1-pip/manifest.json)
are reused. Original references and extraction provenance remain retained in the
[asset manifest](asset-manifest.json). Native rendering is editable C geometry,
not a full-screen screenshot. Host scaling does not establish physical readability.

All known Companion screen families and the complete Dock family use the shared [LVGL UI integration](../../specs/architecture.md#native-ui-foundation).
Retained widget trees, shared layout/theme, source-exact native resource sprites
and a Vera glyph adapter belong to the host context; game/view state stays in the
Kit. The [Companions preview](../../docs/evidence/native-companion-resident-preview/README.md)
retains saved portrait/property/visits through offline inspection.
[Resident-list/visit controls](../../docs/evidence/native-companion-resident-actions/README.md)
use the same retained tree. Lab Home and workspace previews also use retained
LVGL across standalone and connected frame routes; [native evidence](../../docs/evidence/native-lab-home/README.md)
records changed controls, copied-state and lifetime checks. Remaining Lab action
pages retain manual renderers. Focused `companion_cargo_ui_checks`,
`companion_probe_ui_checks` and `companion_resident_ui_checks` cover this boundary
alongside domain, Kit and presenter checks. Dock's exported LVGL output uses four
gray levels; no physical e-paper driver or refresh behavior is established.

The headless Cargo path uses still focus in the live presenter. A controlled
120ms LVGL focus-fade export exercises the animation API without changing game
time, readiness or inputs. It is animation evidence, not live scheduling or audio
playback. ESP32/RPi physical drivers, power and performance remain unvalidated.

The superseded standalone review presenter was removed after checking callers;
use `native/presenter/server.py` with `CRITTER_DEMO_BINARY` pointing at the selected
Lab executable. The production path is the one exercised by HTTP tests and CI.

## Three-device mode

`selected_lab kit-serve` is used by the deployed presenter. Device0 is Lab,
1 Companion and2 Dock. `device ID status`, `device ID frame REVISION` and
`device ID INPUT REVISION` address distinct native contexts. Companion/Dock
`device ID link 0|1` is a simulation fault control, not a hardware action.
Legacy `serve` remains the single-device regression fixture; its old mutation
commands are unavailable in kit mode. Health/release routes remain compatible.

The presenter negotiates lossless gzip for native BMP responses and keeps raw BMP
available. HTTP/1.1 connections are reused; compression and response transfer run
outside the native command lock. Background polling is bounded and cannot queue
ahead of physical input. One frame fetch per device resolves to the latest native
revision; stale-frame rejection is retryable. Native interaction epochs retain valid time-only
repaint gestures and reject obsolete action meanings; stale input is never replayed. Down acknowledgement still precedes a separate release request,
and decoded/painted frames alone receive readiness acknowledgement. Transport,
overlap and blur checks live in `bridge.test.mjs`; HTTP encoding/reuse checks live
in `test_selected_presenter.py`. These changes do not establish radio latency.

The same game save gains `.kit` and `.kit.required` sidecars. Preserve all files
together: the first is the atomic transfer/cache journal; the marker prevents
silently replacing a missing journal after a transfer. Do not delete a sidecar
to bypass recovery. The simulator runs one host process and logical wireless
exchange, with no claim of independent endpoint stores or physical radio tests.
`three_device_kit_checks` covers ownership, interruption, duplicate acceptance,
restart at commit intent, required-journal loss and monochrome/native dimensions.


Device-input POST may include `ready: true` only on a physical down carrying
its painted revision. Python validates the whole request, executes READY/down
under one bounded native lock and returns the down result. Up is never part of
that sequence. Kit READY uses the same minimum/current interaction range as Lab;
semantic refresh still invalidates earlier frames. Native kit tests cover delayed
acknowledgement after a time-only repaint and rejection after navigation.

## Companion gathering and receipt interaction

The mode selector previews Probe/Cargo/Companions immediately with clamped
Left/Right. Down/Confirm enters actions, with a separate fresh Confirm required
to invoke one. Back restores task callers before returning to the selector.
Probe shows the generated local map, named sites, retained finite whole offers
and always-visible carried integer inventory. Directions steer; Confirm takes
a single offer immediately or opens a chooser when real alternatives exist.
Trace discovery reveals a route; a sealed sample requires explicit collection.
Cargo/Send use the same source resource sprites and
whole counts; no item fractions or preparation appear in their manifest.

Lab reception opens automatically once on arrival and cancels held input. Back
restores its previous navigation; world state remains current. Acceptance and
receipt are distinct durable states. Send stops gathering; fresh acceptance
unloads once and ends the source expedition, including an early return. Receipt
closes the handoff, and the next outing starts a new route identity. An empty
outing can Finish without sending a phantom haul. Source remainders and earned
units persist across restart. Legacy preparation/chance records remain frozen
and returnable; loading never converts them into new rewards. Recovery boundaries are
in [architecture](../../specs/architecture.md#three-device-host-simulator);
provisional finite offers and capacity belong in [V1](V1.md).

The [Gemini connected screen references](../../design/companion-connected-art/README.md)
preserve the reviewed Probe/Cargo/reception compositions and production constraints.
Reception07 is the lighter concept with concise Returning copy. Core V1
uses shared source-preserving material assets and a separate Gemini landscape;
actual Cargo/offline screens and retained portraits passed focused inspection.
The concept screens themselves are references, not executable layouts. Illustrated
research findings also passed focused actual review; exact evidence and the
release boundary are tracked in
[the Core V1 evidence](../../docs/evidence/polished-core-v1/README.md).
The playable map/site screens and received-only Lab log have an
[actual native journey](../../docs/evidence/playable-expeditions/README.md),
including cancellation/restart, sample collection, research and repeat resupply.

## Resetting the simulator sandbox

The presenter has one **Reset sandbox** control above the device shells. Confirming
starts a fresh native game across Lab, Companion and Dock, with the fixture's
default links and stock. This is simulator administration, not a device button or
hardware reset. Other connected browsers reconnect when the sandbox changes;
held input, queued releases and old frames cannot carry into the fresh game.

Before starting the new process, the presenter stops native under its command
lock and moves the configured save, `.kit`, `.kit.required` and any corresponding
`.tmp` files into a uniquely named `<save>.reset-<id>` sibling directory. It keeps
session and storage lock files at their original paths. The response identifies
the backup directory. Backups are retained until the operator disposes of them.
There is no browser restore or arbitrary file-management endpoint.

If fresh startup fails, the presenter restores the old files and checks all three
devices before serving them again. Failed new files are retained in the backup
with `.failed-new` suffixes. A failed rollback leaves native transport unavailable;
stop the presenter and restore the matching save and sidecars together from the
backup before restarting. An interrupted multi-file move also requires that
operator recovery. Do not combine files from different worlds or delete the
required marker to bypass recovery. After any unconfirmed reset, reload to verify
state before playing. The focused check is
`python3 native/tests/test_sandbox_reset.py [path/to/selected_lab]`; supplying the
binary exercises the actual three-device fresh state, old-input rejection,
backup sidecars and persistence after restart.
