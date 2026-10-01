# What can be built today

Start with [builder getting started](docs/builders/getting-started.md) for the
current native simulator. This repository supplies product specifications,
source and art, but cannot yet produce a complete physical kit.

| Component | Available evidence/source | Remaining boundary |
| --- | --- | --- |
| Connected game | Native C17 Lab/Companion/Dock host simulation; gathering, reception, research, genome selection, incubation, reveal, habitat visits and durable saves | Provisional Pip content/balance; broader studies, capture/training, ecology and independent device authority |
| Native presentation | Native frames and physical-control transport; Companion Probe/Cargo and complete Dock family use LVGL with checked native output | Remaining Lab/Companion manual-renderer migration; shared ESP-IDF UI builds, art and human usability |
| Lab platform | Linux x86-64 executable built with GCC/CMake/Ninja; Raspberry Pi4 development reference | ARM build, HDMI/input integration, board performance and physical evidence |
| Portable firmware | Legacy nRF52840 Probe and ESP32-S3 Companion compiler scaffolds | Current combined Companion drivers, sensors, radio, storage, board validation and flashing instructions |
| Caddy | Complete retained LVGL four-gray host family; same shared UI compiled/linked in headless ESP32-S3 target | Real input/state/radio, display/printer drivers, charging and bench evidence |
| Earlier experiments | Pinned Node/browser studies and genetic/transfer fixtures | Separate studies do not form another integrated product |
| Cloud and mobile | Product roles, contracts and explicit fallback direction | Production services, authentication, synchronization and mobile game implementation |
| Website | Public website source under `website/` | A website is not the game client or physical-kit proof |
| Electronics/enclosures | [Reference profiles](specs/devices.md) and preserved concept art | Schematics, PCB sources, measured power/thermal/RF budgets, editable case CAD, fabrication and assembly instructions |

The current product is one combined Companion, one home Lab and one shared Caddy.
Probe is a mode. The caddy development reference is 5.79-inch monochrome,
792×272; older separate-Probe/3.7-inch builds and renders remain historical evidence.
Lab targets Linux C17 on Raspberry Pi4. Companion and Caddy target ESP32 with
ESP-IDF; the current Companion scaffold is historical and contains no current
LVGL game UI. The Caddy has a headless shared-UI compile target, with no physical
panel or game authority adapter. Every product screen must
use LVGL; [architecture coverage](specs/architecture.md#current-migration-coverage-and-target-evidence)
records the incomplete migration. Native host checks establish software behavior, not flashed-device or
physical-display performance.

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
