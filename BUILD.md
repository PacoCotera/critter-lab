# Current build boundary

The new electronics-first specification is [here](specs/devices.md#electronics-first-v1-reference-specification). Current product: combined Companion, home Lab and caddy. Existing separate-Probe targets below are legacy build fixtures; they are not a second current portable. New recommended MCU/display profiles are not yet integrated. Software proof and human playtest precede hardware development; mobile fallback is explicit. No physical kit, power budget or production electronics freeze is claimed.

## Existing build evidence

# What can be built today

Start with [the local setup guide](docs/builders/getting-started.md) to run the available software. This repository is intended to supply the whole product, but a complete physical kit cannot yet be built from it.

| Component | Available | Missing for a complete build |
| --- | --- | --- |
| Game and genetics | Specifications and small executable fixtures | Balanced content and complete research/expression/behavior contracts |
| Art and animation | Original concepts, pixel masks/font, renderer and static exports | Production asset pipeline, animation and automated content tools |
| Local experiments | Node server, browser applications, pinned dependencies and tests | Integrated game client and production authentication |
| Cloud services | Architecture and synchronization contracts | Backend, generation jobs, migrations, deployment and recovery tooling |
| App and website | Responsibilities and product boundaries | Implementations and reproducible builds |
| Console, Probe and Companion | Device requirements and [native build scaffolds](native/README.md) for Linux, nRF52840 and ESP32-S3 | Functional firmware, peripheral profiles, board validation and flashing instructions |
| Electronics and caddy | Hardware concepts and constraints | Schematics, PCB sources, BOM and measured electrical/charging validation |
| Enclosures | Concept images | Editable CAD, fabrication files and assembly instructions |

A concept image is not a wiring diagram or case model. Schematics, pin assignments, charging limits and part selections must accompany validated hardware designs before an assembly guide can be written. No purchase list or substitute build recipe is implied here.

Product-required build and content tools belong in this public source. Credentials and player data do not. See the [specifications](specs/README.md) for requirements and the [experiment index](prototype/README.md) for implementation boundaries.
