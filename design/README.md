# Current product direction

The current family is **one combined Companion, one home Lab, and one shared printer/habitat caddy**. Probe is a Companion mode, not a separate device. Current appearance is the sage/stone family with recessed caddy branding and grey OK; original four-object renders below are preserved prior art, not current hardware requirements.

![Current family appearance](lab-controls/combined-family-materials.png)

See [current concept details](lab-controls/concepts.md) and [electronics-first reference](../specs/devices.md#electronics-first-v1-reference-specification). The immediate gate is software game/firmware validation before PCB/enclosure development; a mobile app is the fallback. Renders do not establish dimensions, material finish, display performance or finalized screen assets. The docked Lab owns visual habitats/research, Companion runs Probe, and caddy shows summaries.

## Retained design references and studies

The following material preserves earlier exploration. Where it describes separate Probe hardware or older palettes/layouts, the current direction above supersedes it.

# Design reference and review guide

This page is for reviewing how Critter Lab looks, feels and operates. For a plain introduction to playing, start with [Your first discovery](sample-to-critter-walkthrough.md). For rules and system boundaries, use the [specification map](../specs/README.md); for runnable work, use [builder getting started](../docs/builders/getting-started.md).

## The device family

![Original Critter Lab family: a sloped tabletop Lab with landscape display, rotary control, three keys and printer; a narrow two-key Probe; and a larger color Companion with three lower controls.](references/branded-family.png)

*Approved concept direction, not an engineering specification. These are the original device references; final dimensions, electronics, controls and display modules remain open.*

The family uses cream enclosures, charcoal frames, restrained orange controls and tactile details. Pixel critters bring character to the screens and printed cards. The Lab is a deliberate workbench, the Probe a quick outdoor instrument, and the Companion a portable place for an individual. Their different proportions and controls should remain recognizable across illustrations.

The [ecosystem reference](references/ecosystem.png) shows the portables with their Caddy and an optional app view. The [Companion close-up](references/companion.png) provides a clearer view of its silhouette, controls and creature display. Use these references to understand the physical relationships; they do not establish charging measurements, data transfer or completed app behavior.

## Screen design in progress

Playful Pixel Lab is selected as the foundation for refinement. Continue the [refinement-02 vocabulary](visual-language/refinement-02/README.md); do not restart selection among earlier explorations. The Lab collection-to-discovery composition is approved; other components and screen compositions remain proposals. The rejected PR18 packet is closed and excluded from current review. No later complete accepted screen set has been verified in the retained repository artifacts.

The [screen design standard](screen-design-standard.md) defines the current exploration and review sequence. Previous prototype layouts are rejected as the target experience. Visual concepts must be reviewed before their implementation.

## Accepted flow and next design gate

Current review: [the connected research game model](research-and-creation.md) relates capsules, prepared genome records, typed inventory, expedition profiles and signals/points. It works through choices among several partially decoded genomes before further screen work. The prior research-journey paper sketch is superseded as screen direction: unknown regions are not locks, and the workbench needs meaningful discovery and collection choices rather than explanatory progress panels.

[Expedition-to-discovery storyboard](expedition-review/README.md) records accepted choices for interruption, optional encounters and the bridge into research. Its authored content and artwork are not approval of screen styling. The next gate is the connected device-screen experience described in the [product plan](../docs/README.md#design-and-delivery-roadmap), using existing selected references and actual console controls. Creation and Companion life remain later context.

The [current Lab collection-to-discovery study](genome-workbench/README.md) applies that direction to the demonstrated Pip genetics: a partial allele pair, explicit resource cost and a carried-versus-expressed discovery. Its five static frames are an owner-approved composition and interaction direction, not a complete journey or implemented UI.

The [Probe expedition study](probe-expedition/README.md) continues gathering through Lab receipt in nine native monochrome states. Existing Next/Confirm controls, unknown capsule contents and separate stock/point accounting are explicit; this study awaits visual review.

## What to review

| Question | Material and boundary |
| --- | --- |
| Where does the player make meaningful research choices? | [Research and creation](research-and-creation.md) explains the accepted V1 investigation structure; the worked content, costs and timing remain illustrative or open. |
| What is spent when a critter is created? | [Creation terms](creation-terms.md) explains accepted sample use, retained knowledge, repeat creation and interruption handling. |
| Can simple sampling stay varied? | [Probe sampling](probe-sampling.md) proposes broad sensed context, stable per-sample variation and optional fictional events. |
| How does one expedition play out? | [First expedition](first-expedition.md) is a proposed textual scenario from preparation to a saved research finding; [foundation demo plan](../docs/builders/foundation-demo.md) records hardware and simulation gates. |
| Can a newcomer understand the journey? | [Your first discovery](sample-to-critter-walkthrough.md) follows exploration, research, a supply shortage, creation and companionship. It is a concept story, not an implemented sequence. |
| Does the interaction explain what changes? | [Experience specification](../specs/experience.md) covers navigation and feedback. Review the player action, its consequence and the return path together, rather than approving an isolated attractive screen. |
| What can the hardware actually support? | [Device specification](../specs/devices.md) and [build coverage](../BUILD.md) distinguish exploration from available engineering work. A render cannot demonstrate physical readability, refresh, fit or power. |
| What have the screen experiments demonstrated? | [Lab prototype](../prototype/lab/README.md) and [transfer study](../prototype/transfer/browser/README.md) contain scoped implementation evidence. Their authored placeholders and earlier layouts are not the target visual design. |

Keep shared direction separate from unresolved choices. The original instrument character guides presentation; final screen compositions, individual creature designs, motion and engineering details still need their own review. A new illustration must not silently add controls, change a critter's identity or turn a proposed mechanic into a promise.

## Sources and attribution

The [asset manifest](asset-manifest.json) records original-image hashes and design status. Concept references are AI-generated product explorations, preserved as supplied; they are not CAD or assembly instructions. Existing pixel experiments include source masks, palettes, font data and rendering code under `prototype/pixel/`.

Use the project name and restrained Dirty Pawz Press attribution consistently with the [branding policy](../BRANDING.md). That policy addresses attribution and official status; it does not supply a company-wide visual identity standard.

[All documentation](../docs/README.md) Â· [Player introduction](../docs/players/README.md) Â· [Current product status](../STATUS.md)
