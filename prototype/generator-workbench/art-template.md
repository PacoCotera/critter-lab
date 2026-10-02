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
