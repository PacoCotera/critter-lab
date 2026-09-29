# Product specifications

These documents describe Critter Lab's product rules and engineering boundaries. They are the authoritative product specification; experiments are evidence for a limited implementation, not substitutes for the design.

For orientation rather than requirements, use the [documentation map](../docs/README.md), [player introduction](../docs/players/README.md) or [builder starting guide](../docs/builders/getting-started.md).

| Subsystem | Specification |
| --- | --- |
| Ecosystem and generation | [Architecture](architecture.md) |
| Content tools, app/site and backend boundaries | [Architecture](architecture.md#content-management-boundary) |
| Research, resources, crafting and progression | [Gameplay](gameplay.md) |
| Players, shared equipment, consent and discovery | [Players and social play](players-social.md) |
| Heredity, expression, development and behavior | [Genetics](genetics.md) |
| Bounded Pip content and engine proof | [Genetic engine contract](genetic-engine.md) |
| Field evidence and fictional encounters | [Probe](probe.md) |
| Physical devices, power and interfaces | [Devices](devices.md) |
| Interaction, readability and device profiles | [Experience](experience.md) |
| Authoritative state, offline activity and recovery | [Cloud synchronization](cloud-sync.md) |

## Reading status correctly

The proposed [sample-to-critter system contract](sample-to-critter-contract.md) defines record and acceptance boundaries. The separate [player walkthrough concept](../design/sample-to-critter-walkthrough.md) introduces the experience; it is not a source of technical requirements.

**Accepted** identifies settled direction. **Proposed** identifies a candidate rule or contract. **Open** identifies a missing decision. Separately, implementation may be a host experiment, integrated software or physically validated hardware. Approved direction does not mean implemented behavior. Numeric examples and placeholders are not balance or canonical art unless explicitly identified as such.

Version product releases separately from record/protocol formats, genetics rules/content, expression contexts, appearance mappings, preserved assets and firmware/hardware profiles. A consumer must identify the versions it supports. Unsupported records are preserved; upgrades never silently add genes, reroll an individual or replace its finished art. Compatibility and migration policy remain explicit design work.

The current target is a combined Companion, home Lab and shared caddy, with a mobile fallback. The electronics-first software gate precedes physical development. Current host experiments do not establish production cloud generation, firmware, sensing, charging, printing or manufacturing readiness.
