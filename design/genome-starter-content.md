# First locus and expression content

Status: considered **starter-package proposal**,1October2026. Game design, genetics
and procedural art direction discussed the same content and resolved construction
and gameplay objections. No new biology, numerical balance, visual identity,
package implementation or production art is approved by this document.

This supplies the [authoring workbench](../prototype/generator-workbench/README.md#authoring-engine-next-design)
and follows the eleven-family/five-layer [genetics framework](../specs/genetics.md).
The broad compendium grows across all families; its first executable package needs
connected, fully defined content rather than thousands of gene names.

## V1 diversity and genome-first construction

Owner requires ground, flying and swimming creatures and a much broader range of
body organizations. Fish, microbes, insects and land animals illustrate desired
range; they are not a mandatory taxonomy. **Classes emerge from expressed genomes,
not a hardcoded species/class selection.** The [genetics framework](../specs/genetics.md)
owns that dependency and the five-layer meaning.

The previous 24-locus proposal varies one six-legged organization. More colors,
limb lengths or gene names cannot supply a new topology or propulsion mechanism.
Retain that proposal as one local calibration case, not the V1 catalogue or a
ceiling on anatomy. The workbench now has a provisional48-record connected-volume package alongside the preserved five-locus Pip reference. Its [implementation coverage](../prototype/generator-workbench/README.md) is narrower than the full breadth proposed here.

Proposed pipeline: inherited developmental/module contributors → expression and
compatible assembly → body/surface/control graph → legal movement and other
capabilities → generated representations and optional descriptive classification.
Classification cannot choose anatomy, add missing loci, determine copy counts or
make two creatures reproductively compatible. It is a view of the result.

Reusable operators construct volumes, branches, segmentation, support materials,
attachments, surfaces and actuation/deformation channels. They are general
building rules, not species portraits or three predefined whole-body templates.
The genome must encode the modeled organization decisions, including fixed
contributors; the renderer cannot supply an unrecorded skeleton. Which topology
operations and bounds are executable is still a design choice. Generativity does
not mean arbitrary part combinations or undeclared physical capabilities.

### Small breadth proof — output cases, not selectable classes

| Proposed contrasting output | What must actually differ | Example inherited contributions and legal motion |
| --- | --- | --- |
| Ground-moving articulated body | Support graph, linked contacts, silhouette and contact-based movement | Symmetry/attachment count, segment proportions, joint envelope and coordination produce planted stepping/turning |
| Air-moving membrane-supported body | Rooted lifting/deforming surfaces, their support and control channels; not decorative wings on the ground body | Surface span, material flexibility, support geometry and actuation resolve a declared fictional aerial movement/turning rule |
| Water-moving axial body | Continuous bending organization and propulsion/steering surfaces; no renamed walking feet | Segment arrangement, axial compliance, propulsion extent and phase coordination produce swimming/turning |
| Distributed/deformable organization challenge | No mandatory head, eyes, legs, hinges or underside | Generated static volume/surface organization tests applicability; motion, reproduction and physiology stay explicitly unsupported until defined |

These are deliberately selected **genome input comparisons** for the generator,
not a menu of organism classes. Exact example bodies and fictional motion laws
remain provisional. Two related genomes for each moving case should visibly
change silhouette and a legal motion characteristic. At least one related-input
comparison must change the generated organization, attachment or deformation
graph; three fixed bodies with scalar adjustments are insufficient. The fourth initially tests
structural breadth only; it does not satisfy a moving microbial-creature promise.
A later mixed-mode case must derive both modes from compatible contributors,
not from a ground/air/water class tag. No cross-body breeding is assumed.

The proof also needs a compatible actual-parent cross in two contrasting
organizations, one suppressed/carried surface contribution and an incompatible
construction rejected without repairing inherited copies. This is a bounded
proposal, not a commitment to several complete organism simulations at once.

### Sharing content without flattening anatomy

Share copy-resolution/contribution/gating operators, causal traces and art craft.
Share an actual locus ID only when its inherited meaning and output/applicability
contract match. A leg-link record cannot become a fin or wing record by renaming
it. Surface pattern operators need the expressed body's coordinate atlas; energy
rules need the actual supported action and medium. No universal speed score,
paired-copy scheme or identical face/material is imposed across outputs.

The compendium therefore needs developmental/topology contributors alongside
proportions, pigment and movement. Candidate coverage includes symmetry and
organization, repeated regions, attachment roles, material/support, surface
regions, actuation and coordination, and medium-specific support. Numerical depth
is useful only when these compose into perceptibly different coherent creatures.
The earlier 60–90 candidate horizon is a planning aid, not a quota or breadth proof.

## Local six-legged calibration package

Propose a separately versioned worked package using flexible support, three
bilateral limb pairs and a two-link hinge construction case. Its fixed modeled
organization must be inspectable genomic content, not a class-selected renderer
preset. Existing Pip-v1 remains a pinned legacy fixture; this proposal does not
rewrite its records or claim that it follows the expanded generator.

The 24 candidate records below (8Structure/7Appearance/6Movement/3Energy) explore
variation inside that organization. They are not24mandatory research purchases
or a universal biological model. Compare squat/wide short-step and elongated/
narrow long-step individuals, lawful offspring and similar visible individuals
with different carried marking variants. First motion for this calibration case
is walk/turn. Burst needs its own legal primitive; flight/swimming belong to the
broader V1 diversity proof, not this local package's motion coverage.

## Local calibration type groups

| Order | Locus type | First expression behavior | Visible result |
| --- | --- | --- | --- |
| 1 | Body/limb morphometry | Declared pair-to-envelope maps; coupled attachment/joint constraints | Coherent silhouette, link proportions, stance and contact shape |
| 2 | Pigment contributions | Explicit dominance or codominant surface partition | Body/underside palette on the same constructed body |
| 3 | Marking/texture contributors | Recessive enable, suppression gate, spatial layout/coverage/contrast | Carried versus visible pattern; surface detail follows changing anatomy |
| 4 | Gait/control contributors | Declared coordination policies resolved against reach, contacts and joints | Distinct legal steps and turns, not an arbitrary speed stat |
| 5 | Energy support | Matched-action efficiency, capacity and recovery profiles | Traceable support/limits for existing actions; current reserves remain state |
| 6 | Context/regulatory extensions | Explicit response and retained-mark rules, after the core package | Same inherited genome can have different permitted expression/performance |

Dosage here means a declared mapping for the three two-allele pair states, not a
universal0–100scale or arbitrary averaging. Numerical construction parameters need
property-specific units/normalization and calibrated bounds before execution.
Codominance needs a defined composition; it does not permit random RGB blending.
The UI, portrait/prompt renderer and animation cannot supply their own gene rules.

## Twenty-four concrete candidate records

| Proposed ID | Alleles and first expression type | Worked consequence and dependency |
| --- | --- | --- |
| structure.body-proportion | compact / elongated; dosage→compact, intermediate, elongated envelope | compact/elongated gives the declared intermediate trunk; changes attachment spacing, not number of segments |
| structure.body-profile | low / raised; dosage→body-clearance envelope | low/low lowers trunk clearance; does not imply passing every low gap |
| structure.proximal-leg | short / extended; dosage→proximal segment envelope | short/extended gives intermediate proximal reach; preserve attachment graph |
| structure.distal-leg | short / extended; dosage→distal segment envelope | extended/extended increases distal reach within class bounds; alone grants no climb |
| structure.joint-envelope | narrow / broad; dosage→articulation envelope | narrow/broad gives intermediate allowed flexion; trajectories still require collision checks |
| structure.stance-spread | close / wide; dosage→legal contact-position envelope | wide/wide enlarges stance footprint; implies neither universal stability nor faster movement |
| structure.contact-edge | rounded / hooked; rounded/rounded rounded, any hooked hooked | hooked/rounded permits the declared rough-contact geometry; no adhesion or smooth-wall grip |
| structure.support-frame | light / braced; dosage→support/load envelope | light/braced resolves a bounded intermediate support case; no inferred armor or unmodeled mass value |
| appearance.body-palette | charcoal / russet; codominant palette composition | charcoal/russet uses the declared two-tone partition; mapping is authored, not arbitrary blended RGB |
| appearance.underside-palette | cream / slate; codominant underside composition | cream/slate resolves a specified underside partition; no nutrition or protection bonus |
| appearance.marking-switch | plain / pale; pale/pale enables markings, other pairs plain | plain/pale carries pale without visible markings, analogous to the existing Pip boundary but a new package record |
| appearance.marking-layout | bands / patches; codominant spatial layout | bands/patches specifies both allowed layout components, expressed only when marking-switch enables them |
| appearance.marking-extent | localized / broad; dosage→permitted anatomical regions | localized/broad gives declared intermediate region coverage, never changes body geometry |
| appearance.marking-contrast | soft / crisp; dosage→contrast class | soft/crisp gives intermediate contrast only on expressed markings; absence is not low contrast |
| appearance.surface-texture | smooth / fine-ridged; any fine-ridged gives fine-ridged visual surface | fine-ridged/smooth gives the declared texture; cannot grant armor, friction or grip without a separate explicit rule |
| movement.gait-rhythm | paired / alternating; dosage→paired, hybrid, alternating coordination policy | paired/alternating selects a defined hybrid phase policy, not a random animation blend; six-legged contacts prerequisite |
| movement.placement-control | broad / precise; dosage→placement constraint class | precise/precise supports the narrower placement policy within reachable contacts; not learned accuracy |
| movement.stride-bias | short / long; dosage→preferred stride envelope | short/long gives intermediate preference clipped to legal reach; no universal speed number |
| movement.burst-recruitment | steady / burst; any burst permits burst primitive | steady/burst permits, but does not force, a short burst if structural and current resource guards pass |
| movement.turn-coordination | pivot / sweeping; dosage→pivot, hybrid, sweeping turn policy | pivot/sweeping gives the defined hybrid policy; body footprint and joint/contact limits govern actual turn |
| movement.contact-release | early / late; dosage→contact-release phase class | early/late gives intermediate phase; contact geometry and gait must make it feasible, not universally superior |
| energy.action-efficiency | ordinary / economical; any economical gives lower cost for a matched supported action | ordinary/economical changes the qualitative action-cost relation; never grants a missing motion or implies infinite endurance |
| energy.reserve-envelope | shallow / deep; dosage→reserve capacity class | shallow/deep resolves the intermediate envelope; actual current reserves remain state, not genotype |
| energy.recovery-profile | gradual / prompt; dosage→declared recovery-response class | gradual/prompt gives intermediate response under matched recovery conditions; actual timing and food physiology remain open |


Names, alleles and pair mappings remain draft content. Executable records require
all three pair-state outputs, applicability, dependencies, construction/motion
bindings, typed bounds, valid/invalid cases, and a clear baseline/region/copy/output
visual mapping. A dimension can have many contributors and a locus can affect
several dimensions; keep one actual locus record with linked affected families.

Resolve the structural/appearance gates before deriving motion. Desired stride is
a preference envelope; the actual legal trajectory follows reach/joint/contact
constraints with a recorded reason. A hard requested trajectory outside those
constraints rejects. No silent allele substitution, limb lengthening or added
joint repairs an impossible motion. A valid body lacking one maneuver differs
from an invalid constructed body.

## Polygenic and cross-dimension expression

Multiple structural and coordination loci jointly resolve reach, contact and
legal gait. The expression of marking layout/extent/contrast is suppressed when
the marking gate is off, while their inherited copies remain present. These are
concrete joint-contribution and epistatic examples for the first engine extension.

Energy profiles can then interact with supported movement, current condition and
actual environment under declared rules. Do not infer vision/metabolism pathways
from movement alone or label every combination a new gene. Detailed physiology,
context response curves and epigenetic mechanics are later design, with their
interfaces retained now. Inherited regulatory alleles and acquired regulatory
marks remain distinct; ordinary fatigue/training is not automatically epigenetic.

After the core works, a bounded workbench-only mark experiment could suppress an
otherwise eligible action through a declared rule. It cannot create that action
for an ineligible genotype or alter allele copies. Target, establishment/removal,
persistence and reproduction reset/transmission must be explicit. This is a
proposed later experiment, not a starter locus or selected incubation mechanic.

## Easy visual representation is part of every type

Use the retained [genome-field art](references/genome-field/README.md) as visual
lineage, with current [C18 craft](screen-design-standard.md) guiding the game.
The adult developer framework need not copy the device layout.

| Representation | What it must communicate | What selecting a part reveals |
| --- | --- | --- |
| Baseline | Source-supported genomic foundation, inspectable developmental contributors, actual modules, applicable/invariant/variable regions and permitted construction vocabulary | Source definitions, constraints and candidate contributors; no invented allele pairs for unmodeled module internals |
| Genome part | Stable region/locus IDs, actual copy glyphs and sequence segments, carried/suppressed information | The same locus record, dependencies and affected body/surface/motion channels; surrounding field stays recognizable |
| Fully expressed genome | Resolved output glyphs and causal contribution field beside the same creature | Exact contributors/context/marks and the corresponding visible or functional output |
| Regulatory overlay | Declared marks, scope and state distinct from inherited copy glyphs | Establishment/history, affected rules, persistence/reset policy and downstream changes |

Grouping is navigation, not chromosome position. Causal edges and grouping lines
must have distinguishable meanings. Full authoring access can inspect every
applicable declared contributor; child-facing research receives only established
knowledge. Unknown, carried/unexpressed, absent and unsupported remain distinct.
The phenotype sequence/field never replaces the inherited record or its latent
variants. A replay digest is separate from this inspectable fingerprint artwork.

Derive the coordinate atlas from the actual expressed body graph. Local anterior/
posterior or dorsal/lateral/ventral axes apply only where those roles exist; radial,
axial, membrane or distributed surfaces need their own declared coordinates.
Surface masks and markings attach to that atlas, not image rectangles. Attachment
IDs remain stable through proportion changes and all views.
A selected locus/output highlights the same body, surface or rig channel. General
pixel treatment/lighting belongs to the calibrated construction style; the genome
supplies anatomy and surface contributions. Preview, sprites, motion, paper and
encyclopedia must depict one resolved creature.

## Game value and discovery

A sample can pose connected inquiries about **form/reach/contact**, **surface
expression**, and **gait/action support**. A finding may explain several linked
contributors and make another question useful; this is not one fee per locus.
Research reveals what the supported genome contains, never installs a chosen gene.
Fully decoded samples expose complete supported configurations, not independently
selectable attractive branch fragments.

Breeding uses actual parental copies. Players can combine a favorite appearance
with characteristic motion, uncover carried variation and establish family lines
without a guaranteed best child. Pet initiative/learned responses use eligible
motion and retained individual history; a training achievement does not silently
change transmitted loci. Exact research economy and behavior repertoire remain
unselected; this content discussion is not human playtest evidence.

## Future expedition engineering tools

Owner proposes **CRISPR-CAS-like expedition items** for engineering well-known loci.
This is a fictional game proposal, not selected gameplay or a real editing protocol.

First candidate operation: a researched, explicitly identified copy is replaced
with an available compatible variant. A declared viable null/inactivation can be
added only for loci with known effects and supported null variants. Regulatory
allele substitution is a later inherited edit, distinct from an epigenetic mark.
A toolkit is not itself an allele donor or permission to pick any desired trait.

The smallest authoring proof forks a fully known pre-incubation genome, retains
the original, edits one copy and compares all changed/unchanged downstream outputs
under the same context. Whole-genome/structure validation applies after the edit.
Record source genome, target/copy, replacement source, operation/content versions
and resulting inherited revision. Engineered origin is not invented parental
ancestry. Editing a saved living resident is not selected by this proposal.

The visual grammar shows inherited-copy before/after, selected cause and whole
phenotype comparison. A null symbol does not mean dragging away a body part; an
invalid essential-structure edit rejects. A change can affect several outputs or
have no visible result. Availability, knowledge threshold, use/cost, recovery,
permission and transmission require separate game design. Unrestricted allele
shopping would erase much of the purpose of gathering and breeding; targeted
engineering should complement those activities.

## Acceptance and deferred detail

Inspect the baseline, contrasting bodies/coat/walk/turn, a lawful offspring, a
carried-but-unexpressed marking case, an unavailable maneuver and a rejected
construction. Each valid result needs the same inherited sequence, expression
field, causal trace and generated creature, with replay retaining exact outcomes.
Inspect both complete workbench access and a partial-knowledge projection. Art
must inspect actual outputs against the original references; this document is not
that art review or an implemented fully expressed genome.

Variable body topology and ground/air/water coverage are required design scope
for the broader generator; they are not supplied by this local package. Exact
topology operators need their own bounded implementation proof. Defer unrestricted
hybrids, general linked groups/recombination, broad variable-copy reproduction,
quantitative metabolism/vision, transformations, learned models, mutations,
engineering gameplay and broad epigenetic networks. The framework/first compendium implementation is documented in the workbench. This older local package proposal is not its executable content or an approval of canonical biology. No native build or sandbox deployment follows from the host authoring proof.
