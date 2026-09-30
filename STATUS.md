# Product status

Current outcome: Companion gathering → Cargo return → Lab acceptance → research
and creation in the same durable local world. The current family remains a
combined Companion, home Lab and shared printer/summary Caddy.
[Selected sage/stone appearance](design/lab-controls/combined-family-materials.png)
is concept art, not a physical prototype.

## Delivered software boundary

The [three-device host simulator](native/selected-lab/README.md) presents native
Lab, Companion and Dock contexts together. Companion modes preview Probe, Cargo
and Companions before entering an action; activation requires a separate fresh
input. Whole earned supplies and next-attempt activity remain visibly distinct.
Sending seals a haul and stops gathering. Arrival opens Lab reception; fresh
Accept credits once and ends that outing. Receipt confirms transport metadata;
a new outing has a new identity. [Gameplay](specs/gameplay.md#expedition-continuity-and-return--accepted)
and [architecture](specs/architecture.md#three-device-host-simulator) own those rules and recovery limits.

The local [Pip loop](native/selected-lab/V1.md) continues through five paid
research findings, explicit complete-genome selection, incubation, deliberate
reveal and habitat visits. Saves retain supplies, discoveries, individual
identity and transfer state across restart. The browser presents native pixels
and the accepted physical-control panel; screen artwork is not clickable.
The sandbox reset is outside device shells and preserves the matching saved
world and sidecars in a recoverable backup.

The validated source checkpoint is `d097a1ee8027cfbe4501c08bb47a01cc942b25b8`:
three native CTest suites, HTTP presenter checks and the full timed
research/incubation/restart journey passed. [CI run 36673219813](https://github.com/PacoCotera/critter-lab/actions/runs/36673219813)
passed for that revision. The [live sandbox](https://critterlab.basicberry.com)
reports its running release revision and activation time; source validation alone
does not establish that live activation. Only the existing Git/CI release path
publishes accepted source.

Inspect the [actual native gallery](design/connected-device-review/native/README.md)
for the current functional presentation. The [Gemini Probe03, Cargo04 and reception07 references](design/companion-connected-art/README.md)
are reviewed concepts, not native implementation. The owner rejected earlier
native visual fidelity as final; behavioral checks do not approve art quality.

## Remaining proof

The next named screen outcome is faithful shared-asset derivation from the
approved C18 and connected Gemini concepts, followed by native-size inspection.
The retained Home hierarchy continues. Research still needs its collection-wide
Overview separated from each sample workbench; its current five-study Pip
fixture is not a complete research content system. [Experience](specs/experience.md)
and the [screen standard](design/screen-design-standard.md) own these requirements.

The Lab reference is Raspberry Pi4; current execution is Linux x86-64 C17 host
simulation. One process owns three logical devices and simulated radio exchange.
ARM builds, independent endpoint storage, radio, display/input drivers, printer,
charging, power and physical performance remain unverified. Companions marks
party assignment as unavailable; capture/training, distinct authored route
events, cloud services and habitat ecology remain outside this slice. Timings,
chance, capacities and content limits remain provisional in the play guide.

## Product development gate

[Electronics reference](specs/devices.md#electronics-first-v1-reference-specification)
→ integrated software proof → human playtest → hardware or mobile decision.
The caddy development display is the 5.79-inch monochrome module, 792×272; earlier
3.7-inch and separate-Probe depictions are historical. Bench evidence and owner
selection precede dependent PCB/enclosure work or purchases.

Standalone core play and optional global Cloud Pass operations are desired
capabilities. Their authority, reconciliation and entitlement contracts remain
open under [architecture](specs/architecture.md) and [cloud/local records](specs/cloud-sync.md).
No complete V1, physical kit, human-playtest acceptance, production BOM or hardware
feasibility is claimed. [Build coverage](BUILD.md) identifies available sources.
