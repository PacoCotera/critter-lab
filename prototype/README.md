# First playable experiment

A reversible local software experiment. UI, creature artwork and fictional genetics are provisional, not approved product designs. No hardware choice is implied.

The separate [pixel lab fixture](lab/README.md) runs at `/lab/`. It explores research and a parentless founder with independent persistence, leaving this older breeding/share experiment intact. It is a tested technical baseline, not the newer homecoming flow or accepted product polish.

Requires Node.js 22 or newer and npm; no build step. QR generation uses the locked `qrcode` package; tests independently decode its output with `jsqr`.

```sh
npm ci
npm start
# Open http://127.0.0.1:4173
npm test
```

Equivalent direct commands: `node prototype/server.mjs` and `node --test prototype/tests/*.test.mjs` from the repo root. Use `CRITTER_PORT` to select another port if needed. The server listens only on this computer and serves an explicit allowlist of prototype files, not the repository.

## Try it

1. Inspect the two starters and baseline forecast.
2. Complete the labeled research fixture; inspect the saved finding and computed comparison, then toggle Mist Thread for the next incubation. The finding remains available after sample consumption and reload, using the saved incubation's parent snapshots for its historical comparison. Its evidence is explicitly a developer fixture with no physical observations.
3. Begin incubation. The sample is consumed when that event is saved.
4. Use the labeled developer hatch action; inspect the offspring and parentage.
5. Reload to confirm the same individual remains. Choose **Share / print individual card**, open its dossier, and verify the same individual ID, family, expressed traits and carried traits. Compare saved parent/offspring variants in the ancestry section; parental appearance remains explicitly unknown. The dossier cannot add a pet to your collection.
6. **Preview print layout** displays the same card template used by printing: a provisional 72 mm content width and 38 mm QR. Screen scale is not physical calibration. **Print card with QR** waits for the real QR and portrait images before requesting the browser print dialog. The embedded browser did not expose a native print dialog during QA; use a browser with print support for paper output. Physical printing and camera scanning still need testing.
7. Open `/review.html` to navigate research, forecast, simulated hatch and dossier scenes in color, monochrome and a six-color palette. Inspect all eight placeholder appearances within one working family, not eight species. An optional fixed 800 × 480 CSS-pixel content area checks layout; it does not establish native hardware resolution or physical size. This page does not save changes or simulate physical display refresh.

The intended experience includes responsive animation with a specimen view and idle rotation through collection, research and encyclopaedia. The review offers manual idle start and an **Automatic idle demo** checkbox, off by default. When enabled, 30 seconds without input starts a 12-second scene cycle. Both timings are provisional. Held inputs defer entry; hidden or unfocused pages pause timers and resume with fresh intervals. The first wake gesture restores the prior view without activating a control; a fresh action works normally. The previews never advance research, claim ownership or award discoveries. Monochrome/six-color modes remain static palette studies; reduced-motion preferences suppress CSS animation.

Progress lives in this browser's local storage. Do not treat it as a backed-up collection or a multi-device authority. A malformed saved record blocks writes instead of silently replacing it. Reset clears this experiment only, after confirmation.

## Boundaries

- `discovery.mjs` is a standalone, unconnected novelty experiment with six tests. It distinguishes new individuals, families and expressed trait/value pairs, excludes carried traits, ignores label changes and rejects conflicting snapshots. Its provisional trait scope is global per stable trait/value pair supplied by the caller. It grants no points or rights and proves no authenticity; evolving snapshots and production eligibility remain undesigned.

- Research and incubation use explicit developer shortcuts; there is no real unattended timer yet.
- One research pack is available per reset. Breeding without a sample remains available.
- The core stores resolved inheritance and sample activation once, with exact parent snapshots and rules/random version identifiers. Replaying the same operation is idempotent; conflicting operation inputs and spent samples are rejected.
- New UI hatches preserve provisional color and monochrome SVG portraits. Older development records without art fall back to their stored expression; production migration/version support is not implemented.
- Shared snapshots persist separately in ignored `prototype/.data/` JSON files. Publication selects public fields and checks birth consistency; repeated identical submissions reuse the record, while conflicting records cannot overwrite it. This is not authenticated provenance or anti-cheat validation. No ownership, breeding permission, scan reward or evolution action is implemented.
- QR links resolve on the running local server. The default localhost address is not reachable by a separate phone. See the opt-in connectivity experiment below.
- Loaded pages use local computation and storage; no CDN/cloud dependency. Cold offline installation/service-worker behavior has not been implemented or claimed.
- Thermal printing, device screens, human playtests, full art review and hardware tests remain pending.

## Optional phone connectivity experiment

For a trusted local network only, start a separate session with `CRITTER_HOST=0.0.0.0` and `CRITTER_PUBLIC_ORIGIN=http://<computer-LAN-IP>:4173` (replace the placeholder with the computer's actual address). Open that same address on the computer before preparing the card. Both devices must reach that address; OS firewall configuration may be required. This has not been verified on a physical phone. Do not port-forward or deploy this unauthenticated prototype publicly.

Loopback is the default; no firewall rules or LAN exposure are enabled by installation. A printed link depends on that server and address remaining available. Offline portable records and production identity/permission design remain open.



Optional browser checks require a separately installed Playwright package and browser; they are not part of the application runtime. Host simulations do not establish physical display, firmware, storage or power performance.
