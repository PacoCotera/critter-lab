# Game design

Status: concept foundation. This describes intended play, not implemented behavior. Numerical balance, sample recipes, and timers remain open.

The [player/assets/discovery draft](players-social.md) defines rights and sharing boundaries; the [cloud-sync draft](cloud-sync.md) identifies technical consequences. Proposed defaults do not establish approved gameplay rules.

Current research direction is **B — supported possibilities and later guided synthesis**. See [genetics](genetics.md). Exact influence and balance remain open; creation requires the fully unlocked genomic knowledge described below.

## Sandbox and equipment progression — accepted direction

Player goals are self-directed: collecting attractive or rare cosmetic combinations is as valid as pursuing adaptable or specialized critters. Do not make practical performance the universal measure of collection value or require cosmetic traits to grant a gameplay bonus. The genome-linked behavioral direction is recorded in [genetics](genetics.md); detailed transitions and balance remain open.

### Players and shared Labs

Accepted: multiple players sharing one physical Lab, with separate critters, inventory and discovered knowledge in cloud-authoritative profiles. Switching the active user must not merge ownership or records; background progress belongs to the appropriate player, not whoever is using the terminal. Exact offline switching and in-flight operation behavior require design.

Accepted: virtual Lab upgrades, research chips and the equipped configuration follow the player profile across Labs, rather than belonging to a shared physical terminal. Switching profiles must not transfer chips or grant the previous player's bonuses. Physical device capabilities still constrain what a terminal supports; profile equipment does not upgrade its actual electronics. Compatibility presentation and how ongoing jobs relate to equipment changes remain open.

Accepted: the household goal: one physical kit can serve multiple players. Probes may be shared, with each expedition assigned to one player; cargo and progress from that outing remain attributed to that player even when another player later uses the Probe. Changing the current operator cannot transfer an earlier expedition's pending records. Exact handover, offline retention/capacity and syncing UX remain to be designed.

Accepted: include the Companion in shared-kit use. It can switch between players while their critters, training and progress remain attributed to the correct profile. Sharing physical equipment does not transfer specimen ownership or grant care/breeding rights over another player's critters. Carrying one versus multiple critters and detailed offline handover remain open. Ordinary player switching is separate from changing a device's registration or owner.

Profile access needs an identity mechanism. Proposed: tapping a Probe or Companion, or using an NFC identity card. These are candidate interactions, not a selected reader, radio, credential or hardware purchase. Distinguish recognizing/selecting a profile from authorization to view private state or perform actions. Device registration, player identity and specimen ownership remain separate. Lost tokens, recovery and confirmation for consequential actions require a bounded architecture/UX proposal without assuming every player owns a portable.

### Personal encounters and social samples

Rejected: global first-discovery recognition as unnecessary complexity for the initial design. Discoveries are personal; do not add first-discoverer rankings or rewards to V1. Meeting another player's critter can add an encounter entry to the visitor's encyclopedia while specimen ownership remains unchanged.

Proposed: obtaining a sample or similar benefit from another player's critter to encourage encounters. Encounter knowledge, a received research reference and specimen ownership/permissions stay distinct. Public scanning alone does not grant a sample or reward.

Selected: explicit specimen-owner agreement before another player may take a sample. Merely presenting, meeting or scanning the critter is insufficient. Sample consent does not authorize breeding, ownership transfer or unrelated private-data disclosure.

Selected: **research-reference samples for V1**. They enable study of specifically shared properties and clues that can inform experiments using the recipient's own materials. They supply no donor alleles, founder material, breeding input or generation weights. A learned clue does not expand the creation candidate set or guarantee the donor's traits. Sample collection causes no damage/depletion/change to the source in V1. References are recipient-bound, with no tradeable creation stock or repeat-copy supply/point reward. Issue consent names the recipient and disclosed study scope; it does not authorize a full private genome export or recurring collection.

Reference samples do not make the studied specimen an ancestor of a separately created founder. Specific findings, effort, consent presentation and limits on new references remain open; later genetic-material donation would require a separate rule and consent.

Sandbox play must inform every aspect of the game: players experiment, choose goals and pursue approaches rather than following a compulsory sequence of recipe unlocks. Apply this principle to research, crafting, exploration, creature development and social play. In the recipe discussion, product chose this direction over research first unlocking permission to attempt combinations. Knowledge provides clues and a record of discoveries; it is not by default permission to try. Inventory, equipment capabilities, compatibility and other explicit game rules still need definition; sandbox does not remove player consent or authorization.

The Lab, Probe and Companion should eventually support upgrades such as more capacity, greater speed and more processing capability. Technology levels are an inspiration for progression rather than a specified mechanic. The Lab can be equipped with different research chips that boost different aspects of play. Exact upgrade tiers, acquisition, fitting limits, chip effects, switching costs and balance remain open. This is a programme requirement, not approval to build a large upgrade system in the first crafting prototype.

Lab research chips are virtual equipment. Product identified crafting, discovery and trading as acquisition routes; drops from rare critters are also a possible source. What triggers a critter drop remains open: no combat, killing, harvesting, neglect or breeding reward rule is implied. Recipes, drop eligibility/rates, chip identity and trade rules still need design. This does not establish that all device upgrades use the same chip system.

Accepted: a limited number of Lab chip slots, with chips safely removable and reusable so players can reconfigure equipment for different projects rather than permanently accumulate every bonus. Slot counts, compatibility/stacking rules, timing during active research and any fitting costs remain open; do not invent destruction or consumption on removal. This acceptance concerns Lab research chips, not every device upgrade.

Distinguish simulated game capabilities from actual electronics: an in-game processing upgrade does not increase ESP32 RAM/CPU or cloud capacity. Virtual research chips do not require physical slots or accessories. Future designs should let players steer specialization; detailed bonuses and tradeoffs remain to be designed.

## Core loop

### Device motivations — accepted direction, initial experience

The Probe offers wonder through real-world sampling and collection: sound, light and environmental conditions inform the experience, with visible collection/points progress. Exact sensors, mappings and scoring remain open.

The Lab is an exploratory workbench and ongoing research control terminal. Players unlock samples, steer discovery and watch incubation/creation progress. Returning should reveal useful changes: inventory and points, research progress, findings or developmental changes, setbacks and resources running low. Those changes suggest interventions and what to seek on the next outing. Occasional major discoveries and new-critter reveals provide surprise. This is more than choosing between research and a final reveal; the changing process itself should invite frequent checking. Exact timers, setbacks, resource costs, notifications and consequences of absence remain unselected; frequent interest does not itself approve a neglect penalty.

The Companion centers training, evolution and bonding with individuals. Single-critter versus multi-critter use, and a larger virtual habitat with one or a few carried on the Companion, remain open alternatives. No population/capacity decision was made.

Product direction on resource exhaustion: behavior depends on the resource; some shortages may pause research while others permit it to continue. Do not apply one universal pause or penalty. Resource categories, acquisition and exact consequences need game-designer proposals for product discussion, not a request that the product invent the economy. No specific setback or loss rule is approved yet.

Product resource direction: mix tangible laboratory materials with fantastical resources. Culture is a desired ingredient concept, but critters include non-animal and non-biological forms such as ghosts; ordinary culture cannot be a universal creation requirement. Anima, spirits, electricity and ethereal substances were exploratory examples, not approved resource names, mappings or currencies. Design resource applicability around the supported creation/maintenance process, without imposing animal biology or a fixed one-resource-per-family taxonomy. The earlier two-supply proposal remains a discussion example, not a complete or approved economy.

Clarification: ghost is only an example within a broad range of critters, not one half of an animal/ghost division. Resource requirements may attach to a characteristic the sample can support, rather than a whole critter category. Research can reveal a route to unlocking its expression; exact effects on founder synthesis, phenotype, inherited variants and later development require genetics reconciliation under selected research B.

### Resource discovery and Lab crafting — accepted direction

Characteristic-related resources can be found randomly through exploration at qualitatively very low drop rates, and players may discover recipes to craft them in the Lab. Mixing ingredients should offer an alchemy-like discovery experience: outcomes of an unfamiliar combination are not fully known beforehand. This links exploratory finds, experimentation and purposeful collection to research and potential expression.

Refinement: crafting should remain experimental and context-sensitive, using ingredient types/properties and Lab/environment conditions rather than one universally fixed recipe list. The concern is that published exact recipes could trivialize discovery. The liquid-plus-solid producing a paste potentially useful to a mud-related critter is an illustrative relationship, not an approved recipe or creature category. Semi-random outcomes are an exploration direction, not a chosen probability model.

Required: reliability: players must be able to use what they have learned. Purity, yield and effectiveness are candidate later dimensions; an insufficient quantity of an ingredient might lower yield or effectiveness rather than always causing total failure. Define which relationships are dependable and which outcomes vary before implementation; do not interpret reliability as approval of universally deterministic recipes, or experimentation as permission for arbitrary rerolls. Exact resource properties, context effects, variation, costs, shortage thresholds and failure rules remain open. Contextual recipes alone do not prevent guides, brute force or manipulated inputs.

Do not turn crafting into unrestricted gene editing, a guaranteed phenotype or a fixed genome inside every unresearched sample; resolve those boundaries with the genetics framework. Recipe knowledge, crafted resource properties and genetic expression are separate records and rules.

Scope clarification: purity, yield and effectiveness variations are future ideas, not first-version requirements. Retain extensibility without implementing these mechanics now.

The Lab encyclopedia must let players refer back to discovered recipes, ingredients/elements and what they do. A recipe/element tree is a proposed presentation, not a selected navigation structure. A corresponding mobile-app section is a possible supporting view; Lab access must stand on its own. For contextual crafting, the recorded knowledge should distinguish what was learned from unsupported guarantees; exact entry fields and disclosure rules remain to be designed. No automatic sharing, global recipe unlock or publication was approved.

Selected: personal recipe discovery: learned recipes enter that player's encyclopedia, not an automatic unlock for all players. Sharing is a separate future design topic. Keep a player's discovered knowledge separate from the system's recipe/content definitions; this does not select secrecy guarantees, shared-household behavior or a rule preventing experimentation with an undiscovered combination.

Intended scale is a substantial crafting network, using Factorio as a reference for recipe depth: potentially hundreds of items/recipes, inventory ingredients repeatedly combined into intermediates, and rare materials reached through substantial resource investment and multiple prerequisite crafts. Six or seven prerequisite crafts was an illustrative example, not a fixed depth or first-version quota. The purpose is critter research and creation; this does not select factory automation, belts, a production-line UI or Factorio's exact mechanics. The earlier two-supply starting proposal must not constrain the long-term system to two currencies. Preserve contextual experimentation and learned reliability while designing the recipe dependency graph; breadth, pacing, costs and the minimum initial content set remain open.

Product V1 scope correction: unfamiliar mixtures need useful clues; mostly blind mixing is not the selected direction. Keep the first prototype's crafting scope relatively simple, with expandable mechanics rather than implementing the full long-term network. A clue should guide an attempt without necessarily disclosing the exact outcome. The initial item/recipe counts, clue wording, context factors and unsuccessful-mixture effects remain to be proposed. No hundreds-item catalogue, deep-chain quota or purity/yield/effectiveness system is required for V1. Extensibility is a design constraint, not permission for speculative infrastructure or implementation before core design review.

Decision on failed crafting: failure wastes the committed ingredients or returns only a fraction. Partial recovery should be explicitly reported (suggested player wording: “You recovered…”), with the actual recovered items/quantities. Which failures permit recovery and the amounts remain open; do not imply guaranteed recovery or choose a rate. Whether failure also reveals new knowledge is not yet selected. This updates the previously open unsuccessful-mixture consequence without adding a quality/yield simulation to V1.

### Journey

Accepted: the initial research interaction direction: inspect supported sample clues → choose an investigation → commit its displayed resources and run an experiment → observe and retain a finding → pursue a follow-up or possible creation direction. Findings enter personal knowledge; research does not automatically create an individual. Bound the first worked design to one sample, two investigation choices and one meaningful follow-up. This approves the loop, not exact findings, gene mappings, costs, timers or implementation. A concrete example is required before implementation.

Product V1 engagement constraint: experiments must remain simple; elaborate setup can harm engagement. Proposed interpretation: a small set of meaningful investigation choices with the Lab handling comparison/setup details. Do not require manual temperature/light parameter tuning, control-group configuration, timed inputs or a scientific interpretation quiz in V1. Show a clear result and suggested next action; deeper evidence may be optional. Specific buttons, costs, pacing and presentation still need UX design. Players need not perform each scientific setup step. Preserve choice, discovery and useful uncertainty without turning routine research into a chore.

Proposed: a small set of recognizable experiment types, such as material and energy tests, and a full battery of tests contributing to a good outcome. Category names beyond those examples, applicability per sample, and the meaning of a good outcome are not settled. Discuss whether broad testing improves knowledge/control/reliability or is a prerequisite to creation; do not silently impose a compulsory checklist against the sandbox principle or guarantee better rarity/stats. Tests should remain simple selections with Lab-handled procedure. This reopens the research coverage/outcome relationship while preserving the low-complexity interaction requirement.

Product resolved the creation gate: **creation requires a fully unlocked genome; creation from an incompletely researched genome is not allowed.** A genome can have locked parts opened through research and expenditure of resources gathered with the Probe. Research therefore completes required genomic knowledge, not merely optional confidence before an uncertain creation attempt. Keep the simple test-category interaction and visible progress through locked parts. Sandbox freedom applies to choosing projects, research routes, resource gathering/crafting and equipment; it does not remove this explicit prerequisite.

Research is connected to the meaningful genome bitmap, including unresolved regions, findings, supported configuration differences and complete-genome readiness. Product also wants small starting genomes and increasing complexity as play progresses and alleles are discovered. The [genetics mapping and progression requirements](genetics.md) own the distinction between knowledge, genetic content and display. Prototype research sections cannot be disconnected checklists over an arbitrary decorative pattern.

What is fixed in a sample before research, how guided synthesis B operates within this gate, and how genomic parts map to tests/resources require one reconciled genetics proposal. Do not infer exact allele sequences, a one-test-per-gene rule, guaranteed rarity, or random undisclosed genome allocation at creation. The previously accepted console-only path remains a separate requirement to reconcile with Probe-sourced resources; this statement alone does not remove it.

Accepted: the V1 research-to-creation walkthrough as a bounded happy-path/mockup design, explicitly noting its limited coverage of the much broader genome. It uses stable supported configurations, research to reveal all modeled genomic information and selection before creation, with a small content/test scope. This acceptance applies to the illustrative V1 loop, not a reduction of the five-layer genetics framework or a production ceiling of three profiles/few configurations. Research screen groups are not genome layers. Broader generation and non-happy-path behavior remain unimplemented and require scoped work.

1. Collect field observations with the explore probe or start a console-only lab investigation.
2. Form a sample, inspect its evidence, and research its possibilities.
3. Create a parentless lab founder from researched material, or select compatible parents for breeding. Starter acquisition and detailed research choices remain to be designed; neither path requires meeting another player.
4. Preview the supported outcomes and commit the chosen creation or breeding process.
5. Incubate and reveal a unique individual with explicit origin; record parentage only when parents exist.
6. Raise it, study its traits, selectively breed, print it, and share discoveries.

Founder creation and breeding are separate paths into the same [genetics framework](genetics.md). A founder has an explicitly parentless origin, not missing or invented parents. For breeding, the provisional sample enhancement must preserve inheritance from the actual parents. Whether a sample is required for every breeding remains open. Evidence mappings, research choices, points meaning and when an outcome becomes fixed are not selected. The breeding experiment implements a small two-parent path. A separate lab fixture exercises research and deliberate reveal of an authored parentless founder; it does not yet generate founder genomes from real samples.

## Field sampling

The sampler has an e-ink screen showing sample formation, completed packs, and the evidence behind progress. Inputs include light, temperature, humidity, movement/vibration, and sound. Pressure is a candidate additional input.

Proposed scoring uses sustained observations, meaningful transitions, and combinations. Points alone are insufficient: each recipe also has required evidence. Cap repeated contributions so leaving a device under one lamp does not continuously farm complete samples. Show which observation earned each contribution and what is missing.

Illustrative recipe only: a mist sample needs humid exposure, subdued light, and cool conditions. The mockup's 40-point target and readings are artwork, not approved thresholds. Temperature readings must account for body and charging heat; device placement affects all observations. Pause environmental scoring while charging. A screen can explain why readings are excluded.

Use summarized sound levels or patterns, not recordings or speech recognition. Store only evidence needed for gameplay; location tracking is not required by the concept.

## Expedition resources

Probe expeditions also gather resources for laboratory use and critter feeding, independently of the phone. Resources are consumable inventory quantities; samples carry research evidence. Evidence points do not automatically become food or materials. Item types, yields, storage limits, nutrition compatibility and transfer/consumption authority remain proposals in [probe evidence](probe.md). Standalone operation does not select random event generation; event scheduling remains open.

## Research without the sampler

The Incubator produces samples through slower lab investigations. A player chooses a question, observes intermediate findings, and may choose which result to develop. Research must be interesting in its own right and must not require a field sample to begin. Avoid entire creature families exclusive to sampler owners.

Proposed states: queued, researching, finding available, ready, collected. Timers, optional decisions, and whether every experiment has an intermediate choice require playtesting. Do not make missed check-ins destroy work.

## Sample handoff

Transfer transport remains unselected; BLE is a proposal. Whole-haul transfer uses distinct local Lab storage, Probe clearance and completion acknowledgments. See the architecture for its conservative host contract and the separate cloud-acceptance gap.

Transport copies are not extra spendable items. Importing is distinct from consuming. The proposed consumption point is successful incubation creation, with an atomic record linking the sample and egg. Offline copying, trading, and cross-instance rules remain open; do not claim that a unique ID alone prevents double spending.

## Breeding, growth, and care

Keep separate: inherited genetics, expressed appearance, learned skills, habits/temperament, and life stage. Growth can change appearance while retaining identifying features. Skills may later help research; do not assume learned skills are genetically inherited.

Care should be forgiving when the player is busy. The Companion and app offer the same individual's history rather than independent pet copies. Aging, evolution triggers, lifespan, and absence behavior are unresolved. Death or punitive neglect is not an agreed feature.

## Social play

Social interaction is part of the product direction. The following are the current proposed rules, not a finished multiplayer specification:

| Interaction | Intended experience | Rule to resolve |
| --- | --- | --- |
| Scan a card or device | Meet a critter and inspect its ancestry | Which information is public and cached? |
| Breed together | Each contributes a parent; each receives a sibling egg | Consent, cost, cooldown, cancellation and offline support |
| Exchange samples | Bring different environmental discoveries to each other's lab | Gift versus trade, consumption, and authority |
| Companion visits | Short cooperative activities and shared memories | Transport, skill rewards and scope |
| Share lineage | Print a newborn and trace connections across friends | Profile privacy and cross-instance identity |

Scanning is an introduction, not ownership transfer or automatic breeding permission. A public QR should not act as a secret authorization token. Breeding permission and ownership transfer are separate explicit actions. Printed copies remain shareable even if permission changes.

## Scan incentives

Agreed direction: make scanning rewarding, potentially contributing exploration points, while keeping it balanced. Public scans still do not transfer ownership, grant breeding consent or directly set evolution state. Reward rules validate an eligible discovery rather than trusting points encoded on the card.

Proposals for a small balance experiment:

- Reward a first eligible specimen discovery; show what was learned in the dossier.
- Offer a modest extra discovery benefit for a previously unseen trait or family, with exact categories and values to be tested.
- Repeated scans remain useful for inspecting an updated record, but do not automatically repeat the original reward. Copies, screenshots and reprints resolve to the same discovery identity.
- Test diminishing returns and a limit on the contribution of scanning to progression, so scanning complements field sampling and lab research rather than replacing them. Do not tie the entire game to finding other players.
- Reject fabricated or ineligible records for shared rewards. Test self-scanning, alternate-account/creature farms and reciprocal farming; unique specimen IDs and caps alone are insufficient protection.
- Do not present scanning a remotely shared picture as proof of physical travel or an in-person encounter. Decide separately whether remote discovery deserves the same or different rewards.

Compare no-scan, occasional-scan, enthusiastic-collector and repeat/farming play patterns. Inspect progression speed, meaningful discoveries and whether the cap creates pressure to log in daily. Numerical rewards, reset windows, remote/self-scan eligibility and which progress track receives points remain open for design review. Record rejected duplicate credit without making the player feel punished for viewing a card again.

## First playtest questions

Can someone explain why an offspring looks related to its parents? Is one completed sample satisfying? Does the printed card prompt a real exchange? Is research enjoyable without field hardware? Can a returning player resume without feeling punished?
