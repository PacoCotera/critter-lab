# Native font assets

Unmodified Bitstream Vera Sans (`Vera.ttf`) is supplied by the ReportLab font distribution. Its exact bundled redistribution license is in [LICENSE.txt](LICENSE.txt). Font SHA-256: `c4c45690b345435b2cba52ecabe275f05e49b389b39fe68ad03afbb551288d3d`.

Regenerate from this directory with `python generate.py` and Pillow 12.3.0. Glyphs cover ASCII 32-126; unsupported bytes display a question mark. Copy deliberately uses ASCII punctuation. Each intended pixel size is independently rasterized with bearing, advance and baseline metrics; no runtime scaling or rasterization occurs. Generated coverage uses unsigned 8-bit alpha.

`portable_font_data.c` contains 9, 11, 20 and 24px masks (47,932 coverage bytes). `lab_font_data.c` contains 18, 19, 21, 22, 26, 34, 24, 28, 32, 36, 40, 44 and 48px masks (514,815 coverage bytes); its declared count is `LAB_FONT_COUNT`. The original six sizes retain their glyph data and order. The additional intended sizes support the initial outing and saved finding layouts; other Lab scenes retain their existing sizes. Missing Lab sizes assert rather than silently substitute smaller text. Glyph records add 24 bytes per glyph on these targets. The Lab atlas and scene link only into the Linux Lab target behind `CRITTER_LAB_SCENE`; portable targets link only portable assets. Actual linker size reports remain build evidence.

Coverage blends into RGB with integer alpha; Probe thresholds coverage into its independent packed 1bpp scanline. All coordinates clip as signed values before indexing. No font module knows game state, filesystem or transport.

