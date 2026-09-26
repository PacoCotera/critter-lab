#ifndef NATIVE_FONT_H
#define NATIVE_FONT_H
#include <stdint.h>
typedef struct {
  uint32_t offset;
  int width, height, left, top, advance;
} NativeGlyph;
typedef struct {
  int size, baseline;
  const uint8_t *coverage;
  const NativeGlyph *glyphs;
} NativeFont;
extern const NativeFont portable_fonts[4];
extern const NativeFont lab_fonts[6];
int native_text_width(const NativeFont *font, const char *text);
void native_text_row(const NativeFont *font, const char *text, int x, int y,
                     unsigned row_y, unsigned width, uint8_t *pixels,
                     int mono, const uint8_t color[3]);
#endif
