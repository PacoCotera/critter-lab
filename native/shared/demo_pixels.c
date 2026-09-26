#include "demo_pixels.h"
#include <stdio.h>
#include <string.h>
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
static const DemoDisplayProfile profiles[] = {
    {1024, 600, DEMO_RGB888, 3072},
    {122, 250, DEMO_MONO1, 16},
    {368, 448, DEMO_RGB888, 1104}};
const DemoDisplayProfile *demo_display_profile(DemoDisplay display) {
  if (display < DEMO_LAB || display > DEMO_COMPANION)
    return NULL;
  return &profiles[display];
}
typedef struct {
  uint8_t *pixels;
  const DemoDisplayProfile *profile;
  unsigned width, y;
} Row;
static void pixel(Row *r, unsigned x, int shade) {
  if (r->profile->format == DEMO_MONO1) {
    uint8_t mask = (uint8_t)(0x80u >> (x % 8));
    if (shade)
      r->pixels[x / 8] |= mask;
    else
      r->pixels[x / 8] &= (uint8_t)~mask;
  } else {
    uint8_t *color = r->pixels + x * 3;
    static const uint8_t colors[][3] = {
        {242, 235, 221}, {41, 42, 39}, {216, 203, 176},
        {198, 107, 46}, {71, 99, 77}};
    memcpy(color, colors[shade], 3);
  }
}
static void box(Row *r, int x, int y, int w, int h, int shade) {
  if ((int)r->y < y || (int)r->y >= y + h)
    return;
  for (int i = x; i < x + w; ++i)
    if (i >= 0 && i < (int)r->width)
      pixel(r, (unsigned)i, shade);
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
static void probe_scene(Row *r, const Demo *d) {
  const char *title = "NO OUTING";
  const char *state = "PREPARE AT LAB";
  const char *hint = "";
  if (d->phase == 1) {
    title = "READY";
    state = "MATERIAL TRAIL";
    hint = "START";
  } else if (d->phase == 2) {
    title = d->event == 1 ? "OBSERVATION" : "GATHERING";
    state = d->event == 1 ? "REPEATED BANDS" : "SAMPLE FORMING";
    hint = d->event == 1 ? "INSPECT / LEAVE" : "CHECK STATUS";
  } else if (d->phase == 3) {
    title = "OUTING COMPLETE";
    state = "SAMPLE 01 SEALED";
    hint = "RETURN TO LAB";
  } else if (d->phase == 4) {
    title = "SAMPLE RECEIVED";
    state = "SAMPLE 01";
    hint = "REVIEW AT LAB";
  }
  text(r, 8, 10, title, d->phase == 1 ? 2 : 1);
  box(r, 8, 30, 106, 1, 1);
  sample(r, 37, 42);
  text(r, 8, 103, state, 1);
  if (d->event == 2 && d->phase == 2) {
    text(r, 8, 133, "CLUE: REPEATED", 1);
    text(r, 8, 146, "BANDS", 1);
    text(r, 8, 166, "SUGGESTS STRUCTURE", 1);
  }
  text(r, 8, 205, hint, 1);
}
static void companion_scene(Row *r) {
  text(r, 24, 28, "COMPANION", 2);
  box(r, 156, 112, 56, 2, 1);
  box(r, 156, 166, 56, 2, 1);
  box(r, 156, 112, 2, 56, 1);
  box(r, 210, 112, 2, 56, 1);
  box(r, 170, 136, 28, 8, 1);
  text(r, 24, 210, "NO CRITTER SELECTED", 2);
}
static void outline(Row *r, int x, int y, int w, int h, int shade) {
  box(r, x, y, w, 2, shade);
  box(r, x, y + h - 2, w, 2, shade);
  box(r, x, y, 2, h, shade);
  box(r, x + w - 2, y, 2, h, shade);
}
static void regions(Row *r, int known) {
  box(r, 32, 144, 368, 252, 2);
  text(r, 48, 160, "SAMPLE 01 REGIONS", 3);
  const char *labels[] = {known ? "STRUCTURE KNOWN" : "STRUCTURE ?",
                          "REGION 02 UNKNOWN", "REGION 03 UNKNOWN"};
  for (int i = 0; i < 3; ++i) {
    outline(r, 48, 196 + i * 60, 336, 48, 1);
    text(r, 60, 208 + i * 60, labels[i], 3);
  }
}
static void trail(Row *r) {
  box(r, 32, 144, 368, 252, 2);
  text(r, 48, 160, "MATERIAL TRAIL", 4);
  box(r, 100, 240, 140, 4, 1);
  box(r, 236, 240, 4, 94, 1);
  box(r, 236, 330, 102, 4, 1);
  outline(r, 80, 220, 40, 40, 1);
  outline(r, 220, 220, 40, 40, 1);
  outline(r, 318, 312, 40, 40, 1);
}
static void action_slot(Row *r, int x, int width, const char *label) {
  outline(r, x, 526, width, 48, 3);
  text(r, x + 14, 540, label, 3);
}
static void lab_scene(Row *r, const Demo *d) {
  char stock[64];
  text(r, 32, 20, "CRITTER LAB", 3);
  text(r, 788, 20, "EXPLORER", 3);
  box(r, 32, 58, 960, 3, 1);
  box(r, 32, 504, 960, 3, 1);
  const char *title = d->phase == 0 ? (d->lab_page ? "LOAD REVIEW" : "EXPEDITIONS")
                      : d->phase == 1 ? "PROBE READY"
                      : d->phase == 2 ? "OUT ON THE TRAIL"
                      : d->phase == 3 ? "INCOMING HAUL"
                      : d->lab_page == 5 ? "MATERIAL FINDING"
                      : d->lab_page == 2 ? "SAMPLE 01" : "STUDY REVIEW";
  text(r, 32, 80, title, 5);
  if (d->phase == 0) {
    trail(r);
    text(r, 440, 148, "MEDIUM-LENGTH OUTING", 3);
    text(r, 440, 190, "STRUCTURAL CLUES", 3);
    text(r, 440, 220, "AND STUDY SUPPLIES", 3);
    text(r, 440, 262, "EVENT: LAYERED FRAGMENT", 3);
    text(r, 440, 304, "DIFFICULTY UNASSIGNED", 3);
    text(r, 32, 430, d->lab_page ? "LOAD THIS OUTING ONTO THE PROBE."
                               : "CHOOSE AN OUTING - REVIEW BEFORE LOADING.", 3);
    if (d->lab_page) {
      text(r, 32, 460, "START IT SEPARATELY ON THE PROBE.", 3);
      action_slot(r, 32, 210, "BACK");
    }
    action_slot(r, 440, 552, d->lab_page ? "LOAD PROBE" : "REVIEW OUTING");
  } else if (d->phase < 3) {
    trail(r);
    text(r, 440, 148, "OUTING 01", 3);
    text(r, 440, 210, d->phase == 1 ? "START ON YOUR PROBE." : "MATERIAL TRAIL", 3);
    if (d->phase == 2) {
      text(r, 440, 252, "CHECK THE PROBE", 3);
      text(r, 440, 282, "WHEN CONVENIENT.", 3);
    }
    text(r, 32, 430, "YOUR SAMPLE WILL RETURN HERE.", 3);
  } else if (d->phase == 3) {
    box(r, 32, 144, 368, 252, 2);
    text(r, 48, 160, "SAMPLE 01", 4);
    outline(r, 150, 230, 100, 120, 1);
    box(r, 176, 216, 48, 16, 1);
    text(r, 440, 148, "SAMPLE 01 - SEALED", 3);
    snprintf(stock, sizeof(stock), "REAGENT SUPPLY: %u", d->reagent);
    text(r, 440, 190, stock, 3);
    text(r, 440, 232, "MATERIAL TRAIL / OUTING 01", 3);
    text(r, 32, 430, "RECEIVE THE HAUL INTO YOUR LAB.", 3);
    text(r, 32, 460, "CARGO IS ADDED WHEN YOU RECEIVE.", 3);
    action_slot(r, 440, 552, "RECEIVE HAUL");
  } else {
    regions(r, d->finding);
    if (d->lab_page == 5) {
      outline(r, 440, 144, 252, 150, 1);
      outline(r, 724, 144, 268, 150, 1);
      for (int i = 0; i < 3; ++i) {
        box(r, 472, 176 + i * 32, 186, 12, 1);
        box(r, 764 + i * 72, 170, 16, 100, 1);
      }
      text(r, 440, 306, "LAYERED", 3);
      text(r, 724, 306, "FIBROUS", 3);
      if (d->event == 2) {
        text(r, 440, 350, "BANDS SUGGESTED WHERE", 3);
        text(r, 440, 380, "TO INVESTIGATE.", 3);
      } else
        text(r, 440, 350, "REFERENCE DIAGRAMS", 3);
      int saved_y = d->event == 2 ? 414 : 394;
      box(r, 440, saved_y, 22, 22, 4);
      text(r, 474, saved_y, "SAVED", 3);
      snprintf(stock, sizeof(stock), "REAGENT REMAINING: %u", d->reagent);
      text(r, 600, saved_y, stock, 2);
      text(r, 32, 450, "RESEARCH SUPPORTS LAYERED AND FIBROUS FORMS.", 3);
      text(r, 32, 480, "NO FORM SELECTED. OTHER REGIONS UNRESOLVED.", 3);
      action_slot(r, 32, 210, "SAMPLE");
    } else if (d->lab_page == 2) {
      text(r, 440, 148, "STRUCTURAL REGION", 3);
      text(r, 440, 190, d->finding ? "SUPPORTED FORMS KNOWN" : "UNRESOLVED", 3);
      text(r, 440, 232, "OTHER REGIONS UNRESOLVED", 3);
      snprintf(stock, sizeof(stock), "REAGENT SUPPLY: %u", d->reagent);
      text(r, 440, 274, stock, 3);
      text(r, 32, 430, d->event == 2 ? "OBSERVED BANDS SUGGEST STUDYING STRUCTURE."
                                   : "INVESTIGATE THE STRUCTURAL REGION.", 3);
      action_slot(r, 440, 552, d->finding ? "VIEW FINDING" : "REVIEW MATERIAL STUDY");
    } else {
      text(r, 440, 148, "MATERIAL STUDY", 3);
      text(r, 440, 190, "EXAMINES STRUCTURE", 3);
      text(r, 440, 232, "COST: 1 REAGENT", 3);
      snprintf(stock, sizeof(stock), "AVAILABLE: %u", d->reagent);
      text(r, 440, 274, stock, 3);
      text(r, 32, 430, "WHICH STRUCTURAL POSSIBILITIES", 3);
      text(r, 32, 460, "DOES THIS SAMPLE SUPPORT?", 3);
      action_slot(r, 32, 210, "BACK");
      action_slot(r, 440, 552, d->finding ? "VIEW FINDING"
                                  : d->reagent ? "RUN STUDY" : "NEED 1 REAGENT");
    }
  }
}
int demo_render_row(const Demo *d, DemoDisplay display, unsigned y,
                    uint8_t *pixels, size_t capacity) {
  const DemoDisplayProfile *profile = demo_display_profile(display);
  if (!d || !profile || !pixels || y >= profile->height ||
      capacity < profile->row_bytes)
    return 0;
  memset(pixels, 0, profile->row_bytes);
  Row r = {pixels, profile, profile->width, y};
  box(&r, 0, 0, (int)profile->width, (int)profile->height, 0);
  if (display == DEMO_PROBE) {
    probe_scene(&r, d);
    return 1;
  }
  if (display == DEMO_COMPANION) {
    companion_scene(&r);
    return 1;
  }
  lab_scene(&r, d);
  return 1;
}
