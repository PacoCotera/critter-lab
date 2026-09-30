# Historical connected Lab and Probe visual review

Current implemented screen evidence is in the [native three-device gallery](native/README.md). The terminology, fractional cargo ledger and former controls below belong to this rejected historical study; current rules are in [gameplay](../../specs/gameplay.md) and [experience](../../specs/experience.md).

**Visual treatment rejected.** The owner rejected these screens as a poor translation of the selected Playful Pixel Lab direction. This supersedes earlier assent. Retain the packet only as behavioral/state evidence; its layouts, typography and artwork are not approved implementation targets. No new UI should be built from these plates.

## Whole journey

Weather research at the Lab → mixed resource gathering → inspect observations → free cargo space → inspect and explicitly accept optional exposure → return two ready packs → spend one Filament pack to discover the second Crown allele. This trip finds no capsule and does not finish decoding a genome or create a critter.

The Lab remains a broad instrument workbench with small persistent identity. The Probe uses three resource vessels and compact monochrome views. The visual vocabulary comes from [refinement 02](../visual-language/refinement-02/README.md); vessel/instrument character comes from the [original Probe reference](../screens/sampler-studies.png). That old reference's points, biological clues and incubation claims are superseded and are not imported. Earlier [expedition frames](../probe-expedition/README.md) retain behavioral evidence, not approved target layouts.

![Lab preparation, receipt and research](lab-board.png)

![Probe gathering, observations and choices](probe-board.png)

## Walkthrough and physical input

Screen labels are focus targets operated with the depicted device controls, not touch buttons. Lab: rotate, Confirm, Back. Probe: Next, Confirm. Unassigned keys and knob press have no action. Changing focus never commits a choice. The proposed focus order below is part of this review.

| Moment | Input → screen response | Consequence / return |
| --- | --- | --- |
| Lab expedition plan | Rotate the profile selector between eligible profiles (General survey / Weather research in this study); Confirm selects Weather research; Back restores Lab caller | Profile changes event opportunities, not baseline resource rates. No research stock spent |
| Probe hold | Next cycles Cargo → Observations → Event (only when waiting) → Return to Lab; Confirm opens the focused destination | Three vessels show unfinished fraction; ready packs and shared used units are separate. Full hold pauses gathering, not all observation |
| Observations | Next selects Log or Return; Confirm opens it | Broad conditions, valid observed duration and availability are read-only. Return restores gathering focus. Multiple channels never multiply time |
| Log | Next cycles entry navigation and Return; Confirm activates the target | Observation, game event, player choice and game result stay distinct. Browsing never draws an event or accepts risk |
| Discard preview | Reach Data through hold management; Keep is initial focus. Next then Confirm can discard one ready Data pack | Exact loss is one pack and exactly one unit is freed. The 40% unfinished Data fraction remains. Keep changes nothing |
| Optional storm | Inspect after freeing space; safe Shelter is initial focus. Explicit Next/Confirm accepts the displayed bounded exposure | No penalty for ignoring the event. Fictional shock can damage the Probe; earned cargo is retained. This is not a real weather alert |
| Exposure result | Safe result returns to a usable hold; return path retains cargo | The displayed safe outcome is one branch, not guaranteed. No automatic repeat exposure |
| Lab receipt | Confirm starts the existing receipt flow; saved acknowledgement appears only after ownership is confirmed | Energy 1 and Filament 1 enter Lab stock once. Partial packs remain on Probe. Back does not erase a pending receipt |
| Lab Crown research | Rotate to Crown and inspect; commitment must show Filament cost 1 and stock 1 before Confirm. Result shows retained C/c knowledge | One Filament spent once; Lab retains Energy 1. Other loci stay unknown; the full genome remains undecoded. Back returns to collection |

A fresh gesture is required after a frame becomes visible; input from an earlier frame cannot accept risk or discard. These are interaction intentions, not implemented refresh behavior.

## One consistent cargo ledger

Values below are pack-equivalent units, in Data / Energy / Filament order. One complete pack occupies one unit; fractions share the same four-unit hold.

| Point in this example | Probe | Lab |
| --- | --- | --- |
| Full hold | 1.40 / 0.80 / 1.80 | 0 / 0 / 0 |
| Discard one Data pack | 0.40 / 0.80 / 1.80 | 0 / 0 / 0 |
| Safe bounded exposure ends | 0.47 / 1.64 / 1.89 | 0 / 0 / 0 |
| Whole-pack receipt acknowledged | 0.47 / 0.64 / 0.89 | 0 / 1 / 1 |
| Crown study saved | same retained fractions | 0 / 1 / 0 |

This uses the [executable expedition arithmetic](../../prototype/expedition/example.md). It is a seeded visual scenario, not a replay of the short [sensor comparison](../../prototype/expedition/sensor-example.md). Observation coverage shown in a frame does not establish how long filling the hold takes. Time, rates and risk remain illustrative rather than final balance.

## Review scope and missing coverage

Judge the whole visual hierarchy, resource-versus-observation distinction, physical focus and the return-to-research connection. The boards are scaled for comparison; native exports use Lab 1024×600 and Probe 122×250. Screen proportions are study inputs, not selected hardware. Neither scaling nor static inspection establishes physical readability or e-ink performance.

The supplementary states cover study commitment/pending, uncertain receipt recovery and damage/service. Capsule discovery, incubation, Companion life, all inventory variants, full sensor-unavailable presentation and every loading/failure state remain outside this slice. These additions do not make the packet a complete game screen specification.

## Commitment and recovery in the same journey

![Study commitment, pending and retained finding](study-board.png)

![Uncertain receipt, retained damaged cargo and service](recovery-board.png)

The damage branch is an alternative to the safe branch above. It retains Data 0.435 / Energy 1.220 / Filament 1.845 (3.5 units). Confirmed offload moves Energy 1 and Filament 1 to Lab; Probe retains 0.435 / 0.220 / 0.845 (1.5 units). Free service restores operation after offload; it does not replay the ended encounter or alter research stock.

| State | Focus and input | Retained truth |
| --- | --- | --- |
| Study confirmation | Safe Keep/Not now initially; rotate to Start study, Confirm commits | Cost 1 Filament, available 1; Crown C plus unknown copy. Focus/viewing spends nothing |
| Study pending | Return to workbench is usable; Back leaves the view | Filament 1 is visibly committed and unavailable to a second spend while pending; leaving does not cancel a submitted save. Reveal C/c only after saved acknowledgement |
| Receipt uncertain | Check receipt initially focused; Confirm reconciles the same receipt; Back returns | Incoming Energy 1 / Filament 1 are not spendable yet. Do not request a new award or discard retained cargo |
| Probe damaged | Return to Lab initially focused; Next can select Review hold | Gathering stops, cargo stays. Returning does not itself credit the Lab |
| Lab service | After confirmed receipt, Service restores operation; Back leaves damaged | No research resources charged, no automatic study and no replay of the storm |

## Deferred simulator handoff — not ready

After a new visual review is accepted, implementation may connect the reviewed screens and these state transitions to the existing expedition/observation reducers and persistence. The browser entry currently serves older separate experiments; a focused architecture check must choose the smallest adapter into that existing host. Keep game state authoritative in those reducers, not in screen handlers. No framework replacement, new service, telemetry platform or rules rewrite is implied.

Acceptance for that next slice: perform the resupply-to-Crown journey using depicted device controls, reopen the same save, reconcile the same uncertain receipt, prevent repeat study spending, preserve cargo through damage, and show simulated Observations without granting credit on inspection. Use relevant behavior checks plus one actual browser walkthrough; current static evidence does not establish those results.

Validation: the original nine native exports and all five supplementary states were inspected; one independent UX/domain review checked the ledger, control reachability, disclosure and readable bounds. Review copy fixes clarify conditional storm risk, receipt and Crown language, plus exact destination order. Supplement review corrected the pending-stock label so committed Filament is not shown as spendable. A deterministic regeneration check passed for each bounded export; scaled boards are presentation aids. Arial is a local prototype font, not a bundled device selection. The crown diagram is a conceptual local finding, not canonical creature art. Original references and useful unfinished work remain intact; generated art here is proposed screen geometry, not a canonical critter asset.
