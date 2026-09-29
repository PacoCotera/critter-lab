# Foundation demo: a native expedition loop

Current hardware reference: combined Companion, home Lab and shared caddy. The caddy uses the proposed-for-bench [5.79-inch monochrome module](../../specs/devices.md#electronics-first-v1-reference-specification), 792×272; older separate-Probe and 3.7-inch depictions are historical. Original art and earlier build evidence are preserved, not physical validation.


The owner has authorized a small playable scaffold on the preliminary native targets. This increment ends at **one saved research finding**. It does not create a critter or represent a finished hardware simulator. The [first expedition](../../design/first-expedition.md) supplies the illustrative player journey; [native setup](../../native/README.md) supplies build and run instructions.

## What this increment implements

Review and load Material trail at the Lab, start the Probe, advance demonstration time, optionally inspect a clue, complete the expedition, receive its haul once, and spend one unit of lab supplies on Structure study. Reopening retains the finding and remaining inventory. Required genomic regions remain unknown; creation is unavailable. Names, quantities and timing are fixtures, not approved game balance.

Player controls perform explicit actions. Separate engineering controls advance simulated time. The browser cannot award resources, advance research, choose game states or draw device pixels.

## Execution and module boundaries

| Part | Responsibility |
| --- | --- |
| Portable C domain | Fixed-size state, legal commands, expedition progress, inventory and research transitions. No SDK, filesystem or networking dependencies. |
| Portable C renderer | Bounded row rendering, pixel font and native device images. No gameplay decisions. |
| Linux Lab adapter | Strict command protocol, versioned save, revision checks, retry handling and BMP output. |
| Python bridge | Authenticated, serialized subprocess transport and fixed HTTP routes. No duplicate game rules. |
| HTML presenter | Display native frames, relay available commands and wait for the matching frame before enabling input. |
| Zephyr Probe and ESP-IDF Companion | Compile and invoke the shared domain/renderer in native target scaffolds. Physical peripheral adapters remain future work. |

The selected Lab panel is Waveshare 7inch HDMI LCD (H), **1024×600 color landscape**. The Probe's candidate e-paper profile is **122×250 portrait, packed 1-bpp**, with controller rotation still requiring a real panel adapter. The selected Companion board uses **368×448 color portrait** and an honest empty scene in this slice. Native row formats are RGB888 for Lab/Companion and packed monochrome for Probe; BMP expansion is a Linux transport step, not MCU storage. See [device contracts](../../specs/devices.md). No invented individual populates the Companion.

The Linux executable runs shared application logic, **not an MCU ELF or emulated CPU**. MCU cross-compilation checks language, linkage and static resource fit. Host frames do not prove panel refresh, radio, battery, thermal behavior, stack peaks or hardware readiness. Board testing with real drivers remains necessary.

## Persistence and access

This fixture keeps Lab and Probe state in one local snapshot. Receiving the haul is one local atomic operation; it does not implement distributed transfer, cloud acceptance or independent offline device storage. The current product direction requires durable standalone core play and optional global synchronization; this fixture does not establish that distributed contract.

Commands carry the displayed revision and an operation ID. Exact retries of the last accepted operation return its retained result; changed payloads reject. Older revisions reject, and domain rules also prevent repeated receipt or research spending. Existing corrupt or unsupported saves fail closed. The Linux adapter locks the save and replaces an explicitly encoded snapshot atomically; it does not write raw C structs.

The presenter protects pages, status, images and commands with authentication. Secrets and mutable saves stay outside source control and release files. Fixed demo access is **not** production player isolation or device authorization. Deployment must validate same-origin command handling and access through its configured reverse proxy.

## Delivery evidence

A completed increment requires:

- Linux build and complete prepare → collect → receive → study → reopen walkthrough.
- MCU cross-builds invoking the shared core and renderer.
- Meaningful retry, stale request, save corruption and persistence checks.
- Inspection of actual native images and usable browser controls.
- Authenticated staging access, including rejection of unauthenticated reads and writes.

The native README records implemented commands and validation. A build scaffold alone is not delivery of the playable loop.

## Following increments

After reviewing this scaffold, narrow physical controls, panel behavior, sensing and power constraints; add target adapters and representative board measurements. Independent device storage and the real handoff protocol precede offline/sync claims. Full research unlock, supported complete genome selection, one-time creation and retained individual art form a later gameplay increment. Creation must never substitute random generation for an unresolved genome.
