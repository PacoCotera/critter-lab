#include "demo_pixels.h"
#include "native_font.h"
#include <stdio.h>
#include <string.h>
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
static void text(Row *r, int x, int y, const char *value, int size) {
  const NativeFont *font = &portable_fonts[0];
  for (unsigned i = 0; i < 4; ++i)
    if (portable_fonts[i].size == size)
      font = &portable_fonts[i];
  static const uint8_t ink[3] = {41, 42, 39};
  native_text_row(font, value, x, y, r->y, r->width, r->pixels,
                  r->profile->format == DEMO_MONO1, ink);
}
static void wrapped(Row *r, int x, int y, const char *value, int width) {
  char line[80];
  size_t length = 0;
  while (*value) {
    const char *word = value;
    while (*value && *value != ' ') ++value;
    size_t word_length = (size_t)(value-word);
    char candidate[80];
    memcpy(candidate, line, length);
    size_t offset = length;
    if (offset) candidate[offset++] = ' ';
    memcpy(candidate + offset, word, word_length);
    candidate[offset + word_length] = 0;
    if (length && native_text_width(&portable_fonts[0], candidate) > width) {
      line[length] = 0;
      text(r, x, y, line, 9);
      y += 14;
      length = 0;
    }
    if (length) line[length++] = ' ';
    memcpy(line + length, word, word_length);
    length += word_length;
    while (*value == ' ') ++value;
  }
  line[length] = 0;
  text(r, x, y, line, 9);
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
  const char *title = d->phase == 0 || d->phase == 4 ? "Probe empty"
    : d->phase == 1 ? "Ready" : d->phase == 2 ? (d->event == 1 ? "Fragment found" : "Gathering")
    : d->probe_page == 2 ? "Haul" : "Outing complete";
  text(r, 8, 10, title, 11);
  box(r, 8, 30, 106, 1, 1);
  if (d->phase == 0 || d->phase == 4) {
    wrapped(r, 8, 133, d->phase == 0 ? "Load an outing at the lab." : "Haul moved to the lab.", 106);
  } else {
    text(r, 8, 40, d->phase < 3 ? "Material trail" : "Sample 01 sealed", 9);
    sample(r, 37, 76);
    const char *message = d->phase == 1 ? "The outing is loaded."
      : d->phase == 3 ? (d->probe_page == 2 ? "Lab supplies: 2 units. Receive it at the lab." : "Bring the haul to the lab.")
      : d->event == 1 ? "Repeated bands mark the fragment."
      : d->event == 2 ? "Clue saved: repeated bands. Suggests structure."
      : d->event == 3 ? "Fragment left behind." : "The probe is gathering.";
    wrapped(r, 8, 133, message, 106);
  }
  DemoAction actions[12];
  size_t count = demo_actions(d, actions, 12);
  int slot_y = 204;
  if (d->phase != 0 && d->phase != 4)
    text(r, 8, 188, "Controls below", 9);
  for (size_t i = 0; i < count; ++i) {
    if (strcmp(actions[i].device, "probe")) continue;
    text(r, 12, slot_y + 1, actions[i].label, 9);
    slot_y += 13;
  }
}
static void companion_scene(Row *r) {
  text(r, 24, 32, "Companion", 24);
  box(r, 64, 112, 240, 2, 1);
  box(r, 64, 270, 240, 2, 1);
  box(r, 64, 112, 2, 160, 1);
  box(r, 302, 112, 2, 160, 1);
  text(r, 80, 310, "No critter here yet.", 20);
}
#ifdef CRITTER_LAB_SCENE
void demo_lab_render_row(const Demo *demo, unsigned y, uint8_t *pixels);
#endif
int demo_render_row(const Demo *d, DemoDisplay display, unsigned y,
                    uint8_t *pixels, size_t capacity) {
  const DemoDisplayProfile *profile = demo_display_profile(display);
  if (!d || !profile || !pixels || y >= profile->height ||
      capacity < profile->row_bytes)
    return 0;
#ifndef CRITTER_LAB_SCENE
  if (display == DEMO_LAB)
    return 0;
#endif
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
#ifdef CRITTER_LAB_SCENE
  demo_lab_render_row(d, y, pixels);
#else
  /* MCU entries have only their portable scene resources. */
  return 0;
#endif
  return 1;
}
