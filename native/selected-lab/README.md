# Native Beecho Lab V1

The current [playable loop and limits](V1.md) are authoritative for this target.
Native C17 owns the game, saved world, focus, input authorization and every
1024×600 pixel. The browser transports fixed simulated hardware button events
and displays native BMP frames. It does not implement the game or screen layout.

## Platform

The selected Lab reference is **Raspberry Pi4 Model B**, with the Waveshare 7inch
HDMI LCD (H), 1024×600. The existing build is Linux **x86-64 host simulation** using
GCC, CMake and Ninja. It is not an ESP32/ESP-IDF executable, Pi emulation or a
verified ARM build. Physical HDMI/input integration, board boot and performance
remain untested. ESP-IDF belongs to the separate Companion target.

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
