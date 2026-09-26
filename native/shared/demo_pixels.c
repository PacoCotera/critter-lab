#include "demo_pixels.h"
#include <stdio.h>
/* Original compact 5x7 instrument glyphs. */
static const uint8_t letters[26][7] = {
    {14, 17, 17, 31, 17, 17, 17}, {30, 17, 17, 30, 17, 17, 30},
    {14, 17, 16, 16, 16, 17, 14}, {30, 17, 17, 17, 17, 17, 30},
    {31, 16, 16, 30, 16, 16, 31}, {31, 16, 16, 30, 16, 16, 16},
    {14, 17, 16, 23, 17, 17, 15}, {17, 17, 17, 31, 17, 17, 17},
    {31, 4, 4, 4, 4, 4, 31},      {7, 2, 2, 2, 18, 18, 12},
    {17, 18, 20, 24, 20, 18, 17}, {16, 16, 16, 16, 16, 16, 31},
    {17, 27, 21, 21, 17, 17, 17}, {17, 25, 21, 19, 17, 17, 17},
    {14, 17, 17, 17, 17, 17, 14}, {30, 17, 17, 30, 16, 16, 16},
    {14, 17, 17, 17, 21, 18, 13}, {30, 17, 17, 30, 20, 18, 17},
    {15, 16, 16, 14, 1, 1, 30},   {31, 4, 4, 4, 4, 4, 4},
    {17, 17, 17, 17, 17, 17, 14}, {17, 17, 17, 17, 17, 10, 4},
    {17, 17, 17, 21, 21, 27, 17}, {17, 17, 10, 4, 10, 17, 17},
    {17, 17, 10, 4, 4, 4, 4},     {31, 1, 2, 4, 8, 16, 31}};
static const uint8_t digits[10][7] = {
    {14, 17, 19, 21, 25, 17, 14}, {4, 12, 4, 4, 4, 4, 14},
    {14, 17, 1, 2, 4, 8, 31},     {30, 1, 1, 14, 1, 1, 30},
    {2, 6, 10, 18, 31, 2, 2},     {31, 16, 16, 30, 1, 1, 30},
    {14, 16, 16, 30, 17, 17, 14}, {31, 1, 2, 4, 8, 8, 8},
    {14, 17, 17, 14, 17, 17, 14}, {14, 17, 17, 15, 1, 1, 14}};
typedef struct {
  uint8_t *rgb;
  unsigned width, y;
  int mono;
} Row;
static void box(Row *r, int x, int y, int w, int h, int shade) {
  if ((int)r->y < y || (int)r->y >= y + h)
    return;
  for (int i = x; i < x + w; ++i)
    if (i >= 0 && i < (int)r->width) {
      uint8_t *p = r->rgb + i * 3;
      p[0] = shade ? 35 : r->mono ? 255 : 246;
      p[1] = shade ? 37 : r->mono ? 255 : 240;
      p[2] = shade ? 34 : r->mono ? 255 : 222;
      if (r->mono && shade)
        p[0] = p[1] = p[2] = 0;
    }
}
static void text(Row *r, int x, int y, const char *value, int scale) {
  int gy = ((int)r->y - y) / scale;
  if ((int)r->y < y || gy >= 7)
    return;
  for (size_t i = 0; value[i]; ++i) {
    unsigned char c = (unsigned char)value[i];
    uint8_t bits = 0;
    if (c >= 'a' && c <= 'z')
      c = (unsigned char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z')
      bits = letters[c - 'A'][gy];
    else if (c >= '0' && c <= '9')
      bits = digits[c - '0'][gy];
    else if (c == '-')
      bits = gy == 3 ? 14 : 0;
    else if (c == ':')
      bits = (gy == 2 || gy == 5) ? 4 : 0;
    else if (c == '.')
      bits = gy == 6 ? 4 : 0;
    else if (c == '/')
      bits = (uint8_t)(1u << (gy < 5 ? gy : 4));
    else if (c != ' ') {
      static const uint8_t q[7] = {14, 17, 1, 2, 4, 0, 4};
      bits = q[gy];
    }
    for (int gx = 0; gx < 5; ++gx)
      if (bits & (16 >> gx))
        box(r, x + (int)i * 6 * scale + gx * scale, (int)r->y, scale, 1, 1);
  }
}
static void sample(Row *r, int x, int y) {
  box(r, x + 12, y, 24, 4, 1);
  box(r, x + 15, y + 4, 18, 8, 1);
  box(r, x + 6, y + 12, 36, 40, 1);
  box(r, x + 9, y + 15, 30, 34, 0);
  for (int i = 0; i < 3; ++i)
    box(r, x + 14, y + 22 + i * 7, 20, 3, 1);
}
void demo_dimensions(int probe, unsigned *w, unsigned *h) {
  *w = probe ? 250 : 400;
  *h = probe ? 122 : 240;
}
void demo_row(const Demo *d, int probe, unsigned y, uint8_t *rgb) {
  Row r = {rgb, probe ? 250u : 400u, y, probe};
  char line[64];
  box(&r, 0, 0, (int)r.width, probe ? 122 : 240, 0);
  if (probe) {
    const char *heading = d->phase < 1    ? "No outing loaded"
                          : d->phase == 1 ? "Material trail"
                          : d->phase == 2 ? "Gathering"
                          : d->phase == 3 ? "Outing complete"
                                          : "Haul saved at Lab";
    text(&r, 12, 10, heading, 1);
    box(&r, 12, 24, 226, 1, 1);
    sample(&r, 12, 34);
    const char *state = d->phase == 0   ? "Prepare at Lab"
                        : d->phase == 1 ? "Ready to start"
                        : d->phase == 2 ? "Sample forming"
                        : d->phase == 3 ? "Sample 01 sealed"
                                        : "Sample 01 saved";
    text(&r, 76, 39, state, 1);
    snprintf(line, sizeof(line), "Reagent supply: %u",
             d->phase == 4 ? 0 : d->reagent);
    text(&r, 76, 54, line, 1);
    if (d->event == 1)
      text(&r, 76, 69, "Fragment waiting", 1);
    else if (d->event == 2) {
      text(&r, 76, 69, "Repeated bands", 1);
      text(&r, 76, 80, "Compare layers", 1);
    }
    text(&r, 12, 105,
         d->phase == 1   ? "START"
         : d->phase == 2 ? "CHECK / INSPECT / LEAVE"
         : d->phase == 3 ? "HAUL"
                         : "MATERIAL TRAIL",
         1);
    return;
  }
  text(&r, 20, 16, "CRITTER LAB", 1);
  text(&r, 260, 16, "PLAYER: EXPLORER", 1);
  box(&r, 20, 34, 360, 2, 1);
  if (d->phase == 0) {
    text(&r, 20, 50, d->lab_page ? "Load review" : "Expeditions", 2);
    box(&r, 20, 80, 360, 2, 1);
    text(&r, 30, 94, "Material trail", 2);
    text(&r, 30, 119, "Medium-length outing", 1);
    text(&r, 30, 135, "Structural clues and study supplies.", 1);
    text(&r, 30, 151, "Fictional event: Layered fragment", 1);
    text(&r, 30, 167, "Difficulty: not yet assigned", 1);
    text(&r, 20, 193,
         d->lab_page   ? "Load this outing onto the Probe."
         : d->selected ? "Selected. Review before loading."
                       : "Choose your next investigation.",
         1);
  } else if (d->phase < 3) {
    text(&r, 20, 50, d->phase == 1 ? "Probe ready" : "Out on the trail", 2);
    sample(&r, 30, 91);
    text(&r, 100, 100, "Material trail", 2);
    text(&r, 100, 127, "Explorer - Outing 01", 1);
    text(&r, 20, 179,
         d->phase == 1 ? "Start the outing on your Probe."
                       : "Check the Probe when convenient.",
         1);
    text(&r, 20, 195, "Your sample will return here.", 1);
  } else if (d->phase == 3) {
    text(&r, 20, 50, "Incoming haul", 2);
    sample(&r, 30, 91);
    text(&r, 100, 99, "Sample 01 - sealed", 1);
    text(&r, 100, 117, "Reagent supply: 2", 1);
    text(&r, 100, 135, "Explorer - Material trail", 1);
    text(&r, 20, 179, "Receive this haul into your Lab.", 1);
    text(&r, 20, 195, "No cargo is added until you receive.", 1);
  } else if (d->lab_page == 2) {
    text(&r, 20, 50, "Sample 01", 2);
    sample(&r, 30, 91);
    text(&r, 100, 99, "Material trail - Outing 01", 1);
    snprintf(line, sizeof(line), "Reagent supply: %u", d->reagent);
    text(&r, 100, 119, line, 1);
    text(&r, 100, 139,
         d->finding ? "Material finding saved" : "Ready for investigation", 1);
    text(&r, 20, 179, "Examine the sample through research.", 1);
    text(&r, 20, 195,
         d->event == 2 ? "Clue: compare layered structures."
                       : "Other genomic regions remain unknown.",
         1);
  } else if (d->lab_page == 3 || d->lab_page == 4) {
    text(&r, 20, 50, d->lab_page == 3 ? "Studies" : "Study review", 2);
    text(&r, 30, 94, "Material study", 2);
    text(&r, 30, 124, "Compare structural possibilities.", 1);
    text(&r, 30, 145, "Cost: 1 reagent", 1);
    snprintf(line, sizeof(line), "Available: %u", d->reagent);
    text(&r, 30, 161, line, 1);
    text(&r, 20, 195,
         d->finding ? "A finding is already saved."
                    : "Run once to save a structural finding.",
         1);
  } else {
    text(&r, 20, 50, "Material finding", 2);
    text(&r, 20, 78, "STRUCTURE - KNOWN ALTERNATIVES", 1);
    for (int i = 0; i < 3; ++i)
      box(&r, 30, 98 + i * 10, 65, 5, 1);
    for (int i = 0; i < 3; ++i)
      box(&r, 170 + i * 18, 98, 9, 25, 1);
    text(&r, 30, 139, "Layered", 1);
    text(&r, 170, 139, "Fibrous", 1);
    text(&r, 20, 167, "Region 02: UNKNOWN   Region 03: UNKNOWN", 1);
    text(&r, 20, 187, "Saved. Reagent remaining: 1", 1);
    text(&r, 20, 203, "There is more to investigate.", 1);
  }
  box(&r, 20, 221, 360, 2, 1);
}
