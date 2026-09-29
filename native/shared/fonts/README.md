# Native font assets

Unmodified Bitstream Vera Sans (`Vera.ttf`) and Vera Sans Bold (`VeraBd.ttf`) is supplied by the ReportLab font distribution. Its exact bundled redistribution license is in [LICENSE.txt](LICENSE.txt). Regular font SHA-256: `c4c45690b345435b2cba52ecabe275f05e49b389b39fe68ad03afbb551288d3d`.

Bold font SHA-256: `cc037385e4d55bfde89b13e03091ee93bf40c0c52ddd391ff031ab276f13b8e9`. The Bold source is copied unmodified from the same ReportLab distribution and covered by the same [license](LICENSE.txt).

Regenerate from this directory with `python generate.py` and Pillow 12.3.0. Pass `--heading` to regenerate only the bold Lab heading atlas. Glyphs cover ASCII 32-126; unsupported bytes display a question mark. Copy deliberately uses ASCII punctuation. Each intended pixel size is independently rasterized with bearing, advance and baseline metrics; no runtime scaling or rasterization occurs. Generated coverage uses unsigned 8-bit alpha.

`portable_font_data.c` contains 9, 11, 20 and 24px masks (47,932 coverage bytes). `lab_font_data.c` contains 18, 19, 21, 22, 26, 34, 24, 28, 32, 36, 40, 44 and 48px masks (514,815 coverage bytes); its declared count is `LAB_FONT_COUNT`. `lab_heading_font_data.c` contains separately rasterized 26, 32, 34 and 40px bold heading masks; its count is `LAB_HEADING_FONT_COUNT`. The regular and portable atlases retain their existing data and ownership. The original six sizes retain their glyph data and order. The additional intended sizes support the initial expedition and saved finding layouts; other Lab scenes retain their existing sizes. Missing Lab sizes assert rather than silently substitute smaller text. Glyph records add 24 bytes per glyph on these targets. The Lab atlas and scene link only into the Linux Lab target behind `CRITTER_LAB_SCENE`; portable targets link only portable assets. Actual linker size reports remain build evidence.

Coverage blends into RGB with integer alpha; Probe thresholds coverage into its independent packed 1bpp scanline. All coordinates clip as signed values before indexing. No font module knows game state, filesystem or transport.

