# Explore probe: evidence, normalization and genomic recipes

Status: proposed evidence contract. One fixed sensor set should support extensible genomic content through both measured and simulated experiences. Fictional events are explicitly permitted; sensor selection, timing and mappings remain open.

## Sample disclosure boundary — accepted correction

The Probe must not reveal what is inside a collected sample or infer its genetic traits, structure or supported forms. It may show expedition progress, collection activity, legitimate resources and field encounters, keeping those observations distinct from sample analysis. Sample contents are discovered through research at the Lab. The current prototype's repeated-bands structural clue violates this boundary and is not an approved design.

Progress must develop gradually through actual expedition state. The current two-step fixture is inadequate for this experience; interpolating a cosmetic bar over it does not meet the requirement. Exact pacing, input credit and resource-award rules still require a coherent domain contract.

## Discovery device — accepted experience direction

The Probe is the player's contact with the world. Its resting screen must show collection activity, expedition progress, collected evidence and resources, with encounters visibly distinct from ordinary collecting. A player should be able to see what has changed without repeatedly opening status pages. Sampling remains straightforward and does not demand constant attention.

Player language must reflect discovery rather than cargo transport. Use expedition rather than outing; avoid haul as an umbrella for Probe results. Distinguish observations, evidence, discoveries, samples and resources according to what the game actually records. Samples and supplies are part of an expedition, not its entire purpose.

The owner requests visible collected points as well as progress. Exact point meaning, resource relationships, progression rules and balance remain open; do not substitute supply counts for points or invent a reward counter in the renderer. Progress must come from the expedition state, and observed context must distinguish measured inputs from fictional events and simulator inputs. Show relevant changes rather than animation for its own sake. Refresh cadence and power behavior require physical e-ink validation.

The current native fixture exposes only two simulated collection steps, supplies and an optional encounter. It cannot establish real sensing, passive elapsed-time progression or a complete points economy. The next Probe design must distinguish what can be shown from this state from proposed mechanics.

## Straightforward sampling — accepted direction

Sampling must stay easy to operate with a limited sensor set. Fictional events can add variety without requiring another sensor for every phenomenon or genetic possibility. Do not turn sampling into manual sensor management, a complicated sequence of physical maneuvers or a requirement for constant attention. Preserve meaningful play in ordinary settings and standalone operation. Exact collection actions, event cadence, distribution and eligibility remain design choices.

The [sampling design proposal](../design/probe-sampling.md) explores how broad sensed context and generated variation can produce distinct research leads. Its defaults are not approved algorithms or final UI.

## Lab-selected expedition profiles — accepted direction

The Lab offers a set of expeditions for the player to choose before taking the Probe out. Profiles can define duration, difficulty, yields and predefined events or event pools. The selected expedition shapes random events on the Probe and how sensed evidence contributes to resource types or collection points. A small sensor set can therefore serve different fictional gathering objectives without changing hardware.

The profile provides a bounded context for variation, not necessarily a fixed event sequence. Actual measurements remain distinct from fictional interpretation and generated events. Choosing a profile does not predetermine a complete critter genome or bypass later research. Resource quantities and research evidence remain separate records.

Selection and comparison belong at the Lab; execution must preserve the Probe's straightforward standalone experience. Exact durations, difficulty meaning, event probabilities, yield ranges, eligibility, early return, pause/resume and expiry rules remain open. Difficulty does not yet authorize physical hazards, mandatory reflexes, sample loss or neglect penalties.

Proposed engineering consequence: carry the selected compatible profile/content version and sufficient execution data onto the Probe before departure; retain expedition/player identity and resolved events through interruption. Rules must fit the installed firmware's storage and capabilities. Profile packaging, scheduling, clock trust, validation and synchronization are not selected by this direction.

## Durable boundary

**Selected expedition + sensed conditions + optional phone context + generated world events + player decisions → sample → research → genetic possibilities.**

The same expedition can also yield resources for laboratory activities or critter feeding. Resource gathering and standalone operation without a phone are accepted direction. The genomic flow above is one use of the probe, not its entire collection loop.

The probe should not hard-code one score per gene, dimension or genomic layer. It records a reusable environmental description and a fictional expedition history. The lab interprets both with updateable content/rules. Every future probe-supported recipe can use fixed sensed features, generated events, player choices or combinations; it does not require a matching new physical sensor. A new real physical measurement still cannot be reconstructed from a sensor that never captured it, but that places no corresponding restriction on fictional events.

Simulated phenomena are first-class gameplay, not inferior substitutes or necessarily lower-value samples. Preserve source distinctions internally for reproducibility and honest findings; the player sees a cohesive fictional expedition, not hardware/debug labels. Do not present generated radiation or water events as real-world hazard forecasts.

Genomic layers are information roles, not independent meters to fill. A recipe can establish class/genome possibilities or expression conditions for a creation event. New lifetime effects require explicit rules and consent through the activity; a sample must not silently change a saved creature or its ancestry. New content may define new recipe targets without changing probe hardware. All mappings below are fictional design choices, not biological deductions from sensor values.

## Resource gathering alongside samples

The probe gathers both research samples and usable game resources. Keep their meanings distinct: a sample carries evidence for investigation and potential specimen creation; a resource is an inventory quantity that a defined lab or feeding activity can consume. An expedition may award either or both under explicit rules; evidence points are not automatically spendable food or materials.

Proposed resource categories are laboratory supplies and critter nourishment. Exact items, recipes, quantities, storage limits and gathering choices remain open. Nourishment should be compatible with the broad creature framework: resource properties can meet different nutritional or fantastic physiological needs, rather than assuming every critter eats animal-like food. Feeding rules consume expressed nutrition/affinity requirements and affect lifetime state; they do not redefine the genome around gathering jobs or silently rewrite inherited traits.

Sensed context, fictional encounters and player choices may influence resource availability through versioned game rules. No separate physical sensor is required for each resource. As an illustrative option, a humid/dim expedition could produce a research sample and a fictional nutrient resource; neither the item nor its yield is an approved design.

The probe should show collection results and retain them without a phone. Proposed transfer requirements: record the resource type/version, quantity and collection identity; preserve pending transfers across interruption; retries must not award the same collection twice. Consumption and inventory authority need architecture review, especially for quantities split across devices. Stable identifiers alone do not solve offline double spending. Capacity and transport remain open.

Standalone operation is required. Expedition profiles now define eligible events and can produce randomized encounters. Exact scheduling and where random choices are resolved remain open; the selected content must support standalone execution. The device must support its agreed collection loop without a live phone or server; local rules, prepared content and recovery behavior require subsequent firmware design and validation.

## Candidate fixed sensor envelope

Proposed feasibility baseline: temperature/RH, ambient brightness and low-power 3-axis acceleration form a small measured baseline. Pressure is a useful extra to compare; sound needs a distinct playable benefit and processing/privacy budget. Compare coarse spectral sensing only if it materially improves play beyond brightness. Defer magnetic sensing and gyroscope unless a specific experiment justifies them. Because software events can carry future content, no extra modality is required merely to cover a future gene name. This is a proposed test baseline, not a final parts list or bench-validated minimum.

| Modality | Useful evidence | Limits and candidate manufacturer references |
| --- | --- | --- |
| Temperature + relative humidity | Local thermal/moisture conditions and changes | Enclosure/body/charging heat and airflow affect readings. Bosch BME280 demonstrates combined T/RH/pressure; mounting guidance discusses environmental coupling/self-heating. A separate T/RH device remains an option. |
| Pressure | Ambient pressure and changes over time | Pressure alone does not distinguish elevation change from weather or identify an underwater environment. BME280 is a capability reference, not a depth sensor or selected module. |
| Light intensity + coarse spectral channels | Brightness, light composition and transitions | VEML7700 illustrates ambient-light sensing; ams AS7341 illustrates visible channels plus clear/NIR/flicker. Spectral measurements require optical calibration and window design; channels are not automatically calibrated lux, object color or chemical identification. The AS7341 page lists replacement products; component lifecycle must be reviewed before selection. |
| 3-axis acceleration | Activity, stillness, orientation relative to gravity and sampled vibration | ST LIS2DW12 supports low-power activity detection. Acceleration does not establish travel distance, location or real exploration. Defer gyroscope unless a specific motion interaction requires it. |
| Sound features | Relative sound level, event density and coarse temporal variation | ST MP34DT06J is obsolete/out of production: historical capability reference only, not a shortlist part. PDM microphones need clocking, decimation and feature computation; they are not another slow I2C reading. Persist only aggregated features, never audio or speech transcripts. An uncalibrated digital amplitude is not sound-pressure level; ST calibration discussion distinguishes dBFS and dB SPL. |
| Magnetic field — conditional candidate | Field magnitude and local variation | ST LIS2MDL measures a magnetic vector. Charging coils, magnets, current paths and metal placement require review. This is not ionizing radiation, general electromagnetic radiation or proof of a special location. Include only if clean usable distinctions justify the integration. |

Coarse spectral information adds physical distinctions that brightness alone cannot recover, but generated events may provide greater gameplay variety with fewer components. Compare their actual contribution now; neither future-proofing nor a gene label establishes that extra sensing fits the eventual power/cost budget. Published sensor consumption is not assembled-probe battery life.

Do not add a camera, GPS, gas identification, ionizing-radiation detector, water chemistry or a contact soil probe solely to represent future gene names. They measure different things and introduce separate product requirements. Fictional radiation/magic genes can be enabled through laboratory recipes using ordinary evidence; the game must not call that evidence a real radiation/magic measurement.

## Four input sources and their combination

| Source | Candidate content | What enters the sample |
| --- | --- | --- |
| Sensed | Light/climate/activity summaries, optionally sound/pressure if feasible | Observed features, valid duration and quality |
| Optional paired phone | Location; map-derived setting; time/place context; optionally service-provided weather | Source, observation time, accuracy/freshness and derivation version; unavailable context remains absent |
| Generated | Radiation storm, ghost fluctuation, fictional water surge | Event type/version/seed, intensity/stage and resolved outcome |
| Player decisions | Stay/escape, shield/collect, investigate/leave | Choice, sample/instrument state when chosen and committed consequence |

A hybrid event can use sensed context to weight its occurrence or parameters, while remaining explicitly fictional. Dim conditions might favor a ghost-fluctuation encounter; a water event need not require water exposure or inferred weather. Do not require a rare geography or sensor reading to make a whole genetic family reachable. Event availability, measured influence and console alternatives need balance work.

### Optional phone context

Optional paired-phone context can supplement the device experience. It must remain supporting access, not required routine interaction or assumed continuous location tracking.

Proposed interaction allocation: the probe presents collection progress, encounters and in-expedition choices; the lab presents expedition selection, sample research and specimen creation; the companion presents everyday critter interaction. The phone may supply permitted context, connectivity and synchronization in the background. Pairing, OS permissions and occasional maintenance may need a phone screen, but routine collection, encounter decisions, research results and critter interaction must not repeatedly send the player to it. Exact device screens and flows remain for UX review.

Design graceful absence: if the phone is disconnected, asleep or unable to provide context, the device continues its supported local activity using retained evidence and available simulated content. Do not require opening the app to resolve an encounter. Background assistance is a product intent, not a promise of uninterrupted mobile-OS execution; firmware/app review must establish actual offline behavior and recovery.

| Candidate context | Proposed gameplay use | Interpretation boundary |
| --- | --- | --- |
| Approximate location/region | Regional encounter variation and sample origin | Retain accuracy and freshness; a phone location is not automatically the probe's current location. |
| Location plus map data | Coastal, urban, park or other mapped setting can weight encounters | Coordinates alone do not identify habitat; dataset coverage/version and uncertainty matter. |
| Time and location | Local day/night or seasonal context | Computed context does not replace measured brightness or temperature. |
| Location plus a weather service | Regional weather can influence expedition events | Remote observations/forecasts are distinct from local probe measurements; service or cache availability must be explicit. |
| Valid location changes during an expedition | Variety of explored areas | Sampling noise must not count as travel; distance alone should not determine sample value. |

For example, mapped coastal context plus measured dim/humid conditions could favor a fictional tidal encounter. The player's choice adds an event signature; laboratory research interprets the combined evidence into candidate affinities or abilities. Location weights possibilities rather than directly becoming a water gene. These mappings remain proposed balancing choices.

Start with optional, explicitly enabled expedition context and support approximate location. Precise/approximate and foreground/background access need platform-specific validation. The phone app's background delivery, transport synchronization and energy use require device testing; pairing alone does not guarantee fresh context.

Associate phone evidence with the correct sample and observation window, recording source, time uncertainty, accuracy and derivation version. A stored pairing is insufficient evidence of co-location. If live association is unavailable, attach later context only when its recorded timing and association support it; do not copy the phone's upload-time location onto earlier collection. Stale or unavailable context does not invalidate the probe's own evidence. Reconnection must not duplicate observations or rewards.

Proposed retention default: coarse setting/region tags sufficient for recipes, with no precise route embedded in a public specimen, genome image or QR. Exact location retention, if a later feature needs it, requires a separate product decision. Standalone collection continues without phone permissions, connectivity or map/weather services. Phone context can diversify routes and encounters; whole genetic families still need accessible laboratory or simulated alternatives.

Hardware implication: compare optional phone positioning before considering onboard GPS. This can broaden context without another probe sensor, but does not establish an assembled power saving until radio/app behavior is measured. Standalone and optional-phone behavior remain unvalidated on hardware.

### Worked probe-to-lab event sketch

This is a deferred optional encounter example, not the basic sampling loop, final UX or an approved loss rule. It must not complicate ordinary collection. While a sample forms, a fictional water surge approaches. **Escape** could preserve the current sample and conclude the encounter; **stay** could add a water-event signature with a bounded chance of sample instability. Escape/stay is an in-game decision, not an instruction to move physically or expose the device to water. The outcome records the event and choice once. The lab later studies that signature and may reveal relevant affinity, structure or regulatory possibilities within a valid founder genome. Neither choice needs a physical water sensor.

For a ghost fluctuation, an investigate/leave choice could change the evidence collected; for a radiation storm, shielding versus exposure could influence candidate mutation research. No automatic death, destruction, positive mutation or real hazardous activity follows from those examples. Define stakes at sample/expedition level first; this collection phase need not contain a live critter to damage.

The reference e-ink/two-control concept suggests discrete stages and deliberate choices; it does not freeze hardware. Hardware review requires any timed decision to begin only when the choice page is visibly ready, not merely when the display transfer ends. Actions must correspond to visible labels; repeated/held wake input must not accidentally select an outcome. Save and restore pending choices across sleep/reboot, and keep collection/local receipt independent of encounter rendering or transport connectivity. Exact refresh, signaling and timeouts need hardware/UX testing. Proposed default for an unanswered encounter is to preserve progress rather than demand real-time reflexes; it remains unselected.

A shared rule event should describe prerequisites, sensed context (if any), generated inputs, available choices, costs/stakes, outcome and research/resource effects. The probe runs the expedition; the lab interprets samples in more detail. The Lab selects the expedition; where its execution content is prepared and how random events are scheduled require firmware and service design. The agreed standalone collection loop must execute without a live phone/server, within measured memory and power limits.

New encounter packs should use supported conditions, presentation primitives and bounded data sizes. More elaborate genomic interpretation can remain in the lab; the probe need not execute the entire future genetics framework. Content versions declare compatibility with the installed interpreter. New semantics may require a software update even when hardware remains fixed; arbitrary future processing/storage requirements cannot be promised on an unspecified MCU.

## Evidence contract before scoring

Preserve physical-unit observations or calibrated feature summaries, not just points or labels such as forest/fire/ghost. Each sample should carry:

- Stable sample/event identity, origin, capture sequence and elapsed observation time; timing uncertainty if wall-clock time is unavailable.
- Channel capabilities, sensor/feature versions, units, calibration/configuration reference and validity flags.
- Aligned observation windows: slow environmental readings plus activity/sound summaries and their valid durations. Record co-occurrence at the window resolution; separate marginal totals cannot establish that humid and dark conditions occurred together.
- Distributions or bounded summaries, trends and transitions where explicitly captured; quality/coverage, missing/saturated readings and charging state.
- Collection/scoring versions and retained evidence. Raw audio is excluded; do not assume future algorithms can recover sound details discarded during summarization.
- Generated event IDs, seeds/rule versions, choices and resolved consequences as a separate source type. Preserve enough state to resume encounters without replaying rewards or silently changing outcomes after an update.
- Optional phone context with its own source, capture time, accuracy/freshness, sample association and map/service derivation version. Keep remote or derived context distinguishable from local measurements.

Window duration, cadence, retention and quantization require measured hardware/firmware validation. Recipes can only require features the frozen contract actually supplies. If later firmware adds a derived feature using the same sensors, old samples without it remain unsupported for that requirement rather than silently receiving invented data. A rules/content update alone should suffice for recipes composed from existing features.

## Normalization and points

Do not collapse everything into a single sum. Keep **sample progress**, **environmental profile**, **event history**, and **recipe-specific research evidence** distinguishable. The number of recipe matches does not mint extra physical samples or bypass sample consumption. Simulated events use their own bounded, versioned outcome rules; the physical normalization below applies only to sensed features. A combined recipe can require or weight both sources without pretending event intensity is lux or temperature.

1. **Calibrate and validate.** Convert to canonical units; flag saturation, invalid or missing data. Pause collection scoring during charging under the existing hardware direction. Bench evidence determines any post-charge settling time. Suspected pocket/body effects are not perfectly detectable; do not claim automatic certainty.
2. **Extract stable features.** Temperature/RH/pressure remain in physical units; light can use a logarithmic transform; spectral ratios require gain/exposure correction and enough light; motion uses gravity-separated activity features where validated; sound features retain their stated reference. Low-light spectral noise is unknown, not a strong color signal.
3. **Evaluate recipe memberships.** A versioned curve maps a feature into a 0–1 degree of matching: cool, humid, dim, variable, sustained, etc. These labels are game vocabulary with explicit bounds, not universal biological categories. Use shared calibrated bounds; arbitrary per-session min/max scaling would make identical environments incomparable.
4. **Require the combination.** Mandatory conditions must overlap in the same defined observation window. One proposed conservative combination is the minimum membership among required features. A missing required feature makes the window ineligible, not zero-temperature/zero-sound evidence. Optional inputs cannot compensate for missing mandatory evidence.
5. **Integrate valid time.** Accumulate matching duration at a bounded recipe rate with quality weighting and a cap/repetition policy. Extreme readings, shaking or louder sounds do not earn unlimited rewards. Threshold persistence/hysteresis should suppress noise near boundaries.

An illustrative formula for one recipe/window is:

`earned = min(remaining cap, rate × valid minutes × match × quality × repeat factor)`

`match`, `quality` and `repeat factor` each lie in [0,1]. Rate is points per minute. Valid minutes already exclude missing intervals; do not multiply the same missing-time fraction again as quality. Quality addresses other defined reliability criteria. Formula, bounds, target and repetition rules are proposals, not calibrated balance or anti-cheat proof.

### Worked normalization example

Illustrative cool/humid/dim recipe; no selected component or required real excursion. Define `clamp(x)` between 0 and 1:

- Cool membership: `1 - clamp((temperature_C - 10) / 20)`.
- Humid membership: `clamp((RH_percent - 40) / 40)`.
- Dim membership: `1 - clamp(log10(1 + lux) / log10(1001))`.

For a constant valid observation window at 18 C, 70% RH and 9 lux, memberships are 0.60, 0.75 and approximately 0.667. The minimum is 0.60. At an illustrative 2 points/minute over 5 valid minutes, quality=1 and repeat factor=1, the recipe earns **6 points**, assuming at least 6 points remain below its cap.

This does not mean 6 humidity genes or +6 health. It is evidence/progress toward the particular research recipe. If humidity is missing, this recipe earns no points from the window and records insufficient evidence. If the same record is replayed, its stable observation identity prevents credit being added a second time by the local authority. Cryptographic provenance and shared progression trust remain separate design work.

## From evidence to new genomic content

Each recipe declares required evidence/features, optional context, applicability, progress thresholds, supported genetic possibilities, permitted research choices, resource consumption and version. Proposed probability weights may establish research possibilities, not secretly replace the fully unlocked selected genome at creation. See the [creation contract](sample-to-critter-contract.md).

| Example recipe direction — not approved mechanics | Potential targets |
| --- | --- |
| Sustained humid/dim conditions | Moisture-compatible variants, appearance variation or low-light sensory potential |
| Repeated light transitions plus valid motion patterns | Candidate regulation, signaling or movement-related variants |
| A future phase/teleportation research recipe using an existing combination of light, pressure variation and activity | Optional fantastic physiology/ability prerequisites, explicitly a fictional mapping |

One observation can inform several candidate recipes; a research/creation event must still specify which result is pursued and how sample resources are used. Reusing environmental vocabulary does not require every resulting creature to have the same phenotype. Preserve uncertainty and bounded player choices from the proposed founder workflow.

For each future dimension or genuinely new layer, content review must provide at least one meaningful route through sensed features, generated encounters, decisions or laboratory research. New layers cannot bypass identity, lineage, compatibility or permission rules. Extension means adding declared interpretations or event content; no dedicated physical sensor per dimension is needed.

Keep old samples' evidence and original scoring version. A new research recipe may examine retained evidence under a newly recorded interpretation, subject to lifecycle rules; it must not retroactively overwrite points, generate a second spendable sample or reroll an existing specimen. An unsupported required feature remains unsupported. Console-only research remains available with honest lab/simulated provenance.

## Evidence needed before freezing the probe

- Compare worn, held, bag/pocket, shade, direct light and representative enclosure conditions; measure heat/airflow bias and settling after charging.
- Check optical window, angle, gain changes, dark noise, clipping and whether retained spectral summaries distinguish useful situations consistently.
- Measure audio/motion false events from wind, handling and device operations; check calibration and privacy of stored features.
- Evaluate pressure interpretation and magnetic interference with caddy, coil, battery/current paths and enclosure hardware if those channels are considered.
- Measure total energy/storage cost at proposed cadences, including computation, radio, display and microphone; no runtime estimate is validated yet.
- Agree exact enclosure envelope/mass and battery/runtime target. Audit I2C addresses/voltages, interrupt pins, PDM resources and worst-case power peaks on the actual board. Calculate storage as window bytes × retained windows plus events, metadata and atomic-write reserve; raw retention is not unlimited.
- Replay sample evidence through recipes: unit consistency, same-window requirements, missing channels, caps, timestamp gaps and idempotent credit. Compare varied and repetitive collection without assuming sensor authenticity proves real exploration.

Genetic evidence semantics and measured sensor/firmware feasibility must agree before hardware selection. No numeric thresholds or parts are selected by this draft.
