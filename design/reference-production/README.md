# Reference-to-production art workbench

An isolated art-production exercise for the approved C18 Lab direction. This is
part of the product repository; it does not replace the live renderer or create a
second visual authority. Editable sources, deterministic exports and native-size
proofs belong here. Role coordination and skills stay in the private repository.

Reference: [approved C18](../game-art-proposals/35-vault-composition/18-c-refined.png)
and its [clean inspection crop](../game-art-proposals/37-lab-extracted-kit/screen-reference.png).
The [screen standard](../screen-design-standard.md) governs visual meaning.

First exercise: frame/header components, quiet and focused navigation treatment,
Data card / Energy crystal / Essence resource family, applied in one 1024×600
Overview composition. Lab-wide information and console semantics are retained;
the rejected text-heavy card layout is replaced by four illustrated status anchors.
No facts, resources or specimen knowledge may be implied by decorative art.

Output is an **offline art proof**, not a native runtime export or deployed UI.
The current family contains 15 editable SVG masters and their RGBA exports;
12 are used in the proof and three useful prior variants are retained separately.
Inspect the [native asset sheet](exports/sheet-native.png) and the
[1024×600 Overview proof](exports/overview-offline.png).

## Production command

The editable masters are `src/*.svg`, authored directly at their intended draw
dimensions. `build.cjs` reads them without modifying them, rasterizes with Sharp,
and composes the proof and sheets from the resulting PNGs. The manifest records
each source/export hash, native dimensions, alpha contract and occupied bounds
measured at half alpha. Current resource masters are 56×68. The four topic masters
are 136×144; their visible silhouettes have different widths to balance unlike
forms, rather than assuming equal canvases establish equal optical weight. The
topic illustrations identify destinations, not owned items or current activity.
The retained 80×80 capsule and two prior status frames appear separately on the
sheet; this composition does not consume those earlier variants.

| Placement | Native footprint / anchor |
| --- | --- |
| Screen perimeter and inter-panel gutter | 24px outer inset; 16px gutter |
| Header | 976×100 at 24,24; resource starts 404/602/800,40 |
| Navigation | 208×416 at 24,140; 184×60 row masters |
| Shared read-only field | 752×416 at 248,140; 24px title/field inset |
| Explore / Research art | 136×144 at 272,213 / 642,213 |
| Incubator / Habitat art | 136×144 at 272,377 / 642,377 |
| Topic text | x422 / x790; shared 23px heading tier |

The measured visible topic bounds are 114×123 (Explore), 119×127 (Research),
104×125 (Incubator), and 120×125 (Habitat). Header resource bounds are 45×62 (Data),
48×63 (Energy), and 49×54 (Essence). They are authored at those sizes; no bitmap
rescaling is used in the proof. The next-unit line is subordinate 14px type with
its nominal baseline at 104, clear of the header's bottom contour.

With Node.js and Sharp available, run from this directory:

```sh
node build.cjs
```

No package installation is part of this exercise. Text loads the repository's
licensed `native/shared/fonts/Vera.ttf` and `VeraBd.ttf` explicitly. This preserves
the existing font family; the reference's exact type identity remains unresolved.
Some hosts report an unwritable fontconfig cache; rendering still uses the explicit
font files. The art PNGs themselves contain no type or live game facts.

Inspect `exports/sheet-native.png` at1×, `exports/sheet-3x.png` at its labeled
nearest-neighbor3×, and `exports/overview-offline.png` at1024×600. This proof retains
five navigation choices, four read-only status regions and whole supply counts;
only the Overview row is focused. It is an offline composition, not runtime or
physical-display evidence. No screenshot matting or large-image downsampling is
used to construct the masters.

## Review and technical evidence

The first proof's composition acceptance was withdrawn after the owner identified
poor margins, icon proportions and excessive text. The [reference comparison](reference-analysis.md)
records the reworked composition. The current proof has larger topic illustrations,
open status regions, consistent insets and concise adjacent facts. Focused art and
UX inspections assessed this actual revision, including empty-state truth and
one warm navigation focus. Proof SHA-256:
`98151907ef0dc81281c09be18dbaeb76338b4c69db615337865ac8305561ed23`.
These checks do not establish owner approval, other states or runtime behavior.

Run `node verify.cjs` to regenerate once and inspect actual exported files.
[Verification](verification.json) records source/export hashes, real RGBA silhouette
transparency, native dimensions, exact 3× nearest-neighbor enlargement and unchanged
files after regeneration. Pixel-level reproducibility is established on the recorded
tool versions, not promised across every font/raster library version.

The reference's exact type identity remains unresolved; this proof retains the
licensed bundled font. Before native integration, review actual composed dynamic
states and supply the renderer with these same masters and measured placements.
