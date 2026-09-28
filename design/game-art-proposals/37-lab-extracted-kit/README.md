# Approved Lab source-preview extraction kit

Exact crops from the approved [refined C baseline](../35-vault-composition/18-c-refined.png). This is a disposable static reference proof, not functional UI or a set of native production sprite masters. The source screenshot is 1280×720; the depicted screen occupies the measured rectangle x264, y78, 752×421. Its `.png` filename contains JPEG encoding, including existing JPEG artifacts.

[Native contact sheet](contact-sheet-1x.png) displays every extracted asset at its source pixel size. [2× contact sheet](contact-sheet-2x.png) uses nearest-neighbor integer enlargement. [Screen reference](screen-reference.png) retains the complete baseline screen region for comparison. No crop was resized, redrawn, recolored or made transparent; dark backgrounds, edge pixels and source compression remain intact. Contact-sheet labels are inspection captions outside the source artwork.

[Manifest](manifest.json) records the immutable source SHA-256, rectangles, crop hashes and exact decoded pixel identity results. [Extraction script](extract.cjs) uses Node and Sharp, preserves the source, and writes only this directory. With Node and Sharp already available, run `node extract.cjs` from this directory. If Sharp resides in an existing shared dependency directory, set `NODE_PATH` to that directory before running; no machine-specific dependency path is embedded in the script. Regeneration checks existing crop hashes against the same source, and each crop checks its decoded pixels against its source rectangle.

Resources and topic references remain distinct: blue Data, gold Energy and lime Essence identify supplies. Crown and Eye-ring are known feature reference illustrations, not a resolved specimen. The sample capsule is reference art. The two frame fragments include their existing neighboring pixels and are comparison fragments, not seamless nine-slice assets. The focused action crop retains baked-in **START** text and must not be presented as an editable typography component.

Independent review must assess crop bounds, baseline fidelity and suitability for any later composition. This kit makes no production sprite, transparency, font-license, navigation, hardware-performance or player-comprehension claim.

Independent review passed: source hash/dimensions and all nine crop bounds/decoded pixel comparisons verified; icon identities and complete silhouettes inspected. Regeneration by the producer preserved hashes. Frame fragments/action limitations remain as stated.
