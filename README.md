# Beecho Lab

Explore outside. Investigate a mystery. Meet a Beecho of your own.

Formerly Critter Lab; repository URLs and original references retain that name.
Beecho Lab is a creature-research game built around physical instruments, with
one shared world across the **combined Companion**, **home Lab** and **Caddy**.
[Meet the game](docs/players/README.md) or [run the software](docs/builders/getting-started.md).

![Current family: sage home Lab, stone combined Companion and shared printer/summary caddy.](design/lab-controls/combined-family-materials.png)

*Selected family appearance, not a manufactured kit or measured hardware design.
[Concept details and earlier studies](design/lab-controls/concepts.md) preserve provenance.*

[Polished Core V1 roadmap](ROADMAP.md) tracks the current implementation round.

## Playable software today

The [three-device native simulator](native/selected-lab/README.md#three-device-mode)
connects Companion gathering and Cargo return with explicit Lab reception,
resource-funded research, genome selection, incubation, deliberate reveal and
habitat visits. Native C17 owns rules, saved state, focus and pixels; the browser
transports the depicted physical controls. The devices are logical contexts in
one Linux host process, with simulated wireless links.

Follow the [Pip play guide](native/selected-lab/V1.md) and inspect the
[actual native screen gallery](design/connected-device-review/native/README.md).
The [live sandbox](https://critterlab.basicberry.com) reports its running revision.
Its reset control preserves a recoverable saved-world backup.

This is a bounded playable prototype. Current connected screens are functional
scaffolding; the [reviewed Gemini Probe, Cargo and reception concepts](design/companion-connected-art/README.md)
remain the next native art implementation. Timing, chance and content are
provisional. [Status](STATUS.md) records evidence and remaining work.

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
