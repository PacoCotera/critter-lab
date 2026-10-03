# Critter Lab

Explore outside. Investigate a mystery. Meet a Beecho of your own.

Critter Lab is a creature-research game built around physical instruments, with
one shared world across the **combined Companion**, **home Lab** and **Caddy**.
[Meet the game](docs/players/README.md) or [run the software](docs/builders/getting-started.md).

## Design before the next implementation

The connected player journey leads the current architecture and game-design
overhaul. The project owner has final say on changed game rules, architecture,
UI/UX and all art direction and acceptance before dependent implementation.
Involve the owner during direction selection and review actual resulting screens
and assets at their intended sizes. Existing approved
direction remains reusable; prototypes, reviewed concepts, merged source and
passing checks are evidence with stated limits, not approval of a new baseline.
[Architecture](specs/architecture.md), [gameplay](specs/gameplay.md) and
[experience](specs/experience.md) retain accepted direction and open decisions.

The hosted destinations are [the website](https://critterlab.basicberry.com/),
[genome workbench](https://critterlab.basicberry.com/genome/) and
[game simulator](https://critterlab.basicberry.com/sandbox/).
The [platform source boundary](prototype/platform-server/README.md) keeps their
processes/releases separate and documents browser-origin retention limits;
source integration alone is not activation evidence.

![Current family: sage home Lab, stone combined Companion and shared printer/summary caddy.](design/lab-controls/combined-family-materials.png)

*Selected family appearance, not a manufactured kit or measured hardware design.
[Concept details and earlier studies](design/lab-controls/concepts.md) preserve provenance.*

[Roadmap](ROADMAP.md) connects the delivered Core V1 with playable exploration.
Steer the Companion, gather at finite sources, investigate traces and collect a
sealed sample. Lab Explore shows received expedition records. The
[actual native walkthrough](docs/evidence/playable-expeditions/README.md) passed
the connected journey and independent game/UX review; the original
[map study](design/expedition-map-study/README.md) remains its design reference.
The native `/api/release` identifies the running game; `/api/platform-release`
identifies the separate website/workbench hosting source.
[Deployment notes](CHANGELOG.md) distinguish the two releases.

## Playable software today

The [three-device native simulator](native/selected-lab/README.md#three-device-mode)
connects Companion gathering and Cargo return with explicit Lab reception,
resource-funded research, genome selection, incubation, deliberate reveal and
habitat visits. Native C17 owns rules, saved state and physical focus; LVGL renders
all current connected screen families into native framebuffers. The browser transports those frames
and the depicted physical controls. The devices are logical contexts in
one Linux host process, with simulated wireless links.

The [native UI foundation](specs/architecture.md#native-ui-foundation) uses LVGL 9.6.0
for all known Companion host screen families, including resident selection and visits. Shared layout, theme, image/font adapters and
physical focus replace manual screen drawing in those workpieces. The
[Cargo proof](docs/evidence/native-ui-foundation/README.md) and
[Probe proof](docs/evidence/native-companion-probe/README.md) record actual
handoff, source-exact material pixels and output review. All device screens must
use this framework. The [complete Dock LVGL family and portable display boundary](docs/evidence/native-dock-lvgl/README.md)
passed native output and independent review. The [Companions preview proof](docs/evidence/native-companion-resident-preview/README.md)
shows retained saved portraits/properties and offline inspection without a visit.
The [resident list/visit proof](docs/evidence/native-companion-resident-actions/README.md)
records save-once visits and shared counts. [Lab Home and workspace previews](docs/evidence/native-lab-home/README.md) now use retained LVGL, with native routes, controls and independent review checked. [Connected Lab reception and received records](docs/evidence/native-lab-reception/README.md) also use LVGL. [Sample research and Library](docs/evidence/native-lab-research/README.md) now share a retained family with copied knowledge, costs and original-size portraits. Creation/incubation and resident/habitat actions also use retained LVGL; [action evidence](docs/evidence/native-lab-actions/README.md) records their boundaries. The [Caddy shared UI compile](docs/evidence/native-dock-lvgl/ESP32.md)
and [current Companion UI](docs/evidence/native-companion-esp/README.md)
now link under ESP-IDF; neither headless target establishes physical operation. Final HiBit
artwork and human playability remain open.
The [Send/Keep proof](docs/evidence/native-companion-send/README.md) records the
shared portable Cargo tree, actual offline return/acceptance, safe focus and
independent technical/UI/UX review.
The [Discard/Finish proof](docs/evidence/native-companion-discard/README.md)
shows exact whole-item decisions, safe Keep/Back, empty outing completion and
separate native maximum/recovery fixtures, with independent craft review.
The [Cargo mode-preview proof](docs/evidence/native-companion-cargo-preview/README.md)
shows read-only browsing, remembered action entry and separate accepted receipts.
Owner permits complete re-layout under the [Companion direction](specs/experience.md#framework-led-companion-layout).

Follow the [Pip play guide](native/selected-lab/V1.md) and inspect the
[actual native screen gallery](design/connected-device-review/native/README.md).
The [live sandbox](https://critterlab.basicberry.com/sandbox/) reports its running revision.
Its reset control preserves a recoverable saved-world backup.

This is a bounded playable Core V1 prototype. Shared retained Gemini resource and
resident art, expedition scenery and illustrated A/B research are integrated and
passed focused actual review. [The native walkthrough](docs/evidence/polished-core-v1/README.md)
shows the connected loop and corrected screens. Timing, chance and content remain
provisional. [Status](STATUS.md) records evidence and remaining work. Native
gameplay releases start a fresh shared game; hosting-only website/workbench
activations preserve the native release and its saved world.

The separate [genome workbench](prototype/generator-workbench/README.md) provides
guided inherited-trait editing, explicit structure refresh, Before/Current
comparison and deliberate rendering with retained variants. Its current default
carries 114 copied pairs, six drafts and eleven named layers, with four domains
still lacking executable contracts. [Actual hosted use](prototype/generator-workbench/evidence/guided-authoring/README.md)
and [two retained Google images](prototype/generator-workbench/evidence/api-rendering/README.md)
show literal source/prompt retention. Returned art is not an accepted
source-faithful master. Broad organism range, full genomic coverage, animation,
sharing and game integration remain unfinished; OpenAI is unconfigured.

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

Critter Lab is a project of **Dirty Pawz Press**. Software uses AGPL-3.0-only,
hardware sources CERN-OHL-S-2.0, and documentation/eligible artwork CC-BY-SA-4.0.
See [licensing](LICENSING.md), [branding](BRANDING.md),
[contributing](CONTRIBUTING.md) and [versioning](releases/README.md).
