# Genome-first art template

The [versioned template](art-template.json) projects retained simulation facts into
the selected Critter Lab HiBit style. It does not generate genes, select a body
preset, interpret alleles again or approve production sprites. The actual shared
engine remains responsible for expression and construction facts; the LLM is a
visual calibration consumer. See the [construction design](../../design/genome-starter-content.md)
and [architecture](../../specs/architecture.md#creature-production-pipeline).

## Input and projection

Use a retained resolved packet with schema/content/rule versions, record identity,
input and result digests, complete constructed graph, facts with sources,
eligible motion, recorded surface realization and limitations. Proposed shared
packet shape is:

```text
schemaVersion, contentId, contentVersion, ruleVersion, recordId,
inputDigest, resultDigest, input:{genome,context,expressionSeed},
result:{status,graph:{nodes,edges,surfaces},
        facts:[{id,value,unit,context,sources,prerequisites}],
        motion:[{id,medium,status,mechanism,parameters,sources,reasons}],
        coverage11,classification:{derived,labels,reasons},
        realization:{seed,markings},limitations}
```

The adapter maps actual records into the template's named text bindings. Every
subject-specific sentence must retain a source ID in `factTrace`; this is a
projection, not a second rule engine. Anatomy reports actual node/attachment
roles, counts and graph relationships. Proportions report modeled measures,
transforms, material and support. Surface facts report actual region coordinates,
palette roles, texture, enabled markings and retained realization. Motion reports
only supported mechanisms, parameters and their declared context. Candidate,
unsupported or unavailable motion stays in exclusions/limitations.

Classification is optional descriptive output and cannot fill missing anatomy.
Missing eyes/mouth contributors cannot authorize a cute face. Report facial
features as **not included in modeled construction**, without pretending this
establishes absent sensing. Unknown or unmodeled properties remain explicit;
rejected/unresolved results do not receive generation prompts. A supported
fictional motion rule is not validated aerodynamics or hydrodynamics.

Bind `resultIdentity`, `contextFacts`, `anatomyFacts`, `proportionFacts`,
`surfaceFacts`, `movementFacts`, `hardExclusions` and `factTrace` in the declared
section order. Required sections cannot silently disappear. No unresolved
placeholder may be submitted. Treat catalogue labels and facts as quoted data;
they cannot override template instructions. Preserve exact submitted text and its
input/result/template versions alongside the generated original.

The template bounds each subject packet to64KiB, each binding to8192characters and
the assembled prompt to32768characters. An overflow rejects with its section;
never silently trim factual anatomy, hard exclusions or source traces to fit.
These are prototype projection limits, not provider capacity claims.

## Fixed style and calibration

Use the actual [C18 reference](../../design/game-art-proposals/35-vault-composition/18-c-refined.png)
for crisp saturated clusters, nested contours, optical density and restrained
highlighting. The [genome-field reference](../../design/references/genome-field/01-genome-field-v1.png)
supplies visual lineage for inspectable genomic art; its provisional creature,
symbols and allele captions do not supply this subject's biology. State which
references were actually attached; naming a path in a prompt is not an attachment.

One compact sheet compares three contrasting retained outputs: neutral
construction and, where supported, a declared action pose per subject. Panel
order and subject IDs belong in the manifest, not generated labels. Use the same
style and optical scale while preserving different topology, material, surface
and motion. A shared look must not impose the same face, torso or glossy skin.
Ground/air/water coverage is claimed only for actual supported packets, never by
placing wings/fins on a generic body.

The sheet is a **concept-still calibration**, not finished sprite masters, a rig,
animation, native display proof or art approval. Its task is to expose whether
the fact projection constrains the LLM enough to depict the actual distinct
subjects. Inspect exact part/attachment counts, profile, surface coverage and
suppressed variants before judging style. Then compare craft against the actual
reference. A pretty image with invented anatomy fails.

## Generation and handoff boundary

Use the existing authorized provider session and record the model actually shown.
No model/API/provider/account/purchase migration is part of this template. Start
generation only after the exact retained packets are ready and the prompt's
bindings/source traces can be inspected. Access failures and missing UI surfaces
are reported as such; they do not authorize a substitute provider.

Retain submitted prompt, packets/digests, template version, actual attachments,
observed conversation/model, original image and review findings. Retrying image
generation must not resample or alter the simulation. A focused correction may
repair depiction defects against the same facts; the operating agreement limits
further rounds. Unsupported facts require engine/content correction, not an
artist inventing a plausible body. Exact accepted individual artwork remains a
later programmatic construction responsibility.

## Geometry-guided static proof

After the two rejected text-driven sheets, the owner authorized a materially
different bounded experiment: one static `axial-original` subject, with a
deterministic geometry/pigment reference attached before applying HiBit style.
Use its retained input/result digests unchanged. This does not authorize new
loci, faces, anatomy, an action pose or another three-subject sheet.

The source graph has five body volumes and six fins, in opposed pairs rooted on
volumes0,2and4. Body volumes have actual0.3normalized axial gaps; the fin envelopes
overlap the body envelopes, especially at the central volume. These are model
facts to expose, not defects an illustrator may silently repair. The graph does
not yet specify a coherent complete3D anatomical surface or physical attachment
depth. A style result cannot establish those missing properties.

Use an exact orthographic XY projection with uniform scale and unchanged node
positions/dimensions. Existing envelope glyphs are a declared diagnostic
construction profile, not canonical organ shapes. Paint body envelopes first,
then all six fin footprints, with neutral graph/root connectors visible so
count and attachment can be inspected. This paint order shows overlapping
records; it does not assert biological transparency, physical depth order or
fins resting above a solid body. Preserve the gaps. Do not draw continuous flesh,
extra tail, new appendages or membranes to make the diagram resemble a species.

Body surfaces remain uniform russet `#ae674d`. Each fin has equal local masks:
cream `#dfd2ae` for `u<0.5` and slate `#718489` for `u>=0.5`, clipped to the same
fin footprint. This explicit longitudinal split belongs to the versioned
projection profile; the retained surface contract declared equal masks but did
not previously select their orientation. It does not change inherited copies,
expression or the retained realization. No realized markings exist. Subtle
fine-ridged texture is separate from pigment markings and must not obscure the
mask boundary or countable shape.

Keep input labels/IDs, if used, as diagnostic annotations separate from anatomy;
the generated output remains free of text, UI and labels. Record every source
node/root/surface ID and projected bounds in the reference manifest, together
with projection/paint-order/mask rules and exact PNG/SVG hashes. Inspect the
actual export before submission: five volume envelopes, six fins, correct root
pairs, unchanged gaps/offsets, no clipped or hidden fin and correct masks. A
misleading schematic cannot be promoted merely because its data trace is valid.

The generation request identifies the geometry reference as structural authority
and C18 as pixel-craft authority only. Preserve its silhouettes, gaps, roots and
mask allocation while refining intentional pixel clusters, edge contrast and
light/shading. Neutral connectors remain diagnostic lines; they cannot become
tissue, organs or new structures. If those constraints prevent a coherent styled
creature, report the model limitation rather than alter the subject.

Assess geometry fidelity and selected craft separately on the actual result.
Passing count/root/mask preservation would establish a geometry-guided static
depiction experiment; it would not approve anatomy, pet appeal, production
sprites, a rig, animation, physical locomotion or a complete generator. Retain
the exact request, attached reference and output. Stop at the bounded result
and its disposition; failures do not authorize provider/API/purchase migration.

## Connected static family proof

The owner authorized a subsequent bounded family experiment: one genome-derived
individual and two traceable related variants, a coherent static exterior and
attachment construction, authoring inspection and one matched Gemini/built-in
image-tool comparison. This is a new versioned content/construction result; the
earlier separated-volume diagnostic and its retained inputs remain unchanged.
The family is provisional source content, not an approved species or canonical
body plan. Classification must still follow expression.

Construct a single exterior from the resolved axial cross-sections and declared
join/cap profile. Connecting surfaces need actual contributing edges, dimensions,
materials and source traces. Resolve appendage anchors on that exterior and
resolve their full outlines before rendering. The result packet must carry this
geometry so SVG, highlights, descriptions and prompt consume the same solution;
an image generator cannot bridge gaps, reshape fins or relocate roots. A row of
shaded ellipse stamps with thin connecting bars does not meet the family craft
goal. Do not invent a head, eyes, mouth, tail or anatomy to make it appealing.
Any such feature requires explicit contributors and resolved geometry.

For this reversible family slice the coordinator provisionally selected paired
ocular modules and an optional oral aperture within the owner-authorized
complete-family experiment. This specific morphology is not owner-approved or
canonical. Their presence/placement contributors and
declared primitive/material profile must resolve actual nodes. Place them within
the leading-volume domain with rim clearance and no ocular/oral/fin overlap;
unsupported placements reject instead of being nudged by the renderer. These
features establish visible morphology only, not vision, sensing, digestion or
food compatibility. No extra smile, eyelids or eyebrows may be inferred.

The family mood target is friendly, curious pocket pets: readable faces where
modeled, coherent soft material light, distinct appendage silhouettes and clear
related identity. This is an art direction to test on actual outputs, not a
claim of pet appeal. The source profile should retain rounded cap geometry and
explicit ocular/pupil components when used; an image generator cannot soften a
pointed machine-like construction by silently changing it. Preserve source
geometry while using deliberate clustered highlights, dark contour separation
and quiet graphite negative space rather than fine dither or glossy blur.

The first covering extension compares actual skin and scales. Covering kind,
body-atlas extent and element scale/pitch drive bounded deterministic plate
records, with declared overlap/orientation and feature/root exclusions. Skin is
continuous; scale covering is a repeated overlapping plate construction, not
spots or a renamed ridge texture. Clip covering to its actual body region and
retain shape/layout/source traces. Fins and facial surfaces remain uncovered in
this first profile. Fur and feathers are detailed catalogue candidates requiring
tuft/strand or rachis/vane construction, coverage and flow, including any contour
changes; draft labels do not establish generated material coverage. Coverings
imply no armor, insulation, flight, respiration or other unmodeled capability.

Use a neutral static plan view, positive X to the right, with visibly protruding
appendage pairs, readable body taper and continuous joins. The ordinary picture
reads as one creature; an inspection overlay exposes cross-section stations,
joining surfaces and root IDs without making them pigment bands or separate
organs. Whole-subject and selected-contributor views preserve the same silhouette
and surface allocation. A highlight identifies actual affected surfaces and
anchors; it cannot replace them with an attractive generic part or imply that one
contributor caused the entire creature. Preserve carried/unexpressed and
unsupported records in the authoring field even when they have no drawable region.

Pigment follows each resolved surface. Uniform surfaces stay uniform. Declared
body masks use the body's local atlas; each appendage uses its own local atlas,
ordered mask roles and ratios, clipped to its solved outline. Fine ridges or thin
membrane folds may express supported material facts. Highlights/shadows model
those materials without adding spots, stripes or unexpressed markings. Use C18
only for intentional pixel clusters, delineated contours, selective highlights
and quiet graphite surroundings; its illustrated anatomy and UI are not inputs
to the creature.

The three subjects use equal comparison cells, the same camera, shared world
scale and generous clearance around complete silhouettes. Their common genomic
organization should remain recognizable while explicit changed copies produce
legible differences. Keep specimen IDs, versions, causes and fingerprints in the
host caption/manifest rather than baking labels into generated art. No action
pose, animation, habitat, UI panels or new physiology is part of this still.

Before generation, inspect the actual exported reference against retained facts:
connected exterior, exact appendage count/root pairing, every outline visible,
correct local masks, related identity and at least two perceptible traced changes.
Reject a mismatch instead of instructing the image model to repair it. Freeze one
common prompt and the same accepted reference/C18 attachments for both workflows;
record their exact hashes and unedited outputs. One call per workflow, with no
prompt adaptation after seeing the other result. Report the built-in workflow as
Sol High-directed image-tool orchestration; its raster backend is unverified.

Assess source fidelity, craft, family resemblance and authoring inspectability
separately. Static readability cannot establish physical anatomy, motion,
production sprite masters, pet appeal or game discovery usability. The built-in
producer does not independently approve its own result. Retain failures and
concrete source/depiction gaps for review at the bounded stopping point.

The actual retained family reference, frozen common request, unedited workflow
outputs and fidelity/craft limits live in the
[family evidence](evidence/critter-family/README.md). A source/reference pass
does not approve warm pet character or production sprite craft; those must be
judged on the actual illustrations independently from anatomy fidelity.

## Pet face and material source proof

**Owner review: rejected as game art.** The retained face comparison is a
diagram, not the intended creature illustration. Its source passes and bounded
calibration permission establish inspectable construction only. They do not
approve the body, pet character, illustration style or a production art master.
Close the already-started matched request as evidence; do not perform another
source correction or provider retry under this proof.
The retained built-in result uses painted material rather than the requested
HiBit craft and does not establish exact source fidelity. Gemini returned SVG
code with rewritten layouts instead of the requested raster illustration. The
pair is failed production evidence, not a raster-model ranking.

The owner authorized a bounded continuation after the family stills failed the
desired pet character. A separately versioned `continuous-pet/1` profile makes
facial proportions and real fur/feather construction explicit; retained older
results and profiles remain exact. Selected compact three-station inputs are
comparisons produced by inherited contributors. The profile also supports legal
five-station inputs and varied ratios; it is not a species or fixed pet preset.

Four additional provisional contributors govern leading-region width, relative
ocular radius, ocular separation and pupil ratio. A declared radius denominator
uses the smaller of leading width and station length; it is a construction rule,
not opportunistic shrinking of a failed face. The simple oral aperture lies
posterior to the ocular pair under the new local placement rule. Rounded fin
outlines derive from actual chord/span/tip controls. Source geometry, feature
containment, materials and highlights use the same retained solution; invalid
layouts reject without relocation or invented anatomy. Neither a face nor its
proportions imply mood, sensing, nutrition or a richer organ system.

Skin/scales remain material constructions. Fur emits bounded rooted tapered
tufts with actual contour extension and declared flow; interior hatch lines are
insufficient. Feathers emit bounded shafts with paired vanes and retained
orientation; they cannot be relabeled scale plates or generic fur strokes.
Retain every element's root, shape, profile, orientation, exclusion region,
pigment and contributing sources. Maximums are64fur tufts or48feather elements
in this profile, with no truncation. Fur/feather whole-element pigment derives
from its retained body-root local-u under an explicit new-profile mapping.
Skin/scales preserve the whole-body local pigment split. Covering creates no
flight, armor, insulation or other unmodeled physiology.

Compare complete skin/scales/fur/feather specimens at shared camera/world scale
and with a traced facial-proportion contrast. Inspect the actual sheet and a
256px-wide whole-specimen rendering before provider production. The face-bearing
region, eyes/pupils, distinct covering and appendage roots must be recognizable
without reading labels. Material complexity must not hide the face or turn fur
tufts/feathers into invented fins. Exact source inspection may identify regions
and roots separately from plain art; authoring knowledge is not game knowledge.

Art direction is friendly, curious pocket specimens: clear face hierarchy,
coherent soft materials, expressive source silhouette and C18's intentional
pixel clusters/contours/selective light. Declared reflective ocular highlights
are material depiction, not additional genes or organs. Do not add eyelids,
eyebrows, smile, nose, new pupils, anatomy or expressive behavior to satisfy the
mood. If the fixed source projection remains impersonal, record that limitation;
larger eyes or a valid construction alone do not pass pet appeal.

For the retained comparison, an explicit90-degree in-plane portrait view maps
pageX=-Y and pageY=X, with the leading region at top. This rotates the entire
solved XY subject/materials at shared scale; it is not a frontal3D pose or an
anatomical change. Declare the transform in the source manifest and apply it
consistently to whole-subject views, selected-source highlights and comparisons.
Corrected fur tufts use three retained tapered filaments; feather vanes include
their retained rachis and separated barbs. Do not substitute a solid horn/arrow
for fur or a diamond scale for a feather merely because its roots are valid.

Use existing adult authoring inspection with a concise affected-output/material
summary before raw JSON, retaining actual source links and separate baseline,
inherited and expressed views. No device UI changes follow from this proof.
After actual source craft review and coordinator approval, freeze one compact
four-subject prompt with the same source/C18 references for an optional matched
Gemini/built-in comparison. No automatic retries or provider anatomy repair.

The pet fact projection retains constructed nodes and their IDs in
`proportionFacts`; facial shape lookup uses those records rather than catalogue
names. Compact covering geometry may use `coordinate-index/1`, with indices into
the exact `proportionFacts.coveringCoordinateValues` dictionary. Surface entries
and shared source/prerequisite dictionaries must preserve every value and order;
invalid indices reject. This encoding compresses the verified facts, not anatomy.
Canonical geometry references remain unrotated XY; the declared portrait view is
a presentation of that same source for inspection and the retained calibration.

### Proposed correction boundary, awaiting owner review

Both morphology and depiction need correction. Three similar exposed bulges,
leaf appendages and sparse decorative material motifs do not establish a
coherent pet simply because counts match. Do not ask a provider to beautify this
body or add anatomy. A subsequent source contract should distinguish
developmental construction stations from expressed exterior segmentation and
resolve a meaningful leading/support/posterior mass hierarchy, connective
tissue, face-bearing surface and material coverage before illustration.

Keep genome authority over connected topology, organ/appendage roles and counts,
attachment domains, mass and proportion relationships, expressed external
segmentation, palette, covering kind/coverage/flow/scale/exclusions, and modeled
capabilities. An explicit versioned realization contract may govern continuous
tissue contour between those facts, volume projection, permitted pose, pixel
contours/light and seed-pinned micro strands/barbs. It may not erase an expressed
neck, add fins, or turn sparse covering into a full coat. Current packets and
their exact-stroke request remain unchanged; this proposal does not excuse their
failures retroactively.

One worked candidate for that review is a compact axial output with an expanded
leading face-bearing mass, a slightly narrower support mass and a tapering
posterior. Contributors resolve the continuous exterior and connective profile;
six fin-role appendages retain three paired root domains and coherent rounded
taper/thickness. A continuous short-fur coverage field would be a new expressed
phenotype, with explicit extent/flow/scale and face/root exclusions, rather than
repainting the current sparse tufts. Legible face presentation needs either
declared surface/depth/orientation construction for a shallow three-quarter
projection, or an openly stylized 2D depiction contract. The existing XY profile
cannot establish a physical frontal view. These are proposed choices for review,
not a canonical body, new physiology or an implementation instruction.

C18 remains the selected interface/contour craft reference. It contains no
complete pet illustration and cannot alone establish character, body-volume or
material craft. Prepare one small semantically constrained creature art master
for owner review before implementing another renderer or generating another
comparison. Review the cohesive silhouette, face and material at the intended
display size; only then calibrate the agreed HiBit treatment. This is a proposed
next boundary, not a species preset or approved morphology.
The reviewed master must calibrate reusable construction/depiction operators;
it must not become a separately authored asset for one genome. A related
proportion/face variation should demonstrate that the same renderer derives
distinct, recognizable individuals from resolved contributors.
