# Beecho Lab

Explore outside. Investigate a mystery. Meet a Beecho of your own.

Formerly Critter Lab; repository URLs and original references retain that name.
Beecho Lab is a creature-research game built around physical instruments, with
one shared world across the **combined Companion**, **home Lab** and **Caddy**.
[Meet the game](docs/players/README.md) or [run the software](docs/builders/getting-started.md).

![Current family: sage home Lab, stone combined Companion and shared printer/summary caddy.](design/lab-controls/combined-family-materials.png)

*Selected family appearance, not a manufactured kit or measured hardware design.
[Concept details and earlier studies](design/lab-controls/concepts.md) preserve provenance.*

[Roadmap](ROADMAP.md) connects the delivered Core V1 with playable exploration.
Steer the Companion, gather at finite sources, investigate traces and collect a
sealed sample. Lab Explore shows received expedition records. The
[actual native walkthrough](docs/evidence/playable-expeditions/README.md) passed
the connected journey and independent game/UX review; the original
[map study](design/expedition-map-study/README.md) remains its design reference.
The sandbox release endpoint identifies the currently activated version.

## Playable software today

The [three-device native simulator](native/selected-lab/README.md#three-device-mode)
connects Companion gathering and Cargo return with explicit Lab reception,
resource-funded research, genome selection, incubation, deliberate reveal and
habitat visits. Native C17 owns rules, saved state, focus and pixels; the browser
transports the depicted physical controls. The devices are logical contexts in
one Linux host process, with simulated wireless links.

The [native UI foundation](specs/architecture.md#native-ui-foundation) is being
integrated incrementally using LVGL9.6.0, starting with real Companion Cargo.
Shared layout, theme, image/font adapters and physical focus replace manual
screen drawing within that slice. The remaining screens and final HiBit artwork
are not yet migrated or accepted.

Follow the [Pip play guide](native/selected-lab/V1.md) and inspect the
[actual native screen gallery](design/connected-device-review/native/README.md).
The [live sandbox](https://critterlab.basicberry.com) reports its running revision.
Its reset control preserves a recoverable saved-world backup.

This is a bounded playable Core V1 prototype. Shared retained Gemini resource and
resident art, expedition scenery and illustrated A/B research are integrated and
passed focused actual review. [The native walkthrough](docs/evidence/polished-core-v1/README.md)
shows the connected loop and corrected screens. Timing, chance and content remain
provisional. [Status](STATUS.md) records evidence and remaining work. Each newly
deployed sandbox version starts a fresh shared game.

## The whole product

The Companion is the everyday portable: **Probe** gathers and investigates,
**Cargo** holds earned findings, and **Companions** supports the intended life
with travelling critters. The Lab is the research workbench and visual window
into habitats. The Caddy is their shared charging/printing home with quiet world,
supply and connection summaries. Probe is a mode, not a separate current device.

[Gameplay](specs/gameplay.md) owns research, creation and expedition rules;
[devices](specs/devices.md) owns physical roles and open electronics choices.
Local core-kit play, nearby-kit interaction and optional Cloud Pass services
are product direction. Cloud, capture/training, ecology, sensors, charging and
physical printing are not delivered by the host simulator.

Software proof and human playtest precede PCB/enclosure development and hardware
investment. The Lab reference is Raspberry Pi4 with Linux C17; present evidence
is Linux x86-64, without ARM or physical-board validation. A mobile app remains
the fallback. See the [build boundary](BUILD.md).

## Start where you are

| You want to… | Start here |
| --- | --- |
| Understand and try the game | [Player introduction](docs/players/README.md) and [Pip play guide](native/selected-lab/V1.md) |
| Review screens and appearance | [Design guide](design/README.md) |
| Understand rules and systems | [Specification map](specs/README.md) |
| Run or help build the software | [Builder getting started](docs/builders/getting-started.md) |
| Find evidence and documents | [Status](STATUS.md) and [documentation map](docs/README.md) |

This is the authoritative public product repository for specifications, code,
product documentation, designs, hardware concepts/sources, original references
and source art. Concepts and unfinished studies retain their provenance and
approval status. It is not a complete buildable physical kit. [Earlier host experiments](prototype/README.md) remain
useful studies with separate saves and limits.

Beecho Lab is a project of **Dirty Pawz Press**. Software uses AGPL-3.0-only,
hardware sources CERN-OHL-S-2.0, and documentation/eligible artwork CC-BY-SA-4.0.
See [licensing](LICENSING.md), [branding](BRANDING.md),
[contributing](CONTRIBUTING.md) and [versioning](releases/README.md).

The [native LVGL Cargo proof](docs/evidence/native-ui-foundation/README.md) records source-exact1× materials, retained layout/focus/frame widgets and actual physical-control handoff. Cargo alone has migrated; other screens and final HiBit craft remain open. Owner permits complete re-layout under the [Companion experience direction](specs/experience.md#framework-led-companion-layout).
