# Companion experience overhaul

Owner-review design proposal, 30 September 2026. Requested by Paco: overhaul the full Companion using the tactile handheld thinking established for Probe. This is a coherent design contract and repair map, not an implemented redesign or approved new game mechanics. Authoritative public entry points remain `specs/experience.md`, `specs/devices.md`, `specs/gameplay.md` and `design/screen-design-standard.md`.

## Player outcome

Pick up the Companion, recognize what it is doing, move between gathering, carried contents and critter company, take a deliberate action, see its effect, and get back without losing context. The device should reward a glance and invite closer inspection. Useful explanation belongs beside the unfamiliar activity; repeated instructions must not occupy the main experience.

The connected journey is **Companion gathers → player reviews Send → Lab accepts and stores → expedition ends → Lab studies the sample and creates one saved critter → the same individual is available for later inspection/company → Caddy shows the same accepted records.** Sending and Lab acceptance are separate acts. Receipt delay does not undo stored supplies or revive the ended outing. Current Companions mode exposes the same saved resident, known facts and visits. Capture/training, traveller assignment and expressive living response remain incomplete; the original reference is not fully delivered.

## What the team inspected

UX, ergonomics and copy review inspected actual 450×600 native frames, the combined-device controls/rules and retained Gemini03/04/C18 references. Their initially different strip/carousel and action-entry suggestions were reconciled into the single contract below. Review applies to this design, not implemented native screens or physical hardware.

Existing reference: **450×600 portrait; four directions, Confirm, Back; one device/inventory; Probe, Cargo, Companions.** The board's touch capability does not authorize touch play. No extra controls, haptics, speaker, sensor, battery meter or connection claim is invented. Legacy122×250 Probe /368×448 Companion paragraphs are superseded by the combined profile in `specs/devices.md`.

## One Home, three live overviews

Keep all three names visible in a slim horizontal instrument rail: **Probe · Cargo · Companions**. Use a shared baseline, fixed label positions and a clear selected marker; do not draw three raised web buttons or a list menu. A carousel hiding the other modes is not recommended for these three known destinations. Existing mode order and clamped ends stay consistent.

Left/Right at the rail replaces the entire overview immediately: principal subject, live facts, state explanation and available actions. No confirmation is needed to see the new mode. A quiet blue marker retains the selected mode. Exactly one warm focus marker shows where input currently applies, moving from rail to actions on entry. A selected mode is not a pending action.

| Mode | Main subject / information hierarchy | Useful action and empty state |
| --- | --- | --- |
| Probe | Actual selected expedition and activity first; elapsed/sample activity distinctly labelled; next try with chance explained; carried supplies and room available below. Last try is clearly past, never a promised next award. | Choose an expedition when idle; View cargo when running. End expedition is secondary and has an explicit early-ending consequence. No route is silently started by changing mode. |
| Cargo | What is actually carried: consistent Data/Energy/Essence objects with whole counts, capacity, and supported sample entries separately. Outing/delivery status remains visible. | Review send opens the actual manifest. Empty cargo explains whether gathering is continuing, an outing ended, or no outing exists. Capacity pause points to a real recovery. |
| Companions | Current: the same saved individual, recognizable portrait, identity, known traits and visits; expressive response remains incomplete. | Implemented: inspect residents and use the existing simple saved visit on this device. No resident, feature unavailable and no selected traveller are different states. Never substitute a fake party/care meter for an unfinished feature. |

Use one quiet device identity, one mode rail and one task subject. Remove the present repeated brand/mode title/tab stack. A compass/terrarium can identify a destination but cannot stand in for actual gathering data or an owned critter. Keep active outing/transfer context when browsing other modes.

## Physical actions and returns

| Context | Directions | Confirm | Back |
| --- | --- | --- | --- |
| Home rail | Left/Right selects adjacent mode and instantly updates overview. Down enters its remembered valid action focus; Up/clamp gives a restrained boundary response. | Enters action focus only. This gesture cannot start, send or discard. | Stays at Home/current mode; no activity change. |
| Mode actions | Up/Down selects a visible supported action. Up from the first returns focus to the rail. Left/Right performs no hidden mode change. | Fresh gesture opens/acts on the focused item. Consequential actions open a review. | Returns focus to the same-mode rail. |
| Inspection/detail | Moves only among visible supported content/actions. A draft quantity control needs its own explicit mapping before implementation. | Opens the named detail/review. | Restores caller, object and valid focus. |
| Unsealed review | Up/Down selects the explicit choices; no mode change. Safe initial choice is Keep. | Fresh Confirm commits the named reviewed action; Keep cancels the draft. | Cancels only the draft and restores caller. |
| Sealed transfer / result | Safe navigation remains available; no hidden cancellation. | Only a real named destination or same-operation recovery. | Leaves the view without cancelling submitted work, then reaches the mode rail. |

The existing two-gesture selector entry is retained in the first prototype: one Confirm enters actions, a later fresh Confirm acts. Make the change of focus visible rather than burying the rule in help. Every reachable Cargo state must escape through Back, including zero cargo after acceptance while confirmation is delayed. A future one-gesture Home can be compared separately; it is not mixed into this contract.

Consume held inputs across changed availability, wake and screen readiness; require fresh activation against the visibly ready frame. Do not replay blocked presses. A harmless timer repaint must not invalidate a valid gesture when interaction meaning is unchanged. The host currently pauses while a portable control is held; this is current software behavior, not evidence of tactile quality on hardware.

## Complete screen/state map

| Screen/state | What the player sees and learns | Control / return / implementation boundary |
| --- | --- | --- |
| Ready | Choose an expedition. One complete sentence explains gathering for Lab work; expedition choices use their actual names. | Existing Companion route choice is preserved. Older Lab-planner text is an authority conflict to reconcile, not a reason to send the player to a nonexistent selector. |
| Expedition choice | Focus previews the actual supported activity. Different names do not imply different yields/sensors if current rules do not provide them. | Confirm deliberately starts; Back retains the prior overview. Do not add a silent randomized route. |
| Gathering | Selected expedition, gathering state, earned whole supplies, elapsed activity and Next try in… each have a clear role. A new award changes the relevant count and shows the actual result. | View cargo is safe. Ordinary mode/cargo browsing preserves running collection. An empty try is visibly a completed try, not an error. |
| Not enough room | Gathering paused. Not enough room for the next try. Show used/total and actual contents; some free space may remain because the next award needs reserved room. | Existing recovery is Send to Lab, ending the outing on acceptance. Proposed manual discard requires connected implementation and explicit class/quantity review. No automatic discard. |
| Cargo / sample inspection | Counts mean carried items; any sample is a separate identified object. Show actual collection origin/duration and available recorded facts, without revealing its hidden genome. | Known worth may be its permitted use or a new research opportunity. Do not invent sale price, rarity, traits or promised loot. The audited early-return0/1/1 haul is supplies-only. Completed surveys can return a supported sample; show one only when actually present. Wild capture remains unimplemented. |
| Unsealed Send review | Exact snapshot and destination; Gathering paused. Explain: Send stops gathering; Lab acceptance stores these items and ends this expedition. | Keep restores the previous outing state; gathering resumes only if it was running and capacity allows. Send requires a fresh deliberate Confirm. No transfer is triggered by entering Cargo. |
| Returning / awaiting Lab acceptance | Show the same sealed haul. Before arrival, show Sending to Lab or Waiting for the Lab; after arrival, show Accept at the Lab. Gathering has stopped. | No Cancel send or Continue gathering. Back navigates safely. Link loss retains the same operation. |
| Accepted; confirmation delayed | Supplies stored at Lab. Expedition ended. Separately: the Companion is waiting for delivery confirmation. Carried quantities are zero. | Do not offer View expedition as though it is active. Back/modes remain reachable; new outing follows existing receipt policy. No duplicate Send or Accept. |
| Accepted; confirmation complete | Delivery success and empty cargo are explicit; the next supported action is returning to Probe to choose another expedition. | No automatic restart, reroll or continuation of the ended route. Clear obsolete result/history association when a new outing starts. |
| End early | End expedition with exact consequence: unfinished activity does not award a completion sample. This is different from a successful return. | Safe cancel default; explicit reviewed End. Do not relabel existing instant ending as a completed expedition. Review is a proposed repair. |
| Link/save failure | Keep actual inventory and operation visible; say what failed and the supported next step. Saved-at-Lab remains distinct from delivery uncertainty. | Never claim a retry button exists if recovery is automatic or external. No blind repeated credit. Storage failure has an explicit stable unavailable state. |
| Companions; no residents | No critters at the Lab yet. Explain how creation at the Lab begins the collection. | No fake traveller or inaccessible training action. Modes remain usable. |
| Companions; current feature missing | Choosing travellers isn't part of this demo yet. Distinguish this from an empty collection. | Truthful interim state, not the completed overhaul outcome. |
| Proposed resident view | Individual art dominates; small identity and known-trait inspection support recognition. Previous/next selection is deliberate and retained. | This uses existing saved residents; it does not assign a party, transfer ownership or give care authority. Architecture must resolve projection/commands/offline behavior before coding. |
| Proposed visit / response | Bring the existing simple Spend time together action to the same individual. Show its actual saved visit result and a bounded expressive response with a still fallback. | No invented hunger, neglect, friendship XP, training, capture or ability gain. Offline read-only cached state must be labelled; no fabricated completed visit. Behavior/art choices require owner steering. |
| Wake / return | Restore current mode/object and actual running/pending state; show content before enabling commitment. | Waking never starts/sends/reveals. No unattended cosmetic animation is a claim of ongoing real sensor activity. |

## Visual grammar and explanations

Derive native art from approved C18 plus retained Gemini03/04, rather than starting a new palette. Graphite provides depth; slender defined blue edges establish structure; saturated blue Data, gold Energy and lime Essence identify resources. Warm accents identify the single focus. Color always has shape, position or text support. Red is actual failure/risk, not ordinary study costs or a harmless pause.

Allocate the principal field to the current activity, cargo objects or the individual. Make composition follow the task: Probe activity, Cargo inventory, resident portrait and transfer result are not copies of the same information card. No device drawn inside its own display. Keep one stable short hint near the bottom, with essential state above likely hand occlusion. Use the native canvas for readable ordinary sentences; do not shrink text to make an overloaded frame fit.

First-use explanation appears beside the relevant unfamiliar state; repeat use gets compact live labels. Example: **Each try may find supplies. The bar shows time until the next try.** Do not make the meter look like a guaranteed award. Send review gets its consequence sentence; routine Home does not repeat the entire protocol. No question navigation, invented slogans, academic headings or forced three-word fragments.

Feedback is a sequence: **focused target → received press → operation pending → actual result → retained state**. Focus is visible at rest; a press acknowledges input without claiming success. Award feedback points to the changed resource/count. Acceptance shows the actual stored result. Error names the failed operation and useful recovery. Repeated presses cannot duplicate spends/transfers. Motion supports the response and has a clear still equivalent; no new haptic/audio hardware is assumed.

## Delivery and polish gates

1. Repair the known lifecycle/orientation issues and obsolete whole-item terminology in a bounded engineering slice with architect/coder/independent review. Reproduce the owner's original Cargo state when available; two fresh-world paths did not establish a universal trap.
2. Produce native-size authored state proofs using one asset/type family: Ready, Gathering, near-capacity pause, Cargo, cancelled Send, sealed return, accepted delayed confirmation, receipt complete, unavailable feature and a clearly proposed resident view. Include long real labels and meaningful zero/maximum counts. Original references remain unchanged; screenshot overlays and compressed crop masters are not final assets.
3. UX/copy/game/art inspect the same actual exports only for their affected risks. Then connect the intended physical controls and walk the target transitions, including interrupted/restarted operation and retained focus. Static frame approval cannot substitute for this.
4. Observe human comprehension: identify current mode/activity, explain carried stock versus the next try, predict Send/Accept consequences, get out of empty accepted Cargo, and recognize the same resident. Observe reach/occlusion/readability on the later physical build. No invented timing, comfort, brightness or battery promise.

Stopping boundary for this pass: one integrated design packet with focused independent reviews and a concrete repair/proof order. It does not authorize purchases, PCB/enclosure commitments, new game subsystems, services or production deployment. Native implementation/art and the proposed resident interaction are still required before calling the experience overhauled.

## Remaining proof

This design adds no required controls or hardware parts. Real reach, hand occlusion,
outdoor readability, cap force, battery, brightness and heat remain unmeasured.
Use the current device/BSP and asset budgets for implementation; do not infer
dimensions or comfort from simulator CSS or concept renders. The recommended
rail/navigation is a proposal; extending Companions to actual saved-resident
inspection/visits needs its own architecture and owner-steered behavior/art proof.

## Final review record

Independent interaction and ergonomics reviewers passed this integrated packet. Copy review found two phase/consequence corrections, now incorporated: cancelling a review restores the prior activity state rather than promising to resume blocked gathering; Accept is offered only after arrival rather than while delivery is blocked. The copywriter inspected the corrected rows and returned PASS. Reviews apply to this design contract, not a native export or implemented behavior. No final native export has yet been produced or approved by this packet.

[Companion promise versus actual pixels](companion-promise/README.md) compares the newly supplied
marketing concept/studies with the native frames. Next acceptance must demonstrate
reference-quality reusable art, a truthful illustrated activity and recognizable
saved-individual experience; blue frames and matching counts alone are insufficient.
Missing capture/training/portable resident functions remain explicit gaps.
