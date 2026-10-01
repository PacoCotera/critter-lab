# Gameplay

Status: accepted directions with proposed mechanics identified below. Production balance, recipes, timers and complete progression are not implemented.

## Combined portable and wild capture

The owner accepted consolidation of Probe and Companion into one everyday portable with **Probe**, **Cargo** and **Companions** modes. Probe supports field investigation, gathering and wild encounters; Cargo contains carried resources/findings and separately identified temporary captures; Companions concerns established travelling critters and their interaction/training/development. Mode switching is a change of view, not implicit expedition cancellation, spending or loss of cargo. Lab genome research remains distinct from field investigation. Device responsibilities are in [devices](devices.md#current-consolidated-product-architecture).

Wild critters can be captured during expeditions. Capture has difficulty, and a newly captured specimen can escape before the player brings it back to the Lab. Temporary containment and bonded companionship are distinct states: this escape concept does not make established travelling companions run away. A captured individual has observable appearance/behaviour but is not automatically a decoded genome. Its research/discovery and introduction into the home collection must respect existing knowledge boundaries.

Exact encounter/capture inputs, capacity, rewards, failure consequences and probability rules remain open. Exposure/event-driven risk, visible containment conditions and an explicit return decision are proposals; the previously discussed Stable/Restless/Unstable labels are not final rules. No real-time unattended-loss timer, species catalogue or capture algorithm is approved or implemented by this direction.

The connected journey is: choose travelling companions, explore/gather or capture, manage cargo and transport risk, return to the Lab, research discoveries, and manage residents/incubations/environments through the shared home habitat. The core kit is designed to work standalone from the box, including nearby-kit interaction. An optional Cloud Pass adds global trading and breeding, lineage, certificates and minigames. Local core progress must be durable without cloud acceptance; global operations need their own validation and recovery. Exact local/global authority, reconciliation and entitlement protocols remain open. This is product direction, not delivered functionality or approved pricing. This does not introduce a second independent caddy care game.

## Self-contained habitats

Owner direction: each habitat is a self-contained environment with its own critter populations, environmental resources and interactions. A tank, island or glacier is a simulation space, not merely a collection background. The container analogy describes bounded state and lifecycle; it does not select Docker, a process per habitat or cloud infrastructure. Habitat count and concurrent active limits remain open.

Desired capability: freeze a habitat and restore it from the cloud. Proposed meaning of freeze is an explicit pause of its simulation, preserving residents, their identity and state, environmental conditions, local resources and pending processes as one coherent saved habitat. Restoration would resume that habitat rather than create duplicate residents/resources. No elapsed-time catch-up, unattended penalty, rewind economy or freely cloneable populations is implied. These pause/restore semantics require owner confirmation before implementation; local durability, optional cloud synchronization, snapshot compatibility and transfer consistency need a bounded system-design step.

The proposed player distinction is active versus frozen habitats; merely closing a view or undocking a device must not implicitly freeze a habitat. Lab provides the visual environment and management view; the caddy summarizes habitat/storage state; Companion provides access to the same world and the chosen travelling residents. Taking a resident out must not leave a second active copy inside a frozen snapshot. Exact transfer rules, environment dynamics, resource replenishment and population consequences remain open. Device display responsibilities remain in [devices](devices.md#coordinated-docked-defaults).

## Sandbox goals and the core loop

Players set their own goals: collecting attractive or rare cosmetic combinations is as valid as developing adaptable or specialized critters. Cosmetic value need not grant a practical bonus. Experimentation and personal knowledge support research, crafting, exploration, development and social play; recipe discovery does not normally grant permission to attempt a combination. Explicit resource, compatibility and consent requirements still apply.

1. Gather field resources and samples with the Companion, then receive them at the Lab.
2. Discover sample contents, choose a study and commit its displayed resources.
3. Retain the finding; pursue a follow-up or supported creation direction.
4. Fully decode and select a complete genome before creating a parentless founder. Breeding is a separate route using actual compatible parents.
5. Incubate, deliberately open the saved individual, then study, train, breed, print and share discoveries.

The [sample-to-critter contract](sample-to-critter-contract.md) owns the proposed record and creation-acceptance boundary. [Genetics](genetics.md) owns heredity, expression and genomic completeness; [players and social play](players-social.md) owns shared equipment, consent and personal discovery.

## What each device contributes

The devices provide distinct physical experiences; see the [experience principle](experience.md#physical-experience-is-the-product). A complete software prototype is permitted while preserving those roles.

The Companion’s **Probe mode** offers real-world sampling, fictional encounters, collection progress and resources without requiring a phone. Samples carry research evidence; resources are consumable inventory quantities. Evidence points do not automatically become food or materials. [Probe evidence](probe.md) defines sensed versus generated inputs, proposed scoring and measurement limits.

Owner correction,30September2026: the Companion selects and executes expeditions.
The Lab does not choose a field expedition or assume live knowledge of an away
Companion. Its field-related view is a log of expedition records actually received,
linked to accepted resources, samples and subsequent research. Any status must state
its received/cached provenance; shared simulator memory is not wireless receipt.
[Companion profiles](probe.md#companion-selected-expedition-profiles--accepted-direction)
define the boundary. The installed map loop permits movement, source choice,
trace discovery and explicit sample collection. Current fieldcontent3 outings reuse fieldcontent2 geometry to
generate saved source positions, connected corridors and terrain; legacy
fieldcontent1 records keep their exact authored geometry. The [native proof](../docs/evidence/procedural-expeditions/README.md)
covers route variation and retained saves. No interactive map event is implemented. Owner playtest rejects repetitive routes and chance-waiting
as the central activity: the next field experience must generate genuinely varied,
saved, reachable maps and expose events with meaningful decisions and consequences.
Distinct sample acquisition and useful research follow-through must supply the
sense of accomplishment. Exact event rules, balance and difficulty remain design
choices; the [next discovery trial](../design/probe-sampling.md#next-discovery-trial)
is a bounded proposal, not an implemented feature.

The **Lab** is an ongoing exploratory workbench. Returning can reveal findings, inventory changes, research progress, developmental changes or resources running low, suggesting interventions and the next expedition. Major discoveries and individual reveals punctuate that process. Frequent interest does not establish a neglect penalty; timers, notifications and setbacks remain open.

The **Companion** centers training, evolution and bonding. One versus several carried individuals, and a larger habitat with a carried subset, remain open. The **caddy** charges the Lab and Companion, prints and summarizes habitats/inventory/status; seating adds no gameplay progress or implicit transfer.

The owner removed console-only acquisition from the product roadmap on 30 September 2026. The Companion conducts gathering; the Lab investigates accepted samples and resources. Local core-kit play remains independent of Cloud Pass and routine phone use. Research discoveries persist; current native studies resolve immediately, with no unattended research queue or missed-check-in penalty.

## Expedition continuity and return — accepted

### Field pacing — owner correction, 1 October 2026

Preparation waiting during interactive exploration is rejected as artificial
friction. The next gathering design must separate meaningful player effort and
uncertain findings from an elapsed-time reward gate. Merely hiding the preparation
bar or shortening its timer does not satisfy this direction. The candidate native proof uses immediate retained finite offers; deployment
and human-play acceptance are separate evidence gates.
The [gathering review](../design/expedition-map-study/README.md#gathering-review-exploration-and-source-decisions)
owns the proposed active action/result contract and its comparison.

Research and incubation may provide background pacing while the Companion is
used; this is a design possibility, not permission to add delays to every study.
Current studies resolve immediately. Incubation already has a provisional
20-second background timer and a separate deliberate opening. New research job
lifecycles, durations, parallelism and offline progress remain undecided. A timer
does not supply discovery depth, and readiness does not automatically spend,
create or reveal another individual.

Returning to the Lab and unloading ends the expedition. Browsing Cargo or leaving before Send does not end it. The current Cargo manifest exposes sealing terms, and one fresh Send seals the returning haul and stops collection; successful fresh Lab acceptance stores it once and ends the source expedition. A later matching receipt confirms delivery metadata; it cannot credit another copy or resume the ended expedition. The next outing starts a new expedition identity.

Early return is an end, not completion of every timed discovery threshold. It cannot grant an unearned sample, extra attempt or late reward. Preserve earned contents, committed outcomes and retained Lab research. Legacy preparation remains separate from inventory and is preserved without conversion; the current field has no preparation gate. Empty outings need an explicit finish path without a phantom haul. Optional encounters never require attendance or gate research/creation. Return reviews actual samples and resources and leads into useful research; exact pacing, events and balance remain open.

## Research and creation

### Research collection and gathering — accepted

Supplies are fungible within their own class and indivisible. A Data card can
substitute for another Data card, but not for an Energy crystal or Essence drop.
Inventory records whole collected items. Current field collection is an explicit
whole-unit Take, resolved and saved immediately. Arrival, inspection, elapsed time
and animation grant nothing. A source exposes its exact retained quantity; a
request that exceeds free cargo space rejects unchanged, without a partial award.
Field1/2 preparation and chance counters remain saved compatibility data, with
their field clock disabled and no conversion into supplies.

Genome information is **unknown**, not locked. Research discovers and decodes it; missing knowledge is distinct from lacking resources to perform a study. Players keep a collection of partially decoded genomes and choose which to research according to their interests, complexity and the resource types available in their Lab inventory. Switching the active research preserves each record's discoveries. This does not create multiple copies of a sample or confer extra incubation uses.

The Probe gathers resources of different types for Lab research and can also gather samples containing genomes to discover and research. These are distinct expedition outputs. Expedition types shape which resources are gathered; a gathering expedition need not be bound to one sample or yield a new sample every time. Signals/collection points, awarded resource quantities, sample capsules and decoded genome knowledge are distinct concepts. Their detailed conversion and capsule preparation are proposed in the [connected research design](../design/research-and-creation.md), not selected hardware or balance rules.

### Local field loop — accepted direction

Owner approved the reviewed map loop on 30 September. Companion movement follows
visible legal paths; arriving or previewing a place awards nothing. Fresh Confirm
takes a single visible whole offer directly. Multiple actual alternatives open a
chooser: directions preview, Back preserves cargo and sources, and fresh Confirm
takes the exact displayed batch. A saved result does not require dismissal before
the next direction continues travel. Revisiting or restarting cannot refill offers.

The reversible field3 fixture has CampData2/Energy2/Essence1, MossEssence12,
RelayData12 and StoneEnergy14. Taking the three remote offers yields38/40;
CampData2 fills40 while Energy2/Essence1 remain. These quantities are provisional
proof content, not canonical balance. An independent deliberate trace reveals the
sample cache; collection remains explicit and its capsule stays neutral until
Lab research. Full supply cargo does not consume the separate capsule slot.

Field1/2 saves retain their exact topology, cargo, counters and preparation but
cannot gather further; return existing cargo or explicitly finish an empty outing.
Return review defaults to Keep. Sending seals current contents until explicit
Lab acceptance credits once, ends the outing and retains received evidence.
Lab cannot infer an away position or source selection. Further encounters,
uncertainty, replenishment and environmental art remain open. The
[field design](../design/probe-sampling.md) remains the exploration reference.

### Research is discovery across expeditions — accepted

Research discoveries decode parts of the genome. **A fully decoded genome is needed to incubate a critter.** Genomes vary in complexity, with game progression from simpler toward more complex research. Show that decoding process at the Lab, not only a generic study-complete message. [Genetics](genetics.md#genome-imagery-and-progression) owns decoded/unknown representation and complexity boundaries. Exact progression gates and balance remain open.

A sample is a cache containing surprises that the player discovers through research, not a hidden question or a quiz to answer. Discoveries reveal its contents and supported possibilities while preserving sample identity and retained findings. This does not make it a hidden finished critter or change the complete-genome and deliberate-creation requirements.

Completing research requires gathering across several Probe expeditions. The player returns to the same research, retaining discoveries while obtaining what further studies need. A completed expedition is not completed research. The Probe shows actual gathering progress toward the current research needs; the Lab shows the research process, discoveries so far, remaining work and gathering needs. Keep gathering, inventory and research completion distinct. The Probe does not reveal undiscovered sample contents.

Exact expedition count, yields, study requirements, timing and progress presentation remain open. Several expeditions do not imply several new samples or a mandatory attendance schedule. Console-only acquisition is removed by owner direction; this loop uses the Companion for gathering. This direction changes the connected walkthrough: demonstrate an initial return, research progress, further gathering and continuation of the same sample.

Accepted research direction: supported possibilities followed by guided synthesis. Research supplies knowledge, not an automatic creature. Creation requires every required genomic region decoded through research and resource expenditure; incomplete research is not an optional gamble. Research groups are navigation, not replacements for the five genetic layers.

Accepted V1 structure: two initial studies and, when findings leave a relevant uncertainty, one follow-up. This is an introductory path, not a minimum click quota. Players choose investigation order and may attempt a known supported comparison early; information it resolves does not require another study. For V1, supported configurations in a sample share its required region set, and every required region must be resolved before final selection. Choosing a simpler configuration cannot bypass unknown required information.

The Lab handles procedure; the player chooses a study, sees the resource cost and receives a clear finding. No manual tuning, control-group setup or interpretation quiz is required. A missing reagent pauses the affected study while preserving findings and other work. Samples differ in clues, possibilities, interactions and supplies. Exact tests, mappings, costs and timings remain open; the [design explanation](../design/research-and-creation.md) uses hypothetical content. This does not cap later genomic complexity or guarantee rarity from a checklist.

The V1 design direction requires complete supported configurations and selection before creation. The [proposed system contract](sample-to-critter-contract.md) describes this boundary; it neither limits production to a few cosmetic configurations nor establishes an implemented generation system. The meaningful bitmap and growing genome complexity are specified in [genetics](genetics.md#genome-imagery-and-progression).

### Study variety — release requirement

Owner direction, 27 September 2026: material studies are an acceptable V1 placeholder, not a sufficient final-release study set. Design a broader range of meaningful study types and discoveries before release. The study types, content mappings and interactions remain open; this requirement does not select a taxonomy or authorize speculative mechanics. Keep V1 placeholder content explicitly labeled and do not mistake implementing it for completing research design.

## Creation inputs and retained discoveries

Accepted V1: one qualifying sample supports one founder creation. Findings, supported alternatives, the sample pattern and creation history remain in the player's research records, with their conditions and versions. Prior knowledge guides new investigations but does not automatically complete a new sample or supply its material.

Accepted explicit creation spends the chosen sample and displayed creation supplies together and saves one parentless individual. Before submission, the player may revise or leave without spending creation inputs; already-used research reagents remain spent. A confirmed rejection spends no creation inputs. Uncertain delivery checks the same request. After acceptance, leaving does not cancel, refund or reroll it. Revealing, inspecting or retrying its visuals charges nothing further.

Another founder requires another qualifying sample acquired in the field with
the Companion. Console-only acquisition has been removed. Breeding is a separate
route with separate permissions and costs. The [creation design](../design/creation-terms.md)
explains the experience. Permanent service/content-failure remedies remain open;
no technical error is an in-world crafting failure.

## Resources and crafting

Accepted: combine tangible laboratory materials with fantastical resources. Culture is a desired ingredient concept, not a universal requirement for plants, bacteria, ghosts and all other body plans. Requirements may attach to supported characteristics rather than entire categories. Resource names, currencies, nutrition compatibility, capacities and exact effects remain open.

Characteristic-related resources can occur as qualitatively very rare exploration finds and can have discoverable Lab crafting routes. Context-sensitive experimentation uses ingredient types/properties and Lab/environment conditions. Learned relationships must remain useful; variability does not permit arbitrary rerolls. Recipe knowledge, crafted material properties and genetic expression are separate records: crafting is not unrestricted gene editing or a guaranteed phenotype.

| Requirement | Boundary |
| --- | --- |
| Useful clues for unfamiliar mixtures | V1 should not be mostly blind mixing; clues need not reveal the entire outcome |
| Personal recipe encyclopedia on the Lab | Retain ingredients/elements, discovered relationships and their known effects; no automatic global unlock or publication |
| Failed crafting loses committed ingredients or returns a fraction | Report actual recovery; eligible failures, amounts and learning from failure remain open |
| Resource-specific shortage behavior | Some shortages may pause research while others allow continuation; no universal penalty is selected |
| Substantial long-term crafting network | Potentially hundreds of items/recipes and multi-step intermediates, serving research and creation; no factory automation or fixed chain depth required |
| Small initial crafting scope | Purity, yield and effectiveness variation are future ideas, not V1 requirements |

A recipe tree and a supporting app view are proposed presentations; Lab access must stand alone. Contextual recipes do not themselves prevent published guides, brute force or manipulated inputs. Exact relationships, costs, context factors and variability need a coherent small content set before expansion.

## Equipment and progression

Accepted: Lab, Probe and Companion can eventually support upgrades such as greater capacity, speed or processing capability. In-game upgrades cannot increase physical RAM/CPU, radio capability or cloud capacity. Device-specific systems need separate definition.

Lab research chips are **virtual**, follow the player's profile across Labs and occupy a limited number of slots. They are safely removable and reusable so players can specialize rather than permanently accumulate all bonuses. Physical electronics still determine whether a terminal supports a capability.

Crafting, discovery and trading are chip acquisition directions; rare-critter drops are a possible source. This selects no combat, killing, harvesting, neglect or breeding reward. Slot counts, stacking/compatibility, bonuses, fitting costs, active-job switching and drop rules remain open. Virtual chips require no physical slots or accessories.

### Research supplies and capability progression — accepted direction

Start with three research resources prepared by the Probe from its gathering. They must support research across the full variety of critter classes and physiologies; they are neither genomic information nor creature-specific ingredients. Exact identities and recipes remain open. Mineral grains, Lumen and Catalyst are rejected names retained only as labels in earlier illustrative artifacts.

Rare findings can enable retained research methods that make new genomic information analyzable. Ordinary resources fund subsequent studies; capsules supply the unknown information. Acquiring a method does not reveal its findings automatically or change genes. Major progression should combine deliberate pursuit with surprise rather than depend exclusively on indefinite rare-drop luck. Its relationship to reusable virtual research chips, consumption and exact gates remains open.

Findings should normally be proportional to player progress; much more complex genomes should be absent from ordinary early discovery or appear only as low-probability exceptions. Probe tiers with range, capacity and detection are a proposed way to express field capabilities. They do not increase physical hardware capabilities or reveal a capsule's genotype. The [gathering design](../design/probe-sampling.md#progress-proportional-discovery-and-probe-tiers) develops field eligibility versus Lab analysis, attainable upgrade paths and remaining choices. Complexity is not a universal power ranking, and existing samples never reroll on upgrade.
## Breeding and lifecycle

Breeding uses actual parents and the shared genetics framework. Whether every breeding requires a sample, and whether samples can enhance it, remain proposed. Founder creation must not fabricate parent records. Starter acquisition, incubation timing and breeding costs remain open.

Keep inherited properties, expressed capabilities, learned skills, habits and life stage separate. Growth may change appearance while preserving identifying features; acquired skills are not automatically inherited. Lifespan, transformation triggers, death, absence behavior and responsibility during loans remain unresolved in [players and social play](players-social.md#care-and-recovery). No punitive neglect system is selected.

## Social incentives

Accepted: scans should be rewarding while remaining balanced. A public code identifies a record, not ownership, breeding permission or a secret credential. The V1 owner-consented research-reference rules are in [players and social play](players-social.md#v1-social-research-references).

Proposed social extensions include joint breeding, environmental sample exchange, Companion visits and shared lineage. Offspring allocation—including a sibling egg for each participant—is an option, not an accepted entitlement. Costs, transport, privacy, grants and transfer authority remain unresolved.

Proposed scan balancing:

- Reward an eligible first individual discovery, with possible modest benefits for a new trait or family.
- Resolve screenshots, reprints and repeated scans to the same identity; inspection need not repeat rewards.
- Limit scanning's contribution so field sampling and Lab research remain valuable without requiring social access.
- Check fabricated records, self-scanning, alternate-account farms and reciprocal farming. IDs and caps alone do not establish trust.
- Distinguish remote-picture discovery from proof of travel or an in-person encounter.

Reward values, reset windows, remote/self-scan eligibility and affected progression tracks remain open. Compare no-scan, occasional, enthusiastic and repetitive play; avoid caps that create compulsory daily attendance.

## Experience checks

Can players explain inherited resemblance, enjoy one completed sample, resume research without punishment, use a printed card for a meaningful exchange and progress without field hardware? These checks assess the design; they are not claims of completed playtesting.
