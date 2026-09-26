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

Use **phenotype** in domain records for what we have called the expressed genome. It includes functional properties, not just visible appearance. Player-facing terminology remains a UX choice.

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

Heritability is a population/environment statistic, not a per-individual transfer probability. Breeding forecasts must name the actual probability they estimate.

### Crossing, viability and classification

Product direction permits adjacent-class crossing. Proposed adjacency is a reproductive compatibility relationship, not a position in a list or an appearance score. Evaluate in stages: compatible inputs → inherited combination → developmental viability → resolved offspring → reproductive capability. Viable but infertile is distinct from non-viable.

Compatible components can produce ordinary or exceptional combinations; incompatible interactions can prevent development. Both should be explained by versioned rules. Exceptional F1 performance and a combination that breeds true are different; later generations may split the combination. Hybrid class assignment, non-viability presentation, costs and forecast disclosure remain product choices.

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

## Founders and genomic knowledge

Accepted: researched samples can produce lab-created founders with explicitly no creature parents. Samples and class templates are provenance, not parents; parentless origin differs from unknown ancestry. Founders enter the same framework as bred offspring and can reproduce when compatible and capable.

Research supports possibilities followed by guided synthesis. Before creation, all required genomic regions must be unlocked and a complete supported genome selected; creation cannot complete missing genes or substitute a hidden lottery. How sample evidence establishes the candidate space remains open. Preserve the console-only route with truthful laboratory provenance. [Gameplay](gameplay.md) defines the research interaction; the [sample-to-critter contract](sample-to-critter-contract.md) separates sample, knowledge, selected genome, phenotype and individual, including transaction/retry boundaries.

Environmental similarity does not establish identical genomes or individual identity. Collection points are neither gene counts nor a universal strength bonus. Research overlays add knowledge without rewriting collected evidence. Opening alone does not promise loot, rarity, family membership or an encyclopedia reward.

### Genome imagery and progression

Accepted: unresolved bitmap regions represent genuinely unresolved required information; findings reveal or annotate them; supported configurations have explainable visual differences. Known carried-but-unexpressed variants must differ from unknown regions. A created individual’s genome view must relate to its actual genome; decorative pixels cannot claim genetic meaning. Encoding, region mapping and minigames remain open.

Start with small genomes and increase complexity as play progresses and alleles are discovered. Exact dimensions, triggers and content scale remain open. Visual density is not genetic complexity. New player knowledge does not mutate existing individuals or revoke their established research completeness; any actual genomic/developmental change is explicit and versioned.

### Existing fixture limits

The bounded host founder fixture uses authored `Cc / Rr / Pp`: crown and eye rings express; pale markings are carried but unexpressed, with no sample-triggered activation. Its questions concern visible structures or carried variation; choosing a direction changes study focus, not genes or probabilities. Simulated field evidence includes declared units/quality; console evidence has laboratory provenance. These scenarios do not prove sensor-to-genome generation.

Unsupported dimensions are unmodeled, not zero biology. The fixture retains one demo identity, genome, phenotype, origin and portrait across reveal/reload; previews do not consume real inventory. The [sample-to-critter contract](sample-to-critter-contract.md) proposes the complete-genome selection and resource-commitment boundary; this older fixture does not implement that production boundary.

## Generation and preservation

The [architecture pipeline](architecture.md#creature-production-pipeline) owns remote generation, automated content tooling, appearance mapping and retained assets. Genetics supplies inspectable inherited information and resolved expression; art cannot choose genes. Preserve algorithm/input/rules versions and exact outcomes; neither a random seed nor a prompt is a sufficient record. Learned state remains separate.

Rarity means frequency of a class, allele, combination or contextual expression under declared population assumptions. Uniqueness does not establish rarity, power, monetary value or usefulness. No universal rarity gene is proposed.

### Neural behavior alternative — proposed

A compact neural model is an option for expressing inherited behavioral tendencies alongside or within a constrained state/decision model. It is not a selected replacement for explicit eligibility guards. Topology, inherited representation, training, inference location and device cost remain unresolved. Learned parameters/history must be distinguished from inherited inputs; training cannot silently change transmitted genes. A model cannot authorize an unavailable capability, change ownership or choose an unapproved care consequence. Stable versioned inputs and behavior observations are required to compare this option with an explicit state model. No neural implementation or measured device feasibility is claimed.
