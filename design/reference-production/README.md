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
Overview composition. Fixed information areas and console semantics are retained.
No facts, resources or specimen knowledge may be implied by decorative art.

Output is an **offline art proof**, not a native runtime export or deployed UI.
The first reviewed family contains 11 editable SVG masters and their RGBA exports.
Inspect the [native asset sheet](exports/sheet-native.png) and the
[1024×600 Overview proof](exports/overview-offline.png).

## Production command

The editable masters are `src/*.svg`, authored directly at their intended draw
dimensions. `build.cjs` reads them without modifying them, rasterizes with Sharp,
and composes the proof and sheets from the resulting PNGs. The manifest records
each source hash, native dimensions and alpha contract. Resource masters are
40×53; the restored sample capsule is80×80. The capsule remains a sample symbol,
not evidence that a sample has been retained.

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

The [reference comparison](reference-analysis.md) records the form reconstruction
and bounded visual review. The final offline proof also passed a focused UX check:
existing empty-Lab facts and illustration roles are retained, only Overview has
warm focus, whole quantities/progress are readable, and headings clear their frame
contours. These checks do not approve other states, runtime behavior or final art.

Run `node verify.cjs` to regenerate once and inspect actual exported files.
[Verification](verification.json) records source/export hashes, real RGBA silhouette
transparency, native dimensions, exact 3× nearest-neighbor enlargement and unchanged
files after regeneration. Pixel-level reproducibility is established on the recorded
tool versions, not promised across every font/raster library version.

The reference's exact type identity remains unresolved; this proof retains the
licensed bundled font. Before native integration, review actual composed dynamic
states and supply the renderer with these same masters and measured placements.
