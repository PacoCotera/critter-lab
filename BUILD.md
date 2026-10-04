# What can be built today

Start with [builder getting started](docs/builders/getting-started.md) for the
current native simulator. This repository supplies product specifications,
source and art, but cannot yet produce a complete physical kit.

| Component | Available evidence/source | Remaining boundary |
| --- | --- | --- |
| Connected game | Native C17 Lab/Companion/Dock host simulation; gathering, reception, research, genome selection, incubation, reveal, habitat visits and durable saves | Provisional Pip content/balance; broader studies, capture/training, ecology and independent device authority |
| Native presentation | All current connected Lab, Companion and Dock families use LVGL with native-frame and physical-control evidence | Physical target runtime, final art and human usability remain separate gates |
| Lab platform | Linux x86-64 executable built with GCC/CMake/Ninja; Raspberry Pi4 development reference | ARM build, HDMI/input integration, board performance and physical evidence |
| Portable firmware | Legacy nRF52840 Probe scaffold; current Companion shared-UI ESP32-S3 headless compile/link proof | Runtime allocation/profile remains unresolved; panel/input/game/save/radio integration and board validation absent |
| Caddy | Complete retained LVGL four-gray host family; same shared UI compiled/linked in headless ESP32-S3 target | Real input/state/radio, display/printer drivers, charging and bench evidence |
| Genome/art authoring | Hosted guided workspace, pinned source/replay, explicit Google rendering and retained original prompts/images | Partial genomic consumers and broad anatomy; source-faithful masters, animation, sharing and game integration unfinished; OpenAI unconfigured |
| Earlier experiments | Pinned Node/browser studies and genetic/transfer fixtures | Separate studies do not form another integrated product |
| Cloud and mobile | Product roles, contracts and explicit fallback direction | Production services, authentication, synchronization and mobile game implementation |
| Website | Public website source under `website/` | A website is not the game client or physical-kit proof |
| Electronics/enclosures | [Reference profiles](specs/devices.md) and preserved concept art | Schematics, PCB sources, measured power/thermal/RF budgets, editable case CAD, fabrication and assembly instructions |

The current product is one combined Companion, one home Lab and one shared Caddy.
Probe is a mode. The caddy development reference is 5.79-inch monochrome,
792×272; older separate-Probe/3.7-inch builds and renders remain historical evidence.
Lab targets Linux C17 on Raspberry Pi4. Companion and Caddy target ESP32 with
ESP-IDF; the [Companion target](native/companion/README.md) now registers current
shared LVGL UI, with [compile/link validation](docs/evidence/native-companion-esp/README.md). The Caddy has a headless shared-UI compile target, with no physical
panel or game authority adapter. Every product screen must
use LVGL; [architecture coverage](specs/architecture.md#current-migration-coverage-and-target-evidence)
records completed current host coverage and unverified physical target paths. Native host checks establish software behavior, not flashed-device or
physical-display performance.

The [design decision boundary](README.md#design-before-the-next-implementation)
precedes dependent architecture/game/UI/art changes. Buildable prototypes remain
evidence, not approval of those designs. [Guided authoring evidence](prototype/generator-workbench/evidence/guided-authoring/README.md)
and [actual API images](prototype/generator-workbench/evidence/api-rendering/README.md)
record the separate host authoring proof. Hosting-only website/workbench
activation preserves native game saves; [deployment notes](CHANGELOG.md) keep
hosting and native releases distinct.

The [native guide](native/README.md) contains build commands and the existing
CI bundle path. The [play guide](native/selected-lab/V1.md) owns fixture limits,
and [status](STATUS.md) identifies tested source and delivery evidence. Source,
executable and presenter must come from the same committed revision; keep saved
worlds and their sidecars outside release bundles and backed up together.

The [electronics-first gate](specs/devices.md#electronics-first-v1-reference-specification)
requires software proof and human playtest before hardware investment. Reference
parts are not a purchase list; renders are not wiring diagrams or mechanical
models. No complete kit, production electronics freeze or measured hardware
feasibility is claimed.
