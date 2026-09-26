# Genetics framework

Status: accepted framework with five information layers, eleven dimension families, structured abilities and separate inherited, expressed and acquired information. Fictional genetics, not a biological simulation. Detailed rules, examples, production schema and canonical designs remain proposed unless stated otherwise.

## Agreed direction and open decisions

- Generate recognizable creatures first, with capacity for thousands of classes and extensive individual variation. Body plans must cover animal-like creatures, plants, insects, bacteria and ghosts. Traits need subdivisions.
- Selective breeding is central: unusual heritable variation in a normally slow class can become a faster, more reliable lineage. Classes establish tendencies without fixing every individual's performance.
- Affinities describe environments, elements and foods that support or hinder thriving. Skills/abilities need inherited structure; the exact tree or network remains open.
- Jobs must not determine genomics. Future games interpret expressed properties for potentially many activities. Playful contests and a larger crowd of critters are future game direction, not genetic categories.
- Adjacent classes should cross, potentially producing non-viable or exceptional offspring. Adjacency, outcomes and probabilities need definition.
- Structured genetic imagery should represent class or individual information. Encoding and visual style are deferred until concrete examples help. It is not an identity or permission system.
- Individual identity differs from class, genome and expression. Public scanning grants neither ownership nor breeding permission. Preserve records and finished art; see [identity and rights](players-social.md).
- The explore probe gathers environmental signals and points into a sample that needs research and can ultimately yield a lab-created specimen without parents. Such founders enter the same genetics framework as bred offspring. Detailed signal mappings and research mechanics are not yet selected.

Developmental transformations/class sequences, damaging exposures, lifespan/death rules and fantastic genes are under discussion. Procedural, genetic and LLM-driven generation frameworks are preferred with automated content tooling from the start; exact technologies remain unselected. Numerical balance, hidden-information mechanics, cross-class rules and canonical creature designs remain open.

## Accepted framework: layers and dimensions

A **layer** explains the role of information: inherited, resolved or acquired. A **dimension** describes a particular property within that structure. These are accepted domain boundaries, not a production schema or implementation architecture.

| Layer | Contents | What it does not mean |
| --- | --- | --- |
| 1. Class and body-plan definition | Recognizable structural grammar, applicable dimensions, founding variation, reproduction contract and possible developmental forms | A fixed stat sheet, job, individual genome or universal cap on improvement |
| 2. Individual genome | Loci (hereditary positions), variants, copy counts, provenance and regulatory variants; inherited from parents or established at lab creation; optional linked groups | Current speed, hunger, experience or one gene per dimension |
| 3. Expression and development | Versioned rules resolving inherited interactions, life stage, context and recorded developmental outcomes | A second independently inherited genome or permission to reroll a saved birth |
| 4. Resolved phenotype | Appearance, intrinsic capabilities, affinity profiles and available abilities, with context and reasons | A job profile, battle power score or identity |
| 5. Lifetime state and history | Age, current condition/resources, experience, injuries, exposures and transformation events | Automatically inheritable changes |

Identity, lineage, permissions and evidence accompany all layers rather than becoming genes. Regulatory variants belong to layer 2; rules interpreting them belong to layer 3. Temporary environment and condition feed evaluation without silently rewriting the inherited genome. Distinguish a reference-condition phenotype from current effective performance so a rested adult and a tired juvenile can be compared honestly.

Use **phenotype** in domain records for what we have called the expressed genome. It includes functional properties, not just visible appearance. This follows NHGRI's genotype/environment distinction. Player-facing terminology remains a UX choice.

A class describes structural membership and founder distributions. Those distributions need not be re-applied as a fresh class average at every birth. Class membership rules should preserve recognizable anatomy without automatically reclassifying every unusually fast individual. Hybrids may remain explicitly unclassified until a versioned classification rule applies; similarity of appearance alone does not define reproductive compatibility.

## Behavioral model — accepted direction and proposed implementations

The behavioral direction links genomes to the available behavior model, with phenotypes weighting state transitions. This establishes the direction of a genotype-linked behavioral model, not an executable state graph or numerical balance.

Proposed mapping within the accepted layers: class/body-plan rules supply the applicable behavioral vocabulary and constraints; an individual's genome resolves through expression into capabilities and behavioral tendencies. Those expressed properties influence eligible transitions and their weights. Environment, current condition and learned history also influence the next action without becoming inherited genes. A shared class does not require identical behavior from every individual, and a genome is not a species identifier.

Keep transition eligibility separate from weighting: an unavailable capability cannot be acquired merely through a favorable random choice. Exact states, guards, timing, weighting and deterministic versus stochastic selection remain design work. This is the creature's behavioral model, not the device navigation model or merely an animation controller; presentation depicts its actions while preserving individual identity.

## Accepted dimension families; dimensions to specify

These are the accepted initial domain families, not a requirement to implement every item now. The listed dimensions are candidates requiring precise definitions. A dimension can be categorical, numeric, a set, a response curve or a relationship. Values and units need definition per dimension; no universal 0–100 scale is selected. Only applicable dimensions belong to a creature, and lack of information is not zero ability.

| Family | Candidate dimensions | Important boundary |
| --- | --- | --- |
| **Structure** | Symmetry, segmentation/branching, appendage arrangement, support material, characteristic size, proportions, mass/density, physical or diffuse organization | Supports cells, plants and ghosts without imposing heads, limbs or organs |
| **Appearance** | Pigmentation, palette, marking geometry, texture, transparency, visible emission and silhouette detail | Readability across color/monochrome and resemblance must be checked; appearance can correlate with function but need not |
| **Mechanics and movement** | Force production, load tolerance, flexibility, coordination; locomotion modes with speed, acceleration, maneuverability and effort efficiency per medium | No universal speed across land, water and air; strength and mass are distinct, and derived values should not contradict structure |
| **Sensing and signaling** | Sensory modalities with sensitivity, range and discrimination; signal channels and emission/control capacity | Detection differs from knowing what a signal means; capability differs from a learned communication system |
| **Cognition and innate tendencies** | Learning rate by task family, memory capacity/retention, response latency; exploration tendency, arousal threshold and social tolerance | Candidate fictional dimensions, not scientific claims about a universal intelligence score; habits and memories remain acquired |
| **Energy and nutrition** | Energy acquisition modes, usable nutrient/resource profiles, storage capacity, baseline demand, exertion cost, assimilation efficiency, rest cycle and recovery rate | Capacity/rate differ from current reserves, hunger or sleep debt; food-use physiology is defined here once |
| **Maintenance and protection** | Structural integrity capacity, damage susceptibility by cause, repair rate/cost, regeneration scope, detoxification and disease resistance where applicable | Current damage is state; repair speed does not imply regrowth of every structure; no single defense number |
| **Affinities and exposure response** | Preferred/tolerated temperature, humidity, light, pressure and media; elemental/chemical compatibility; exposure-response curves and food preference | Food preferences reference nutrition profiles; hazards reference protection/repair rather than duplicate resistance values. A preference is not immunity |
| **Development and longevity** | Growth trajectory, maturity conditions, stage transitions, transformation prerequisites, senescence tendency and longevity potential | Age is state. A transformation keeps individual history; no universal upgrade ladder or death policy is selected |
| **Reproduction** | Reproduction mode, inherited-copy scheme, compatibility requirements, developmental constraints, fertility determinants, offspring investment and reproductive recovery | Ability to cross, offspring viability and offspring fertility differ; breeding permission is external |
| **Fantastic physiology** | Optional mana-like storage/recovery/conversion, phase stability, spatial reach/control and other capability-specific properties | Explicitly fictional and optional. Teleportation, ghosting, summoning and cloning use the ability model below; no mandatory magic-type bucket |

Quantitative dimensions may be influenced by multiple loci; one variant may affect several dimensions. Polygenicity, epistasis and pleiotropy motivate this proposal. They do not require us to simulate molecules or give every improvement a penalty.

### Abilities are structured capabilities

Use an extensible ability vocabulary alongside the dimensions, rather than inventing a new genomic layer for every power. Each ability definition states its inherited prerequisites, required structures/resources, activation conditions, parameters, costs, effects, recovery conditions and possible failure. A tree can present it simply; the underlying prerequisites may form a network.

Examples for discussion: burrowing, light production, chemical secretion, regeneration, teleportation and intangibility. Genes can enable a capability and affect its parameters; practice can improve control without automatically changing transmitted variants. Summoning requires a later distinction between an effect, a temporary entity and a persistent individual. Cloning creates a new individual with an explicit copying/source rule, not a duplicated identity or automatic copied memories.

### Three dimension examples

| Example | Proposed definition | Inherited versus changing |
| --- | --- | --- |
| Swimming speed | Numeric distance/time under declared body size, maturity, medium and effort conditions; locomotion must be applicable | Structure and force/efficiency variants influence the reference result; current fatigue or water conditions can modify performance |
| Radiation-like susceptibility | Fictional response curve over exposure intensity and duration, referencing protection and repair | Susceptibility is a resolved property; accumulated exposure/damage is state. Mutation is a separate event, not a guaranteed benefit |
| Teleportation | Optional ability with prerequisites and range/control/carried-mass parameters | Inherited components determine potential; current resource availability and acquired control constrain use |

## Inheritance and selective breeding

Recommended first model: explicitly declared two-parent, two-copy inheritance for a small compatible class set. It is a tractable fictional subset, not a claim that bacteria, plants and ghosts share one real reproductive system. Preserve room for other copy counts and asexual/copy-based modes without implementing them speculatively.

- Use discrete variants for some visible features, small sets of contributing loci for quantitative traits, and a few explicit interactions. Dominance describes expression between variants, not which is better.
- Begin with independent loci only where stated. Later linked groups and recombination can explain traits that tend to travel together.
- Distinguish recombination of existing variation from a new mutation, whether a trait manifests from how strongly it manifests, and reference capability from trained/current performance.
- Selection can strengthen a trait or make it breed more reliably. It need not improve every child. Avoid a guaranteed upgrade operation and a universal maximum-stat target.
- Compare like conditions and preserve parent/offspring evidence. Research may reveal variants, interactions or environmental responses; what players know need not equal the full internal record. Uncertainty in player knowledge differs from randomness in birth.

The two-locus worked example shows exceptional offspring and stabilization without adding an arbitrary bonus. Real heritability is a population/environment statistic, not a per-individual transfer probability; do not use that word for our exact breeding forecasts. MedlinePlus

### Crossing, viability and classification

Product direction permits adjacent-class crossing. Proposed adjacency is a reproductive compatibility relationship, not a position in a list or an appearance score. Evaluate in stages: compatible inputs → inherited combination → developmental viability → resolved offspring → reproductive capability. Viable but infertile is distinct from non-viable.

Compatible components can produce ordinary or exceptional combinations; incompatible interactions can prevent development. Both should be explained by versioned rules. The research records biological precedents, but supplies no game failure rates. Exceptional F1 performance and a combination that breeds true are different; later generations may split the combination. Hybrid class assignment, non-viability presentation, costs and forecast disclosure remain product choices.

### Transformation, mutation and health

Individual developmental transformation differs from generational evolution. A proposed transition preserves the individual ID, original birth record and new form/event. Decide whether it changes expression only or changes inherited material; do not copy a parent's acquired final form into offspring by default.

Affinity expresses compatibility; exposure supplies context, intensity and duration. Candidate consequences include temporary suppression, damage, persistent impairment, mutation and possibly death. Separate these outcomes rather than using one negative-affinity score. Inherited longevity, repair and susceptibility differ from current health and accumulated damage. Death and unattended-time rules remain open; reconcile any choice with [game design](gameplay.md).

Preserve the birth genome. Record any later heritable revision and its cause; keep acquired nonheritable changes separate. Transmission depends on the reproduction model, including for cloning. Real animal germline/body-cell distinctions are useful inspiration, not a universal rule for every creature category. Hazard simulation does not require real hazardous field exposure.

## Extension policy

Each new dimension needs a stable namespaced ID; plain-language meaning; type and units/reference context; applicability; inherited contributors; expression rule/version; relationships/dependencies; allowable range; persistence policy; and a worked inheritance/condition example. Define absent, unexpressed, unknown and unsupported separately. A true zero is another value, not a substitute for any of them.

| Addition | Expected treatment |
| --- | --- |
| New allele or ability parameter | Extend the appropriate definition with versioned rules and examples |
| New sense, affinity or dimension | Add within the existing family unless it has a genuinely different information role |
| New family or layer | Explain why existing boundaries cannot describe it; obtain cross-domain review before schema work |
| Rule/balance change | Pin existing records and preserve outcomes; any migration is explicit, auditable and does not silently reroll a specimen |

Missing old data cannot be assigned new random genes on load. New content must declare an explicit legacy policy: unsupported/not applicable, a reproducible versioned derivation from existing data when justified, or an explicitly reviewed migration. Unknown content remains inspectable through preserved appearance/readable metadata; unsupported breeding fails explicitly rather than guessing.

Resolve inherited/developmental properties, contextual responses and temporary modifiers in a documented order, with source attribution. Define stacking and dependencies when introducing effects; avoid circular definitions. This borrows the clarity of TCG characteristic/effect separation, not their complete rules engines.

Reserve linkage/provenance now at the conceptual level; do not equate dimension families with chromosomes. These are organizational groups and can share genetic contributors. Example expansion: adding magnetic sensing extends sensing; it does not alter old visual perception or require a new genome layer.

## Explore probe and lab-created founders

Product direction: environmental signals and points contribute to a sample, the sample is researched, and the lab can create a new specimen without creature parents. This adds a creation route; it does not require a sixth genomic layer. The [game loop](gameplay.md) includes founder creation alongside breeding.

Separate **ancestry** from **origin**. A lab-created founder has explicitly no creature parents, rather than unknown or missing parents. Its origin includes sample provenance, research and a creation event. A class template or sample source is not a fictional parent. Once created, a founder has a complete genome and phenotype and can contribute variants to descendants when compatible and reproductively capable.

The mechanics below are a proposal for the next design review, not implied approval from acceptance of the framework.

| Stage | Proposed responsibility |
| --- | --- |
| Probe observations | Summarize actual measured signals, duration and changes. Sensor selection remains open; observations are inputs to fictional genetics, not real DNA collection. |
| Unresearched sample | Preserve evidence, provenance and collection progress under a recipe version. Points measure defined progress; they do not substitute for the evidence profile or automatically increase all attributes. |
| Lab research | Interpret the sample into candidate genetic possibilities and findings. Model B is selected: findings inform a later synthesis choice; the influence mechanism remains open. |
| Founder creation | Establish a valid individual genome using a class/body-plan grammar, eligible variants, research result, recorded random inputs and rules. Validate compatibility and developmental constraints. |
| Expression and reveal | Resolve phenotype using the shared framework, save creation and expression records, preserve artwork and reveal the individual. |
| Later breeding | Recombine actual founder/descendant variants under the same inheritance rules; do not reapply the founder-generation distribution to every birth. |

Recommended design to compare: sample evidence weights or unlocks compatible genetic possibilities; research reveals those possibilities and may let the player choose a bounded direction. Creation then resolves a concrete genome. Higher collection progress should not automatically mean a universally stronger specimen. The point reward function, sample contents, control offered by research and exact point of irreversible resolution remain open.

An illustrative cool/humid/low-light sample could make certain moisture-compatible variants more likely; it does not prove an aquatic body plan, mandate one class or guarantee a particular ability. Multiple independent samples from similar conditions may yield different valid founders. Reopening the same resolved event must preserve its result; it is not a fresh draw. Same environmental observations do not automatically mean the same individual.

Commit a selected creation event and its sample-use outcome consistently, preserving enough inputs and version information for replay. The established at-most-once sample-use principle should cover founder creation too; exact lifecycle transitions and offline authority need architecture review. Research retries, reveal, printing or scanning must not silently consume again or reroll an already resolved specimen. No two-parent placeholder records should be fabricated to satisfy the old prototype.

Exploration can introduce new variation into the collection; selective breeding can recombine and stabilize it. The slower console-research alternative remains an agreed way to play without the probe; do not make whole families probe-exclusive. Field evidence and simulated/lab evidence must retain truthful provenance. Detailed sample scoring, research choices and generation balance belong in the shared game design after product steering; no new implementation is authorized by this proposal alone.

### Sample bitmap and research knowledge

Product direction: opening a sample reveals its unique, initially mysterious genetic bitmap; research may explore parts of it, possibly through a minigame. See the UX [homecoming, opening and discovery direction](experience.md). The minigame and region meanings remain open; the product selected guided synthesis below. This extends the intended experience beyond the bounded founder fixture below.

Proposed semantics keep three records distinct:

- **Sample genetic potential:** a stable representation of that sample and its versioned research possibilities. Its bitmap need not be an already resolved individual genome. Preserve its recognizable identity on reopening; exact encoding, uniqueness guarantees and whether regions have genetic meaning require joint genetics/UX review.
- **Player knowledge:** discoveries, interpretations and unresolved questions about the sample. Research can add annotations or reveal meaning without rewriting underlying evidence. Record supported findings; clicking a region alone does not establish a trait or earn an encyclopaedia discovery.
- **Eventual individual:** a resolved genome and contextual phenotype with a separate individual identity and creation record. Preserve the established result after resolution. Its relationship to the sample bitmap must be explainable under the selected generation/research rules.

The product selected supported possibilities followed by guided synthesis, not a complete founder outcome fixed at collection. A pixel is not yet a gene and visual similarity does not establish compatibility or rarity. Region mapping, loot, consumption and progression rules remain separate open decisions.

### Product comparison: discover a fixed result or guide synthesis

**Latest product constraint:** creation requires a fully unlocked genome. Locked genomic parts are opened through research and resource expenditure; the product identified Probe-gathered resources. No creation from an incomplete genome or random undisclosed genome completion is permitted. This supersedes any proposal that incomplete research is merely an optional risk. Reconcile the existing B model below: whether research reveals fixed sample information or resolves a guided candidate before full unlock is still to be clarified. Do not quietly treat the earlier B description as overriding this newer gate, or infer that all sample content is already a fixed individual. Preserve console-only acquisition as an existing requirement pending explicit reconciliation.

Accepted: Research → creation V1 as a deliberately limited walkthrough/mockup. It exposes supported complete configurations through research and commits the chosen modeled genome before creation, rather than favoring a creation-time genetic lottery. This is not approval to reduce the full five-layer framework or cap production diversity at the illustrative profiles/configurations. Production-scale mappings and generated configuration space remain design work.

**Selected: supported possibilities and later guided synthesis (B).** The earlier fixed-outcome alternative is superseded.

The sample supplies supported possibilities and constraints. Research adds knowledge about them; a later explicit creation choice can guide synthesis. Research itself does not allocate an individual, edit genes or silently apply a preference. The eventual saved outcome remains distinct from the sample and from the player's intent.

Illustrative weighting, numerical chances, costs, eligibility and minigames remain unapproved. Guided synthesis must make supported choices and remaining uncertainty understandable.

**Shared boundaries:** keep sample identity and its base pattern stable; store findings as knowledge separately from sample evidence. Research overlays may change as knowledge grows. Neither model assigns an individual ID or represents a possibility as a live critter before creation. Creation establishes a unique individual with explicit parentless origin; matching patterns or outcomes do not merge identities. Reopening, retrying or repeating an already resolved inquiry preserves that inquiry's result, and committed creation preserves the same individual. A new explicit synthesis choice, where supported, is a distinct recorded action rather than a hidden reroll. Opening alone promises no loot, rarity, family or encyclopaedia reward. Console-only investigations enter the same question/finding/pursue flow with truthful lab provenance and no invented field readings.



### Bounded founder demo contract

Proposed genetics contract for the reversible, fixture-driven receive → investigate → create → meet study. This is not a production schema, a generation algorithm or approval of genetic mappings, balance or final art. Presentation and storage must preserve the same domain facts. The demo uses one authored founder outcome and two alternative source scenarios, not a claim that different research choices generate different genes.

| Domain input/output | Required meaning in the fixture |
| --- | --- |
| Evidence source | Field scenario: explicitly simulated probe evidence, with a fixture ID, declared features/units and quality. Console scenario: a lab investigation with authored observations and no invented field measurements. Retain both collection-origin category and fixture status. |
| Research question | Content supplies an ID/version, readable question, evidence prerequisites and supported finding IDs. Missing required evidence yields an explicit unsupported/needs-evidence state, not invented findings. |
| Finding and direction | Keep observed evidence, fictional interpretation, known facts and unresolved possibilities separate. Direction IDs reference the supported finding; navigation cannot invent them. A selected direction in this study changes the study focus only, not alleles, expression or probabilities. |
| Creation preview | Identifies the selected evidence, study/finding/direction and authored outcome reference. Unknown costs, eligibility or consumption rules keep this a labeled preview, not a real inventory operation. No numerical chance or research duration is implied. |
| Saved demo individual | Stable individual identity within the demo, class/body-plan reference, authored genome, resolved phenotype with context/rule version, explicit parentless origin, creation/study/source references and preserved portrait reference. Fixture records remain outside production inventories. |

Two illustrative supported questions can inspect **visible structures** and **carried variation** using the same authored starter-family record. Findings may describe a resolved crown or distinguish carried pale-marking variants from expressed markings only when the fixture actually supplies those facts. Environmental evidence does not prove either trait. The question/finding/direction interaction is selected for this prototype; these particular question contents remain illustrative and do not establish genetic effects. Do not manufacture moisture tolerance, combat statistics, abilities or a new body plan from the existing three-trait fixture.

For the smallest consistent outcome, an authored `Cc / Rr / Pp` genome under the existing provisional first-slice rules expresses crown and eye rings, with pale markings carried but unexpressed. Record no sample-triggered pale activation for this fixture. This is an example record, not a founder distribution or a claim that probe conditions selected those alleles. The phenotype must match the saved genotype and preserved placeholder art; art production does not choose genetics. Dimensions outside the fixture are unsupported/unmodeled, not zero or absent biology. This outcome is complete only for the declared demo subset.

Origin must explicitly say lab-created founder with no creature parents; unknown ancestry is a different state. Do not populate parent slots with samples, templates or starter creatures. The source scenarios may reuse the authored genotype/art without implying shared identity. Opening an unresolved possibility does not allocate a specimen ID. Once a demo creation is saved, reveal, reload and inspection reference that same identity/genome/phenotype/art; changing the research selection cannot reroll it. A deliberate fixture reset starts an isolated new demonstration and is not a player reward action.

Domain review checks: field versus console provenance remains truthful; unsupported questions cannot resolve; both source scenarios can reach the study without a phone; carried and expressed information agree with the pinned trait rules; duplicate preview/create inputs and reopen preserve one demo result; portrait failure does not generate another individual; samples and resource quantities stay distinct and unspent by fixture navigation. These are acceptance checks to run during implementation, not tests already passed. Production creation, later breeding compatibility, resource consumption and cross-device authority require separate implementation review.

## Generation, records and rendering

### Genome bitmap and progressive complexity — accepted direction

The research/creation loop must connect directly to a meaningful genome bitmap. The accepted mapping is: unresolved regions represent genuinely unresolved required information; findings reveal/annotate it; supported configuration differences have a visible explanation; creation requires all required regions resolved; the created individual's genome view preserves the relationship to its actual genome. Known but unexpressed/carried variants must remain distinct from unknown regions. Decorative pixels cannot claim to encode genetic facts. Genetics and visual design jointly define the mapping; a small truthful V1 mapping is sufficient initially.

Product progression direction: begin with small genomes; as play progresses and alleles are discovered, genomes become increasingly complex. Exact complexity dimensions, discovery triggers and content scale remain to be designed. Keep the accepted genetics layers and dimension families while starting with a small modeled set; visual density alone is not genetic complexity. Distinguish gaining knowledge about an existing genome from altering inherited material. Preserve saved individual records and previously established research completeness; any actual developmental/genomic change must be explicit and versioned, not a silent side effect of account progression. How later, more complex samples become available requires a coherent progression proposal, not assumed recipe-style permission gates.

Product architecture direction: generation runs in remote services; the connected Lab requests and presents results rather than running the generation stack on ESP32. Cloud is authoritative for durable records; devices can retain defined temporary activity pending sync. The [architecture](architecture.md) owns execution, acceptance and recovery boundaries. This does not select detailed generation algorithms or approve gameplay rules.

Content creation and management tooling is part of the system from the beginning. Authored building blocks must be ingestible, inspectable, versioned and usable by automated and procedural workflows, rather than requiring manual art for every birth. The next family proof must exercise an explicit creation/validation path and preserve output provenance; polished hand-made portraits alone cannot demonstrate scalable generation. Automated proposals remain subject to structural/genetic/content validation and product art direction. Automation can expand without replacing the core tool interfaces.

Generate constrained, inspectable hereditary information; resolve it under pinned rules; render the resulting phenotype; preserve the finished art. A random seed alone is not a genome or sufficient reproduction evidence. Record the algorithm, exact inputs, versions and resolved outcomes. LLMs may propose content and genetic algorithms may explore candidates, but no method has yet demonstrated the proposed scale or coherence. Generated text must not create unvalidated rules. Generation technology needs its own bounded comparison after the domain model is reviewed.

Proposed records remain conceptual: individual ID and origin; class/body-plan references; creation event with parent ancestry, clone source or an explicitly parentless lab origin; genome and any revisions; contextual expression snapshots; ability references; life history; samples/evidence; rules/content versions; preserved monochrome/color art and hashes. Permission and sample-use authority follow [architecture](architecture.md), not genetic appearance. Learned state remains separate.

Use authored structural constraints for attachment, proportions, incompatible anatomy, visual layering and animation. Preserve silhouette and major markings across media; color-only differences need readable monochrome representation. AI art is preserved, not assumed reproducible from a prompt. New content must not replace old individuals' art or history. Public scans remain read-only introductions with no implicit breeding or ownership rights.

Rarity has several meanings: frequency of a class, allele, combination or conditional expression. Report the relevant population or generation assumptions when estimating it. Uniqueness alone does not establish rarity, power, monetary value or usefulness. No universal rarity gene is proposed.


### Neural behavior alternative — proposed

A compact neural model is an option for expressing inherited behavioral tendencies alongside or within a constrained state/decision model. It is not a selected replacement for explicit eligibility guards. Topology, inherited representation, training, inference location and device cost remain unresolved. Learned parameters/history must be distinguished from inherited inputs; training cannot silently change transmitted genes. A model cannot authorize an unavailable capability, change ownership or choose an unapproved care consequence. Stable versioned inputs and behavior observations are required to compare this option with an explicit state model. No neural implementation or measured device feasibility is claimed.
