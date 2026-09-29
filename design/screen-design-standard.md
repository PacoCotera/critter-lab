# Screen design standard

## Approved Lab UI baseline

Paco approved [the refined palette C screen](game-art-proposals/35-vault-composition/18-c-refined.png) on28September2026 as the **baseline UI from which the rest of the Lab app is derived**. This exact image is the primary Lab visual reference. The [baseline entry](game-art-proposals/35-vault-composition/README.md) records scope and evidence. Earlier Vault boards, palettes and screen variants are supporting references, not competing visual authorities. Do not reopen palette or composition discovery for routine Lab screens.

The Lab is a playful genetics research device: graphite surfaces, defined electric-blue frames, saturated crisp pixel artwork and selective warm focus. Preserve biological curiosity and the original pixel personality; avoid cosmic/starfield imagery, purple-dominant surfaces, generic web cards and hardware depicted inside hardware.

### Visual roles

| Element | Approved baseline / derivation rule |
| --- | --- |
| Base surfaces | Graphite/dark neutral depth; quieter than art and information |
| Frames | Defined electric-blue leading edges, dark edge contrast and consistent stepped corners; crisp rather than bloomy |
| Resources | Saturated blue Data card, gold faceted Energy crystal, rounded translucent lime Essence; stable shape/color identity across Lab screens |
| Focus | Quiet dark action with selective warm edge/corner highlights and readable light text; one console target, no solid bright yellow fill dominating the screen |
| Header | Unified sample/context identity and aligned resource inventory; preserve hierarchy and comfortable spacing |
| Findings | Equal, clearly labeled illustration panels with art directly on the dark field; no trapezoid platforms or pedestals |
| Type | Friendly prominent headings and legible supporting text, using panel resolution fully |
| Pixel craft | Saturated well-delineated sprites, intentional clusters and consistent apparent pixel density; detail through drawing, not blur or excessive micro-shading |
| Knowledge | Known and unknown remain explicit; unknown imagery reveals no specimen result and must not imply absent, locked or unaffordable |
| Feedback | Pending/saved/error remain distinct from knowledge and focus. Red is for actual error/risk, not affordable study cost |

Approval establishes visual direction, composition grammar and hierarchy. It does not establish exact sampled color tokens, font licensing, native sprite masters, motion, physical-display performance or implemented navigation. Author reusable assets and compact variants faithfully; verify them against this baseline rather than redesigning them independently per screen. Decorative biological imagery is not genomic data. Resource colors must not imply relationships to unrelated traits.

## Across the device family

- **Lab:** a landscape research workbench with visible sample/topic, useful findings, unknowns, stock, study cost and explicit commitment. Illustration supports an activity rather than displacing it.
- **Probe:** compact expedition/gathering progress first; shared resources and state vocabulary, with authored monochrome alternatives where needed. No sensor readings, sonar or event mechanics invented by art.
- **Companion:** individual critter identity, care and response; preserve recognizable heredity across representations.
- **Atlas:** detailed specimen/reference illustration that rewards inspection, using the same identity. Richer editorial treatment may vary without adopting the unselected Archive as the primary UI.
- **Print:** recognizable specimen and symbol family in ordinary monochrome. Specialty finishes are not requirements.

The board's moth-like critter, sample data, counts, hypothetical controls and layouts are exploratory examples, not approved species, implemented mechanics or hardware decisions. Final display technologies remain open. A generated dark-screen board establishes no outdoor readability or physical power/refresh behavior.

## Deriving the remaining Lab app

Carry the approved baseline into the existing research journey: study preview, explicit commitment, research feedback and saved finding, then related Lab views. Reuse the header, frame vocabulary, resource family, typography hierarchy and focus treatment. Adapt information architecture to the player task; do not force every screen into the same two-column template. Preserve current game rules and physical console controls. New information structures and consequential visual departures still need owner review; baseline approval is not blanket approval of every future screen or mechanic.

Preserve richness during local implementation. Match the actual baseline beside the rendered result: icon craft, spacing, frame detail, focus balance and useful visual information. Rejected local34 is not an implementation style reference. A matching palette alone does not establish fidelity.

Home and feature landing derivations are specified in the [Home contract](home-landings/README.md). Compare actual native exports against C18; the earlier control-migration pass did not approve broad visual fidelity.

Current owner playtest retains the Home information architecture but rejects its
visual fidelity as final. The next workbench follows the [connected correction](research-and-creation.md#current-correction-return-to-the-same-research-workpiece).
Inspect the actual study/finding screen as well as Home previews: a functional
landing pass does not approve a text-only finding page. Preserve illustrated
findings, intentional panel depth, controlled negative space, saturated shared
sprites and restrained warm focus halo. Do not enlarge a generic capsule to fill
the research field or replace discovery with identical progress bars. Gemini
provides the approved visual references; the production crew reconstructs reusable
masters and engineering preserves the resulting native
asset scales, anchors and effects. No current native-art final approval is claimed.

The owner explicitly rejected the native Overview styling after the input/resource
correction. Functional and readability passes do not approve its visual fidelity.
The next art proof must faithfully translate the approved Gemini references and be
compared with C18 at native size. Existing JPEG screenshot crops, flood-matted
edges and procedural outline panels are not production masters or an acceptable
substitute for that handoff. Preserve current useful information and physical
controls while the director and production artist resolve the visual craft. The
owner authorizes the [reference-production workbench](reference-production/README.md)
to develop the asset family separately before native integration.

## Screen design sequence

Information architecture, layout, content, visual references, then navigation. Keep game/hardware constraints present throughout. Identify player purpose and data relationships before choosing art placement. No physical room, workbench or second device depicted inside the device screen. The accepted directional/workspace/Back/Confirm panel drives focus, actions and feedback; no touch or invented controls.

## Typography, composition and native craft

Use the1024×600 Lab study profile at full useful resolution; it is not a procurement decision. Establish outer insets, anchors, spacing rhythm and shared dimensions before construction. Essential text must remain legible; sibling components must not resize arbitrarily with labels. Test actual-length content, whole-screen balance, margins and clipping.

Use coherent apparent pixel density and intentional detail hierarchy across resources, symbols and illustration. Typography may use finer rasterization for readability. Do not enlarge crude low-resolution symbols beside finely modeled resources and claim consistency from dimensions alone.

## Sprite-sheet production and application

The [exact baseline extraction kit](game-art-proposals/37-lab-extracted-kit/README.md) provides fixed source-pixel references, a manifest and comparison sheets. Use it to check fidelity. It is not a native sprite master set or finished resizable component library.


Author the assets needed by the current screen as one family, then reuse those exact assets. Maintain recognizable silhouettes, optical weight, light direction, shading clusters and material identity. Data is a card; Energy reads as crystal; Essence is a rounded symmetric translucent drop or sphere. Original owner references remain preserved.

Native masters need measured source dimensions, explicit display scale and deliberate compact variants. Show actual1× use and integer enlargements when claiming pixel work. Generated boards and browser previews are concept references, not native sprite masters. Inspect the whole resource family together, including stock and study sizes.

## Proposed feature-facing visual contract

The player-facing projection leads with features, discovery topics and retained findings rather than requiring players to learn loci or allele notation. The genomic engine and its knowledge rules remain authoritative. The genomic engine remains unchanged; the selected visual identity below governs its presentation. The game mapping belongs in the [player-facing genetics projection](research-and-creation.md#player-facing-genetics-abstraction--proposed-projection); device interaction belongs in the [feature-led research journey](../specs/experience.md#feature-led-research-journey--proposed-screen-projection). Content and information architecture precede screen production.

The visual subject is the feature being explored; the useful content is what this record establishes about it. A generic feature illustration identifies a topic or reference, while a finding states record-specific knowledge. Neither its presence nor its brightness proves that the sample expresses the feature. Default views need no locus grid, mandatory gene letters, copy-count lesson or invented biological network.

| Visual role | Proposed treatment and meaning |
| --- | --- |
| Feature / discovery topic | A stable name and recognizable reference glyph or compact illustration. Keep its identity across overview, study and inspection; do not redraw it as a different object for each knowledge state. A study is discovery, not a quiz or a choice of desired allele. |
| Finding | A short record-specific statement beside the feature, with supported evidence or a free inspection destination. An illustration does not replace the finding. One finding may inform several features; one feature may depend on several findings. |
| Expected outcome | A clear conditional result, with the relevant reference context available. Distinguish the record's expected appearance or behavior from an observed living individual. Use an actual specimen image only when its identity and appearance are supported and approved. |
| Known inherited information | Keep the reference art visible and use a plain knowledge caption. When expression is established as absent, identify inherited-but-not-showing potential explicitly; do not fade it into an unknown, disabled or missing feature. Before expression is established, do not call it unexpressed. |
| Not yet discovered | A labeled information gap or unfinished study destination. Do not use an empty collectible slot, missing body part, blacked-out creature or ghost image to imply absence. Unknown expression is not a negative finding. Outcomes come from the allowed engine-backed knowledge, not an icon's fill state. |
| Console focus / retained context / feedback | Only the current console target receives the focus marker. The selected record remains identifiable through quiet retained context. Pending, saved acknowledgement and error are operation feedback, not feature-knowledge states; a saved check does not mean the whole record is complete. Read-only references receive no button-like treatment or simultaneous focus. |

For Sample A before the supported Markings study, the projection must preserve that a pale-marking version is known while its adult-reference appearance is not yet established. After the accepted `Pp` finding, it may present the expected absence of pale markings and the known inherited pale-marking version under the adult/mild reference. These are two kinds of knowledge, not a single “feature absent” badge. Exact brief labels remain a content/layout decision; the preceding statements define meaning rather than prescribe paragraph copy.

Overview should expose useful features, retained findings and available discovery destinations. Free inspection should add interpretation or evidence rather than repeat the overview. Cost review makes the chosen topic, supplies, affordability and explicit commitment clear; it does not preview the undiscovered answer. Subsequent states retain the record/topic identity. Engine-backed completeness and creation eligibility are separate from the count of visible feature illustrations or finished-looking panels.

Use the selected Vault Data-card, crystal and rounded-Essence visual family. Their material colors and shapes identify supplies; they do not color genes, feature categories or movement/energy relationships. A functional energy-use finding and an Energy resource cost need distinct names and visual roles. Preserve the accepted atlas's reference illustration craft without importing its paper treatment into the primary device screen.

After game/UX content and layout settle, production builds a reusable asset sheet and applies the same assets in the representative native screen. Preserve the selected family when authoring native variants; generated concept crops are not native pixel masters. Art direction checks meaningful illustration/data relationships, stable sibling footprints, margins, spacing, optical weight and focus at the actual 1024×600 study profile. Do not substitute a decorative hero with menus, a jargon form, or labels on unexplained symbols for that visual system. Actual final exports require art, gameplay and UX review; dimensions, checklists and a declared pixel grid do not establish craft or comprehension. This proposal makes no live-control, physical-display or player-comprehension claim.


