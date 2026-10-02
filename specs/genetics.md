# Genetics framework

Status: accepted framework with five information layers, eleven dimension families, structured abilities and separate inherited, expressed and acquired information. Fictional genetics, not a biological simulation. Detailed rules, examples, production schema and canonical designs remain proposed unless stated otherwise.

## Agreed direction and open decisions

- Generate recognizable, extensively varied creatures entirely from their genomes and expressed contributors. Classes are emergent descriptions of results, not hardcoded inputs. V1 requires ground, flying and swimming coverage and widely different body organizations. Organism examples express desired range, not a mandatory biological taxonomy; traits need subdivisions.
- Selective breeding is central: heritable variation can become a faster or more reliable lineage under comparable conditions. Inherited contributors and expression establish tendencies; a class label never assigns performance.
- Affinities describe environments, elements and foods that support or hinder thriving. Skills/abilities need inherited structure; the exact tree or network remains open.
- Jobs must not determine genomics. Future games interpret expressed properties for potentially many activities. Playful contests and a larger crowd of critters are future game direction, not genetic categories.
- Compatible genomes may cross, potentially producing non-viable or exceptional offspring. Compatibility must follow declared inherited/reproductive/developmental contracts; taxonomic adjacency or appearance alone cannot authorize a cross. Outcomes and probabilities need definition.
- Structured genetic imagery should represent inherited or expressed information. The owner requests a reversible, versioned encoding of the layered tree as a string, providing a foundation for fingerprint art, QR transport and sharing. Exact codec and visual style remain proposed; encoding is not an ownership or permission system.
- Individual identity differs from class, genome and expression. Public scanning grants neither ownership nor breeding permission. Preserve records and finished art; see [identity and rights](players-social.md).
- The explore probe gathers environmental signals and points into a sample that needs research and can ultimately yield a lab-created specimen without parents. Such founders enter the same genetics framework as bred offspring. Detailed signal mappings and research mechanics are not yet selected.

Developmental transformations/class sequences, damaging exposures, lifespan/death rules and fantastic genes are under discussion. Procedural, genetic and LLM-driven generation frameworks are preferred with automated content tooling from the start; exact technologies remain unselected. Numerical balance, hidden-information mechanics, cross-class rules and canonical creature designs remain open.

## Accepted framework: layers and dimensions

A **layer** explains the role of information: inherited, resolved or acquired. A **dimension** describes a particular property within that structure. These are accepted domain boundaries, not a production schema or implementation architecture.

| Layer | Contents | What it does not mean |
| --- | --- | --- |
| 1. Genomic foundation and developmental vocabulary | Versioned reusable construction/expression operators, source-baseline constraints, contributor definitions and declared inheritance contracts | A hardcoded class/species body template, individual allele assignment, job or universal cap on improvement |
| 2. Individual genome | Loci (hereditary positions), variants, copy counts, provenance and regulatory variants; inherited from parents or established at lab creation; optional linked groups | Current speed, hunger, experience or one gene per dimension |
| 3. Expression and development | Versioned rules resolving inherited interactions, life stage, context and recorded developmental outcomes | A second independently inherited genome or permission to reroll a saved birth |
| 4. Resolved phenotype | Appearance, intrinsic capabilities, affinity profiles and available abilities, with context and reasons | A job profile, battle power score or identity |
| 5. Lifetime state and history | Age, current condition/resources, experience, injuries, exposures and transformation events | Automatically inheritable changes |

Identity, lineage, permissions and evidence accompany all layers rather than becoming genes. Regulatory variants belong to layer 2; rules interpreting them belong to layer 3. Temporary environment and condition feed evaluation without silently rewriting the inherited genome. Distinguish a reference-condition phenotype from current effective performance so a rested adult and a tired juvenile can be compared honestly.

Use **phenotype** in domain records for what we have called the expressed genome. It includes functional properties, not just visible appearance. Player-facing terminology remains a UX choice.

### Reversible layered representation — accepted direction

Represent the five layers as branches of a versioned creature record. Store each
inherited locus and its ordered copies once; dimension views and polygenic
relationships reference those records rather than duplicating or flattening
them. Preserve foundation/rule versions, regulatory information, provenance,
expression context and realization, resolved phenotype, and modeled lifetime
state in their respective branches. A genome-only payload and a whole-creature
payload must declare their different contents.

The owner requests deterministic binary packing followed by a reversible text
encoding. Decoding must recover the declared tree without silently truncating
branches. Compact mode may reference an exact shared, version-pinned catalogue;
portable mode must carry the foundation needed for reconstruction. A payload
must explicitly distinguish embedded, exactly reconstructable and unmodeled
information. A random seed alone is insufficient unless every dependency and
generation rule is pinned and available.

Short hash references identify retained records and check integrity; they are
not the recoverable payload. The current workbench's `#G` and `#E` references
remain lookup fingerprints. Measure encoded size on actual records before
choosing display length or QR use. Packing does not make acquired lifetime
history heritable, and a genome need not determine later experience. Exact
The separate [tree/string host proof](../prototype/generator-workbench/codec-contract.md)
now implements a bounded lossless mapping with inherited G and complete supplied
T snapshots, shared or embedded foundations and exact numeric preservation.
Its measured files establish reconstruction of current records, not a canonical
production schema, QR transport or validity of imported biological behavior.
Further field layout, catalogue availability and compatibility policy remain
bounded design work.

Owner clarification: **genome → expression/development → anatomy and capabilities
→ optional class description**. Organization, segmentation, attachments and
support/actuation are resolved from inspectable genomic contributors under
reusable compatible-assembly rules. These rules are finite and versioned; they
are not a table selecting a finished fish, insect or mammal. Baselines may constrain
actual source-supported combinations but cannot substitute a hidden class preset
for the genome's modeled construction decisions.

Classification is a downstream view with its own version/reference conditions,
not authority over anatomy, inheritance, copy counts, behavior or compatibility.
It must not rewrite a creature when a name or taxonomy changes. The workbench
manages contributor/module vocabulary and derived classification together;
organism examples do not impose heads, limbs or one universal genome scheme.
The [diversity proposal](../design/genome-starter-content.md#v1-diversity-and-genome-first-construction)
compares different construction/motion outcomes and variation within each.
Existing Pip and other bounded class-first examples remain pinned legacy proofs,
not implementations of this broader direction.

## Behavioral model — accepted direction and proposed implementations

The behavioral direction links genomes to the available behavior model, with phenotypes weighting state transitions. This establishes the direction of a genotype-linked behavioral model, not an executable state graph or numerical balance.

Proposed mapping within the accepted layers: reusable operators interpret the individual genome into anatomy, eligible actions and behavioral tendencies; expressed prerequisites select applicable behavioral rules. Those expressed properties influence eligible transitions and their weights. Environment, current condition and learned history also influence the next action without becoming inherited genes. A shared class does not require identical behavior from every individual, and a genome is not a species identifier.

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

Owner appearance direction: the generated range must include lively, coherent
pet colours rather than repeatedly dull brown/grey combinations. Expand the
inherited pigment vocabulary and inspect its expressed combinations; colour
must still come from genomic expression, not a renderer overriding the palette.
Exact palette families and mixing rules remain provisional until visual review.

Owner explicitly requires future polygenic and cross-dimension traits: movement,
energy physiology and environmental response can jointly shape locomotion,
sensory performance or metabolism. Model several inherited contributions to a
shared trait separately from interactions between resolved traits. Inherited
affinity/response properties differ from the actual environment supplied as
expression context. Structural and capability prerequisites still apply; a
favorable environment cannot invent eyes, limbs or an unavailable locomotion mode.
Catalogue records must allow multiple contributors, interaction/context rules and
per-output reasons from the outset. Exact operators, probabilities and quantitative
physiology remain design work, not an implemented general solver.

Owner also requires future **epigenetic regulation**. Proposed fictional modeling
keeps locus/region regulatory marks distinct from inherited allele copies. Marks
affect expression through declared rules; record their scope, trigger, affected
operator, establishment/removal, persistence and reproduction reset/transmission
policy. Inherited regulatory alleles remain layer2; mark interpretation/development
belongs to layer3 and acquired mark state/history to layer5. This adds neither a
twelfth dimension nor a replacement genome. Temporary fatigue, learned behavior
and an ordinary context response are not automatically epigenetic. Same-genome
experiments compare declared marks/context and their consequences; no marks are
automatically copied to offspring or treated as mutations. Exact mark mechanics
remain proposed. Biological inspiration: [NHGRI Epigenomics Fact Sheet](https://www.genome.gov/about-genomics/fact-sheets/Epigenomics-Fact-Sheet)
distinguishes sequence-preserving regulation and conditional transmission from
the widespread resetting of marks during reproduction. The game remains fictional
genetics, not a molecular simulation.

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

Recommended first inheritance proof: explicitly declared two-parent, two-copy inheritance for a small compatible genome-contract set. It is a tractable fictional subset, not a claim that bacteria, plants and ghosts share one real reproductive system. Preserve room for other copy counts and asexual/copy-based modes without implementing them speculatively.

- Use discrete variants for some visible features, small sets of contributing loci for quantitative traits, and a few explicit interactions. Dominance describes expression between variants, not which is better.
- Begin with independent loci only where stated. Later linked groups and recombination can explain traits that tend to travel together.
- Distinguish recombination of existing variation from a new mutation, whether a trait manifests from how strongly it manifests, and reference capability from trained/current performance.
- Selection can strengthen a trait or make it breed more reliably. It need not improve every child. Avoid a guaranteed upgrade operation and a universal maximum-stat target.
- Compare like conditions and preserve parent/offspring evidence. Research may reveal variants, interactions or environmental responses; what players know need not equal the full internal record. Uncertainty in player knowledge differs from randomness in birth.

Heritability is a population/environment statistic, not a per-individual transfer probability. Breeding forecasts must name the actual probability they estimate.

### Crossing, viability and classification

Product direction permits compatible crossing. Evaluate the actual inherited representation, reproductive systems and developmental constraints; a derived class label, a position in a taxonomy or an appearance score cannot supply compatibility. Evaluate in stages: compatible inputs → inherited combination → developmental viability → resolved offspring → reproductive capability. Viable but infertile is distinct from non-viable.

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

## Genome baseline, collected sample and phenotype — accepted distinction

A **genome baseline** records a source-supported genetic foundation: known/invariant contributors, required systems, applicable rule vocabulary and permitted variation within the existing five-layer framework. It is not a finished species body template, a complete individual allele assignment or a class average freshly imposed on offspring. Modeled topology decisions remain inspectable genomic inputs; expression constructs the body and any class description follows afterward.

A **collected sample** is a particular stable discovery carrying genomic information and supported possibilities consistent with a valid foundation. Research can reveal that foundation and variants distinguishing this sample from others. The sample is not merely a visible-trait fragment. It need not be tissue from an existing individual: tissue is one possible fictional origin, not a universal assumption across all critter classes. Sample provenance does not automatically create a donor parent or ancestry relationship.

A **phenotype** is the expression of a complete genome under specified development and environmental conditions, distinct from lifetime state. A trait-associated fragment alone does not establish the complete genetic information required for incubation. Assembling complementary fragments into a whole genome is not a selected acquisition mechanic.

Discover baseline families through particular samples, and discover variation within those families through further samples. Each sample still requires its own applicable findings, full decoding and selection among explicitly supported configurations. Learning a baseline does not automatically decode another sample, supply missing alleles or permit unrestricted generation. Incubation creates a new individual under the existing one-sample/one-founder rules.
## Founders and genomic knowledge

Accepted: researched samples can produce lab-created founders with explicitly no creature parents. Samples and class templates are provenance, not parents; parentless origin differs from unknown ancestry. Founders enter the same framework as bred offspring and can reproduce when compatible and capable.

Research supports possibilities followed by guided synthesis. Before creation, all required genomic regions must be decoded and a complete supported genome selected; creation cannot complete missing genes or substitute a hidden lottery. How sample evidence establishes the candidate space remains open. Preserve truthful sample provenance from the Companion; console-only acquisition is removed by owner direction. [Gameplay](gameplay.md) defines the research interaction; the [sample-to-critter contract](sample-to-critter-contract.md) separates sample, knowledge, selected genome, phenotype and individual, including transaction/retry boundaries.

Environmental similarity does not establish identical genomes or individual identity. Collection points are neither gene counts nor a universal strength bonus. Research overlays add knowledge without rewriting collected evidence. Opening alone does not promise loot, rarity, family membership or an encyclopedia reward.

### Genome imagery and progression

Owner direction, 1 October 2026: the child-facing Lab experience is a very visual
genome map. Players select regions, deliberately contribute resources to research,
progressively reveal their contents and explore supported expression branches.
Detailed genetic information serves optional deeper inspection for parents;
the main experience must convey discovery through imagery and response, rather
than walls of labels, messages or a school lesson. Reuse the [original genome-field
artwork](../design/references/genome-field/README.md) as visual lineage. Exact
region mapping, expression imagery, experimentation rules, costs and animation
remain design work. Revealing information does not install genes or mutate a
sample; supported branches need not be independently combinable.

Unknown regions represent undiscovered information, never a locked permission or purchased unlock. The player can retain several partially decoded genome research records and choose among them. Resources determine which studies can currently run; they do not make known regions become unknown when spent elsewhere. Zones are a promising owner-supported organizing direction, with visual encoding and exact biological/content mapping still to develop; zones are not automatically the five information layers or one study each.

Accepted: research progressively decodes parts of a genome. A fully decoded genome is required before a critter can be incubated; incubation cannot fill unknown regions or bypass research. The Lab must show which parts are decoded, which remain unknown and the research progress toward completeness. Decoding reveals knowledge and supported possibilities, not mutation of an existing individual.

Genomes vary in complexity, and the game has progression from simpler genomes toward more complex ones. The small V1 worked genome is an introductory example, not a universal study count or ceiling. Exact progression gates, complexity measures and research requirements remain open; do not invent levels, thresholds or assume complexity is merely more pixels.

Owner direction,1October: the genome is the research centerpiece and the causal
connection from a fully researched sample to a parentless founder, then actual-parent
breeding, lineage and variable descendants. Players should learn their creatures
through visible expression, interaction and optional detailed inspection.
Virtual Probe/Lab tiers and boosters gate progression toward much more complex,
longer investigations. Their exact effects/costs remain open. Long research must
preserve useful partial discoveries; completeness still governs founder creation.
The [connected design review](../design/probe-bench-review.md) proposes how this
feels without substituting new canonical genetic rules.

Accepted: unresolved bitmap regions represent genuinely unresolved required information; findings reveal or annotate them; supported configurations have explainable visual differences. Known carried-but-unexpressed variants must differ from unknown regions. A created individual’s genome view must relate to its actual genome; decorative pixels cannot claim genetic meaning. Encoding, region mapping and minigames remain open.

Start with small genomes and increase complexity as play progresses and alleles are discovered. Exact dimensions, triggers and content scale remain open. Visual density is not genetic complexity. New player knowledge does not mutate existing individuals or revoke their established research completeness; any actual genomic/developmental change is explicit and versioned.

### Existing fixture limits

The bounded host founder fixture uses authored `Cc / Rr / Pp`: crown and eye rings express; pale markings are carried but unexpressed, with no sample-triggered activation. Its questions concern visible structures or carried variation; choosing a direction changes study focus, not genes or probabilities. Simulated field evidence includes declared units/quality; console evidence has laboratory provenance. These scenarios do not prove sensor-to-genome generation.

Unsupported dimensions are unmodeled, not zero biology. The fixture retains one demo identity, genome, phenotype, origin and portrait across reveal/reload; previews do not consume real inventory. The [sample-to-critter contract](sample-to-critter-contract.md) proposes the complete-genome selection and resource-commitment boundary; this older fixture does not implement that production boundary.

## Worked bridge: traits, alleles, research and phenotype

**Proposed teaching example, not canonical anatomy or balance.** This applies the existing five-layer/eleven-family framework. Crown, eye rings and pale markings reuse the bounded fixture's allele rules; the movement/energy extension below is a new proposal. A small example does not replace the complete framework or authorize production incubation with unmodeled required information.

### Vocabulary and the five layers in one individual

A **trait** is a describable property, such as crown presence or locomotion efficiency. A **locus** is a hereditary position; an **allele** is a variant at that position. The **genotype** records the allele copies. **Expression rules** resolve their interactions under a declared life stage/context into the **phenotype**. Current condition and learned behavior can change performance without changing the genotype. A trait need not have one locus, and one locus may influence several traits.

| Framework layer | In this example | Research/presentation consequence |
| --- | --- | --- |
| 1 Class/body plan | Compatible example class permits a crown frill, eye rings and body markings; defines applicable structures and reproduction | Establish the applicable vocabulary. A crown allele has no universal meaning across plants, microbes or ghosts |
| 2 Individual genome | Selected candidate has Cc / Rr / Pp, with optional example movement/energy loci described below | Record both copies, including variants that will not visibly express. Decoding establishes what is present/supported |
| 3 Expression/development | C dominates c for crown expression; R dominates r for rings; pale markings require pp in the reference context | Rules explain the result. Dominant does not mean stronger/better; no hidden activation roll in this founder example |
| 4 Resolved phenotype | Crown present, eye rings present, pale markings absent; p is carried but unexpressed | The portrait must agree with this result. A carried allele is known hereditary information, not a faint marking or an unknown region |
| 5 Lifetime state/history | Same individual can later be tired, trained or injured | Current performance and acquired experience remain distinct from inherited potential and are not filled in by decoding a sample |

Research knowledge is an overlay describing which facts the player has established about these layers; it is not a sixth genomic layer. Before creation the record describes a sample and its supported complete configurations. A chosen configuration becomes the individual genome only through the accepted creation boundary. Studying does not rewrite alleles to improve the result.

### Traceable trait cards

| Trait / family | Locus and proposed alleles | Expression rule in declared reference context | Example result | What a study must actually establish |
| --- | --- | --- | --- | --- |
| Crown frill / Structure | crown: C, c; two copies | CC or Cc gives crown; cc gives no crown | Cc → crown present, c carried | Both copies/support for the configuration and the applicable expression rule; appearance alone cannot distinguish CC from Cc |
| Eye rings / Appearance | rings: R, r; two copies | RR or Rr gives rings; rr gives plain eyes | Rr → rings present, r carried | Rr rather than merely a picture of rings; no assumption that rings improve sensing |
| Pale markings / Appearance | markings: P, p; two copies | pp gives pale markings; PP or Pp does not | Pp → no pale markings, p carried; pp → pale markings | Whether the record supports Pp, pp or another defined pair, and what that means for phenotype and later inheritance |
| Burst drive / Mechanics and movement, Energy | move.drive: M, m; two copies; new illustrative content | Any M supports the proposed burst-capable form; mm supports steady form. Under the same maturity/medium/effort context, burst action has greater peak force demand and energy demand | Mm can support burst movement; this alone does not establish exact speed, endurance or learned control | The drive variants, applicable body structure and movement/energy dependency; no numerical speed is implied |
| Movement energy efficiency / Energy, Mechanics and movement | move.efficiency: E, e; two copies; new illustrative content | Any E reduces the energy required for the same supported action compared with ee under matched conditions; it does not grant a missing locomotion mode or burst capability | Mm/Ee retains the burst-capable phenotype with lower action cost than Mm/ee; absolute costs remain undefined | Efficiency pair and its interaction with the drive result; endurance also depends on capacity/condition and cannot be inferred from E alone |

These are fictional qualitative rules for a worked proposal. IDs, inheritance scheme and directions of effect are explicit; parameter values, ranges, physical units and production validation remain open. Movement and efficiency use the existing two-copy experimental scheme, not a universal rule for all classes. Traits are separate from resource names: spending Lumen does not install a light allele, and collecting Mineral grains does not select a crown.

### Genome zones are views into this structure

Proposed knowledge zones: **form** can link the crown locus and body-plan constraints; **markings** can link rings/markings loci; **movement** can link drive and efficiency, with an explicit relation to energy. These are information groupings, not chromosomes, the five layers, or proof that one study corresponds to one zone. A locus may contribute to several displayed traits without being duplicated in the genome.

For a markings example, the research sequence could be:

1. **Unknown:** the markings pair is not established. Show no invented allele or predicted coat.
2. **Partly decoded:** one p copy is established; the other remains unknown. Pp and pp have different appearances, so pale appearance is still unresolved. This is incomplete knowledge, not a selected genotype containing a literal question-mark allele.
3. **Decoded:** Pp is established for the worked supported candidate. The finding is **“Pale variant carried”**; the reference phenotype has no pale markings. A separate fully supported pp candidate, if the sample genuinely supports it, would express pale markings. Research cannot fabricate that alternative.

The visual change must show which hereditary fact became known and its consequence. A filled tile alone cannot explain carried versus expressed. Clicking a zone is not an input model; existing console controls focus, inspect and act on supported studies.

With two compatible Pp parents, the illustrative two-copy model gives PP 25%, Pp 50%, pp 25%, so pale markings can appear in 25% of offspring even when neither parent shows them. These are genotype/expression probabilities under the example rules, not research-success odds or breeding permission. This connects a discovery to an understandable later reason to keep an individual.

### Keep all eleven families visible

| Dimension family | Coverage in the worked bridge |
| --- | --- |
| Structure | Crown/body-plan applicability; final body plan remains proposed |
| Appearance | Rings and pale markings, including carried versus expressed |
| Mechanics and movement | Proposed drive/efficiency interaction; no invented universal speed stat |
| Sensing and signaling | Not modeled here; eye-ring appearance grants no sensory capability |
| Cognition and innate tendencies | Not modeled here; drive does not silently determine personality or intelligence |
| Energy and nutrition | Proposed movement cost relationship; reserves/food compatibility remain separate |
| Maintenance and protection | Not modeled here; a crown grants no implicit armor |
| Affinities and exposure response | Not modeled here; expedition origin does not automatically assign affinity |
| Development and longevity | Reference maturity must be stated; growth/transformations/lifespan not selected by this example |
| Reproduction | Bounded two-copy compatible-parent example; other reproduction schemes and compatibility remain open |
| Fantastic physiology | Not modeled here; optional structured abilities retain their own prerequisites and costs |

“Not modeled” is not absent, zero, known or decoded. For any production sample, authored content must declare which information is required, applicable, known from a valid class definition, or genuinely not applicable. Incubation needs all of that sample's required genomic information decoded, not merely all visible prototype tiles colored in.

### How this supports progression and research choices

An introductory record can teach a discrete inherited feature and carried-versus-expressed variation. A later record can introduce a relationship between loci: knowing Mm alone leaves energy demand incompletely understood until the efficiency contribution is resolved. Later content can add regulatory/developmental or structured-ability prerequisites within the same accepted layers and dimension families. Progression increases meaningful relationships, not mandatory gene count or research repetition.

The [collection/resource example](../design/research-and-creation.md#worked-collection-three-records-two-more-expeditions) now uses these explicit findings. Its study costs remain illustrative. Research tools reveal facts and supported configurations; inventory does not determine alleles and expedition points do not become genetic power.

## Generation and preservation

### Pip: accepted worked phenotype reference

Owner welcomed Pip as a useful complete qualitative example on 27 September2026. Carry it forward as the worked reference for engine/content design; this does not finalize all species content, numerical balance, lifespan policy or art. Pip is a small six-legged terrestrial critter with a rounded charcoal body, cream underside, short jointed legs/claws, amber eyes with pale rings and a soft crown frill. Cc/Rr/Pp expresses crown and rings while carrying p without pale body markings; proposed Mm/Ee supports short bursts with comparatively efficient movement.

Reference context: healthy, rested adult on firm ground in mild conditions. Structure is flexible, without a rigid shell or wings. Movement includes walking, brief dashes and low rough-obstacle clambering, without flight or specialized swimming. Sensing includes nearby visual motion and surface vibrations; signaling includes quiet chirps and frill display. Innate tendencies are moderate curiosity, caution toward sudden movement and tolerance of familiar individuals, with simple association learning. Energy/nutrition uses a proposed plant-derived profile, modest storage and recovery between repeated bursts. Protection supports minor surface repair but no limb regeneration or implicit armor. Affinities favor mild, shaded, moderately humid settings; prolonged heat/dryness can affect performance, with thresholds open. Development moves from smaller juvenile with less-developed frill to adult; no later transformation or lifespan/death rule selected. Reproduction uses the bounded compatible two-parent/two-copy example; costs/timing and exact compatibility open. Fantastic physiology is deliberately not applicable in this proposed class. Dash, clamber and signaling abilities require their actual structures/capabilities and current conditions. Age, fatigue, hunger, injuries and training remain individual state/history.

These class-baseline facts still need explicit inherited contributors and expression/content definitions before engine generation can claim full validity. Five illustrated loci are not a whole genome. Unknown/unmodeled baseline facts cannot silently become known defaults.

### Proposed genetic engine and content library

Owner requests a genetic engine capable of generating valid genomes and managing a locus library, with LLM-assisted and algorithmic generation to reduce baseline-authoring burden. The separate developer workbench is its authoring and experiment surface: it must manage the locus compendium, naming and taxonomy, with deep Structure/Appearance/Movement content, complete baseline/genome/expression inspection, structured genome art and sequence representations, genome-constrained expression sampling and resolved visual-prompt export. Eleven dimension families remain authoritative. The [workbench expansion design](../prototype/generator-workbench/README.md#authoring-engine-next-design) owns the proposed tooling journey and incremental boundaries; current five-locus implementation does not fulfill this direction. Detailed biology, framework/package selection and production integration remain proposed; no paid API access is authorized.

Selected automation direction: LLMs propose reusable loci, variants, family baselines, relationships and worked phenotype/research definitions without per-creature content authors; an explicit rule engine validates content and resolves genomes/phenotypes. Model-shaped data is not semantic proof. Published content uses approved rule operators and declared references; free-form generated explanations cannot define runtime inheritance or substitute for validation.

The locus library records identity/version, applicability, copy scheme, alleles, inheritance, expression contributors and dependencies, affected dimensions, research discoverability and worked cases. Source baselines and inherited developmental contributors compose compatible structural/physiological modules with declared invariants and allowed variation. They are not independently randomized values for every dimension or a generic preset copied into unrelated body plans.

The considered [starter content proposal](../design/genome-starter-content.md)
defines V1 breadth through genome-derived organization and ground/air/water
capabilities. Its24candidate body/surface/walk-turn records are one local
calibration case, not the whole V1 catalogue. New topology operators and the
contrasting construction/motion proof remain proposed and unimplemented. Owner's CRISPR-CAS-like expedition-item
idea is recorded there as future targeted engineering, distinct from expression
sampling and epigenetic marks; edit policy, resources and inheritance remain open.

Founder generation assembles candidates within these constraints; expression resolves them to a phenotype with an explanation trace. Breeding derives alleles from actual parents under explicit inheritance/viability rules rather than manufacturing a replacement valid child. Invalid combinations are rejected or handled by the chosen reproductive policy, not repaired by silently swapping genes. Content versions and saved genomes, expression and assets remain pinned.

The [bounded engine contract](genetic-engine.md) uses Pip's applicable baseline, a small locus set and declared unmodeled boundaries. It covers a varied batch, inherited phenotype causes, incompatible-combination rejection and one compatible cross. This is not the automatic artwork/encyclopedia generator. No whole-library editor, broad class catalogue, universal solver or production service is required for the genetic proof. Later extension follows the existing extension policy and all five layers/eleven families.

Owner further requires configured incubation to trigger an algorithmic creature
generator from the fully decoded genome and its expressed loci. It produces all
creature art, sprites, animations and encyclopedia content without per-creature
illustrators, editors or copywriters. General framework/style rules are designed
and calibrated; automatic output validation enforces them. Configuration's exact
effects remain open and cannot silently change the decoded genome. Training and
learned history still do not rewrite inherited loci.

The [architecture pipeline](architecture.md#creature-production-pipeline) owns remote generation, automated content tooling, appearance mapping and retained assets. Genetics supplies inspectable inherited information and resolved expression; art cannot choose genes. Preserve algorithm/input/rules versions and exact outcomes; neither a random seed nor a prompt is a sufficient record. Learned state remains separate.

Rarity means frequency of a class, allele, combination or contextual expression under declared population assumptions. Uniqueness does not establish rarity, power, monetary value or usefulness. No universal rarity gene is proposed.

### Neural behavior alternative — proposed

A compact neural model is an option for expressing inherited behavioral tendencies alongside or within a constrained state/decision model. It is not a selected replacement for explicit eligibility guards. Topology, inherited representation, training, inference location and device cost remain unresolved. Learned parameters/history must be distinguished from inherited inputs; training cannot silently change transmitted genes. A model cannot authorize an unavailable capability, change ownership or choose an unapproved care consequence. Stable versioned inputs and behavior observations are required to compare this option with an explicit state model. No neural implementation or measured device feasibility is claimed.
