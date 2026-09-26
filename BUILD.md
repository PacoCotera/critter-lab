# Build coverage

The public repository is intended to contain everything needed to build the complete game. That is its scope, not a claim that every subsystem exists today.

| Component | Available now | Required before a complete build |
| --- | --- | --- |
| Game rules and genetics | Product specifications and small executable fixtures | Complete balanced content, expression and behavior contracts |
| Art and animation | Concept references, pixel masks/font/palettes, renderer and static exports | Production assets, animation system and automated generation tooling |
| Local web experiments | Node server, browser applications, dependencies and tests | Production client/service implementation and authentication |
| Cloud/backend | Architecture, authority and sync design | Durable backend, generation services, migrations and deployable reference setup |
| Companion app/site | Product boundaries | Application implementations and reproducible builds |
| Console/Probe/Companion firmware | Device requirements | Source, board profiles, toolchain versions and flashing instructions |
| Electronics and caddy | Hardware directions and unresolved choices | Schematics, PCB source, BOM and measured validation |
| Enclosures | Concept images | Editable case models, drawings, print/fabrication files and assembly instructions |

## Run the host experiments

Install Node.js 22 or later and npm. At repository root run `npm ci`, `npm test`, then `npm start`. Open `http://127.0.0.1:4173`, `/lab/` or `/transfer/`. Dependencies are pinned in `package-lock.json`. Browser state is local; it is not a cloud-backed player account.

Regenerate lab frames with `node prototype/lab/export.mjs` and the independent pixel study with `node prototype/pixel/export.mjs`. See the individual experiment READMEs for transfer and compatibility demos. Optional browser checks require a separately installed Playwright package and Chromium browser.

Production build instructions will be added alongside each implementation. No private agent framework is intended to be necessary to build the product. Credentials, real deployment hosts and private player data are never build inputs committed here.
