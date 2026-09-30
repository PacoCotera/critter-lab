# Expedition map study

Issue [49](https://github.com/PacoCotera/critter-lab/issues/49): an authored static
design proposal for exploring on the Companion and receiving the result at the
Lab. The existing delivered Core V1 is unchanged. This packet is a paper/state
proof, not playable software, selected balance, real sensing or hardware evidence.

The player chooses an Essence resupply goal locally, follows a fictional map,
starts a finite gathering opportunity, investigates a trace, reaches a newly
revealed cache and explicitly collects a sealed sample. The Lab later shows the
received journey and accepted whole items. It knows nothing about the away
Companion until a record is actually received.

## Player walkthrough

All Companion exports are **450×600**. The six main states share the same outing.
The input and transition notes below are separate from the device artwork.

| State | Physical input and visible response | Recorded consequence |
| --- | --- | --- |
| [01 · At Camp](exports/01-map-seed-a.png) | Directions step along four-way paths. Confirm at Camp opens its opportunities; Back returns to modes. Camp already offers all three essential resource classes. | This staged example has one failed Data attempt, then one earned Data. The next Data preparation is 30%; two attempts remain. The selected goal is Essence resupply, not a live Lab request. |
| [02 · Moss bend](exports/02-brook-opportunity.png) | Walk to Moss bend, then Confirm to inspect the place. Up/Down chooses **Inspect trace** or **Start Essence gathering**. A fresh Confirm performs the selected action; Back restores the same map position. The close view shows the local markings. | Inspect trace records the fictional observation **“Sealed-container trace continues east”** and reveals the cache connector/location. It grants no resource or sample. Separately choosing Start Essence pauses Camp Data at 30% and activates the finite Moss bend source. |
| [03 · New east route](exports/03-active-map-lead.png) | Directions move along the newly visible branch. The cyan diamond is the current tile; the caption says travelling. Confirm opens a place only after reaching one, with no action or award on ordinary path tiles. | This staged later moment contains Data 1 / Energy 0 / Essence 1. Data preparation is paused at 30%, Energy has not started, Essence is active at 60% with two attempts left. Gathering continues while moving; movement did not change its source. |
| [04 · Cache](exports/04-sealed-sample.png) | Reach the revealed cache, Confirm to inspect it, then freshly Confirm **Collect sealed sample**, or choose Leave it here. | Collection checks the separate capsule capacity and retains capsule `cache-A-01`. The neutral art shows a sealed sample; no genome, traits or supported forms are disclosed. |
| [05 · Return review](exports/05-return-review.png) | Back to modes, choose Cargo, then open return review. **Keep exploring** is initially focused. Keep/Back restores the same location and preparation; Up then fresh Confirm selects Send. | Review freezes work. Sending seals the record and stops gathering; it transfers only whole supplies and the collected capsule. Preparation is retained local activity state, never an item to spend or unload. Illustrative stores: supplies 2/4, capsules 1/1. |
| [06 · Sent](exports/06-sent-awaiting-receipt.png) | Back returns to modes while the receipt remains pending. | The sealed result remains visible. This source expedition cannot resume. This frame is deliberately before Lab acceptance; the later Lab frames are after acceptance. |
| [08 · Lab received log](exports/08-lab-received-log.png) | Up/Down chooses a received record; Confirm opens its details; Back returns Home. | The selected record is received and accepted at the staged timestamp. Only its walked Camp → Moss bend → cache route and whole accepted contents appear; no avatar, active source or preparation exists in this view. |
| [09 · Lab detail](exports/09-lab-received-detail.png) | Back returns to the same received record. | The two recorded accomplishments are trace inspection and sealed capsule collection. The capsule leads to existing Lab research; reception does not decode it. |

![Companion route after discovering the trace](exports/03-active-map-lead.png)

![Lab received expedition detail](exports/09-lab-received-detail.png)

The sequence condenses intermediate inputs: Camp's opportunity chooser, the
post-trace result moment, selecting Start Essence after the trace, walking to the
cache, the modes/Cargo landing, and fresh Lab acceptance are described transitions,
not additional captured states. **02 → 03 combines three independent events:** a
fresh Inspect trace records the observation and opens the route; a separate Start
Essence changes the active source; elapsed time/chance work then earns one whole
Essence. The Essence award does not reveal the path. The packet does not claim interruption/retry,
offline receipt, capacity-full or exhausted-source screen coverage. These remain
implementation acceptance work after owner direction.

## Useful divergence, not a visit-all checklist

Compare [seed A](exports/01-map-seed-a.png) with
[seed B](exports/07-map-seed-b.png). Seed A puts Moss bend on a direct nearby branch
and Relay → Stone shelf on a longer resupply circuit. Seed B divides the route at
Camp into Relay or Stone shelf arms which meet at Moss bend. The same essential
resource classes remain accessible at Camp; the optional stations offer additional
finite opportunities. Waiting cannot renew an exhausted source. The new sample
route is discovered through inspection, independently of gathering awards.

These are **two authored topology fixtures with seeded terrain**, not a general
procedural-world generator. The paper generator draws orthogonal corridors; it
does not validate the extra movement edges created by adjacent path tiles. The
cache connector is omitted before trace inspection and added afterward. Seeds
1429 and 7183 also change terrain clusters and stream placement reproducibly.

Native translation keeps the reviewed topology but moves seed B's cache branch
one row upward, leaving a blank row beside the ordinary upper arm. This prevents
unmarked shortcuts under actual four-way tile movement. The original illustrated
reference remains preserved; runtime path legality and restart behavior require
the separate native acceptance evidence.

## Visual grammar and editable source

The shared resource family is the exact retained Gemini/C18 PNG family from
`../core-v1-art/exports/`, drawn at 1×. No resource masks, Pip art or originals were
changed. `manifest.json` records the actual bytes, dimensions and hashes used.
Graphite fields, blue stepped shoulders, existing Vera/Vera Bold typography and a
dark inset with restrained amber focus carry the approved C18/03/04/07 direction.

The new fictional map vocabulary is editable code: layered grass clusters, trees,
stone, water, legal path tiles and five place markers. It is a proposed map visual
language, not a new approved creature, real geography or sensor reading. Markers
identify places; nearby terrain does not certify resources or sample contents.
The cyan/white diamond marks current position; a warm outline marks the current
inspectable place; quiet cyan marks an inspected site; the amber base marks the
active gathering source. Collected-cache marking applies only to that resolved
action, not every opportunity or clue at a site.

`study.json` owns the two graphs and representative state values. `build.cjs`
owns the reusable map/frame/resource geometry and composition. Each generated
`source/*.svg` preserves editable geometry plus exact raster resource/text layers;
live copy is edited in the generator, not repainted into an original source asset.
The map raster footprint is **400×220**, placed within its stepped panel; local
Brook/Cache views are exact **2×** crops of that same authored map, with no hidden
route in the pre-inspection Brook view. Received maps stay 400×220 at 1× and contain
only the recorded walked route. No screen screenshot is used as a background.

The Companion preserves native resource footprints (37×48, 43×51, 47×49), explicit
earned counts and separate preparation tracks. Current-location labels have quiet
backing on busy terrain. Location resource-status ink ends around y463; focus
begins at y475 outside its inset. Footer ink ends by y573, clear of the inner frame.
Actual glyph bounds for all exports are in `manifest.json`. Reference/font/source
hashes and local crop/scale coordinates are recorded there too. Full-scale proof
sheets are [six Companion states](exports/companion-sequence.png),
[two seeds](exports/two-seeds.png) and [Lab records](exports/lab-received.png);
these sheets arrange the native screens without resampling them.

## Regenerate with the established toolchain

From the public checkout, using an existing Node/Sharp installation:

```sh
node design/expedition-map-study/build.cjs
```

Set `NODE_PATH` to an existing module directory if Sharp is supplied by a separate
runtime. This uses repository Vera font files and installs nothing. Fontconfig
may report unavailable writable cache directories;
the measured font exports still complete. `manifest.json` records every native
canvas and exported PNG/SVG hash. The normal validation is whole-frame inspection
at native size and one unchanged regeneration comparison, not native gameplay tests.

## Review boundary

Game and UX agreed the proposed state/control/knowledge contract before production.
The first actual inspection identified weak location illustrations and excessive
Lab protocol prose. The revised batch uses same-map local observations and the
received route as the visual subjects. Game design, experience/visual review and
the coordinator inspected the corrected actual native-size exports and passed
this bounded proposal on 30 September 2026. The resolved objections were trace
versus gathering causality, truthful received-map scope, the travelling-position
caption, visible preparation and safe action/footer spacing.

One unchanged regeneration comparison is recorded in `validation.json`. This
does not claim owner acceptance, playable fun, selected yields or complete
procedural-map/runtime behavior. Owner steering is required before dependent
mechanics are implemented.
