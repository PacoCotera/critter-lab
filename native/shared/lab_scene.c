#include "demo_pixels.h"
#include "native_font.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {
  uint8_t *pixels;
  unsigned y;
} LabRow;
static const uint8_t palette[][3] = {
  {16, 27, 50}, {8, 17, 34}, {242, 245, 255}, {173, 187, 211},
  {86, 141, 255}, {192, 160, 255}, {255, 182, 92}, {52, 73, 103}};
static void rectangle(LabRow *row, int x, int y, int w, int h, unsigned color) {
  if ((int)row->y < y || (int)row->y >= y + h)
    return;
  for (int column = x; column < x + w; ++column)
    if (column >= 0 && column < 1024)
      memcpy(row->pixels + column * 3, palette[color], 3);
}
static void outline(LabRow *row, int x, int y, int w, int h, unsigned color) {
  rectangle(row, x, y, w, 1, color);
  rectangle(row, x, y + h - 1, w, 1, color);
  rectangle(row, x, y, 1, h, color);
  rectangle(row, x + w - 1, y, 1, h, color);
}
static const NativeFont *font(int size) {
  for (unsigned i = 0; i < LAB_FONT_COUNT; ++i)
    if (lab_fonts[i].size == size)
      return &lab_fonts[i];
  assert(!"Requested Lab font size is absent from the atlas");
  return NULL;
}
static void label(LabRow *row, int x, int y, const char *text, int size,
                  unsigned color) {
  const NativeFont *selected_font = font(size);
  if (selected_font)
    native_text_row(selected_font, text, x, y, row->y, 1024, row->pixels, 0, palette[color]);
}
static void header(LabRow *row, const char *section, const char *title,
                   const char *context) {
  label(row, 32, 16, section, 24, 3);
  label(row, 32, 68, title, 48, 2);
  if (context) label(row, 32, 126, context, 28, 3);
}
static void explanation(LabRow *row, const char *primary,
                        const char *secondary, int second_size) {
  assert(native_text_width(font(44), primary) <= 960);
  label(row, 32, 414, primary, 44, 2);
  if (secondary) {
    assert(native_text_width(font(second_size), secondary) <= 960);
    label(row, 32, second_size == 44 ? 466 : 474, secondary, second_size, 3);
  }
}
static void pattern(LabRow *row) {
  rectangle(row, 32, 172, 520, 214, 1);
  /* Authored occupancy and integer cell geometry: translation only. */
  for (int y = 0; y < 20; ++y)
    for (int x = 0; x < 20; ++x) {
      int value = (x * 17 + y * 29 + x * y * 3 + (y / 4) * 11) % 19;
      if (value < 8)
        rectangle(row, 192 + x * 10, 179 + y * 10, 7, 7, value < 5 ? 5 : 4);
    }
}
static void trail(LabRow *row) {
  rectangle(row, 32, 172, 520, 214, 1);
  /* Conceptual route, not a promised sensor or geographic map. */
  rectangle(row, 120, 248, 180, 3, 4);
  rectangle(row, 298, 248, 3, 95, 4);
  rectangle(row, 298, 340, 160, 3, 4);
  outline(row, 103, 232, 34, 34, 5);
  outline(row, 283, 232, 34, 34, 5);
  outline(row, 441, 324, 34, 34, 5);
}
static void sealed_sample(LabRow *row, int x, int y) {
  outline(row, x, y, 104, 132, 5);
  rectangle(row, x + 26, y - 16, 52, 17, 4);
  rectangle(row, x + 21, y + 36, 62, 3, 5);
  rectangle(row, x + 21, y + 64, 62, 3, 5);
}
static void cargo(LabRow *row) {
  rectangle(row, 32, 172, 520, 214, 1);
  sealed_sample(row, 244, 214);
}
static void categories(LabRow *row, int loading) {
  sealed_sample(row, 620, 172);
  label(row, 746, 194, "Sample", 40, 2);
  rectangle(row, 620, 330, 38, 3, 5);
  rectangle(row, 620, 344, 38, 3, 5);
  label(row, 676, 306, "Lab supplies", 40, 2);
  if (loading) label(row, 600, 349, "Start it on the probe.", 32, 3);
}
static void sketches(LabRow *row) {
  /* Equal illustrative alternatives, preserving existing shape marks. */
  for (int i = 0; i < 4; ++i) {
    int y = 192 + i * 21;
    rectangle(row, 614, y, 145, 8, 4);
    rectangle(row, 630, y + 8, 129, 2, 5);
  }
  for (int fiber = 0; fiber < 7; ++fiber)
    for (int y = 0; y < 83; ++y) {
      int bend = y < 42 ? y / 5 : (83 - y) / 5;
      rectangle(row, 816 + fiber * 16 + bend, 192 + y, 3, 1, 5);
    }
  label(row, 600, 300, "Layered", 40, 2);
  label(row, 810, 300, "Fibrous", 40, 2);
}
static void supplies(LabRow *row, const Demo *demo, int y, int remaining) {
  char quantity[64];
  label(row, 600, y, "Lab supplies", 32, 3);
  snprintf(quantity, sizeof(quantity), "%u unit%s%s", demo->reagent,
           demo->reagent == 1 ? "" : "s", remaining ? " left" : "");
  label(row, 600, y + 42, quantity, 36, 2);
}
static void note(LabRow *row, const Demo *demo, int y) {
  /* Pending and left encounters do not establish a saved clue. */
  if (demo->event == 1) {
    label(row, 600, y, "Encounter waiting", 32, 3);
  } else if (demo->event == 2) {
    label(row, 600, y, "Clue saved:", 32, 3);
    label(row, 600, y + 38, "Repeated bands", 32, 2);
  }
}
static void resources(LabRow *row, const Demo *demo) {
  char line[64];
  label(row, 600, 172, "Lab supplies", 32, 3);
  label(row, 600, 214, "Uses: 1 unit", 36, 2);
  snprintf(line, sizeof(line), "In stock: %u unit%s", demo->reagent,
           demo->reagent == 1 ? "" : "s");
  assert(native_text_width(font(36), line) <= 392);
  label(row, 600, 262, line, 36, 2);
  if (demo->reagent) {
    unsigned after = demo->reagent - 1;
    snprintf(line, sizeof(line), "After: %u unit%s", after, after == 1 ? "" : "s");
    assert(native_text_width(font(36), line) <= 392);
    label(row, 600, 310, line, 36, 2);
  }
}
static void rail(LabRow *row, const Demo *demo) {
  DemoAction actions[12];
  size_t count = demo_actions(demo, actions, 12);
  int x = 32;
  size_t lab_count = 0;
  for (size_t i = 0; i < count; ++i)
    if (!strcmp(actions[i].device, "lab")) ++lab_count;
  if (!lab_count) return;
  label(row, 32, 526, "Controls below", 18, 3);
  rectangle(row, 32, 547, 960, 1, 7);
  for (size_t i = 0; i < count; ++i) {
    if (strcmp(actions[i].device, "lab")) continue;
    int width = native_text_width(font(32), actions[i].label);
    assert(x + width <= 992);
    label(row, x, 552, actions[i].label, 32, 2);
    x += width + 24;
  }
}
void demo_lab_render_row(const Demo *demo, unsigned y, uint8_t *pixels) {
  LabRow row = {pixels, y};
  rectangle(&row, 0, 0, 1024, 600, 0);
  if (demo->phase == 0) {
    header(&row, "Expeditions", "Material trail", NULL);
    trail(&row);
    categories(&row, demo->lab_page != 0);
    if (demo->lab_page == 0) {
      explanation(&row, "Gather a sample and", "supplies for the lab.", 44);
    } else {
      explanation(&row, "Load this outing onto your probe.", NULL, 36);
    }
  } else if (demo->phase == 1) {
    header(&row, "Research lab", "Probe ready", "Material trail");
    trail(&row);
    categories(&row, 0);
    explanation(&row, "The outing is loaded.", "Start it on your probe.", 36);
  } else if (demo->phase == 2) {
    header(&row, "Research lab", "Out on the trail", "Material trail");
    trail(&row);
    label(&row, 600, 172, "Probe gathering", 32, 2);
    note(&row, demo, 238);
    explanation(&row, "Your probe is gathering.",
                "Check the probe for its latest update.", 36);
  } else if (demo->phase == 3) {
    header(&row, "Research lab", "Incoming haul", "From: Material trail");
    cargo(&row);
    label(&row, 600, 172, "Sample 01", 36, 2);
    label(&row, 600, 216, "Sealed", 36, 2);
    supplies(&row, demo, 268, 0);
    if (demo->event == 1) label(&row, 600, 352, "Encounter waiting", 32, 3);
    explanation(&row, "Move the haul into lab storage.",
                "Your probe will be empty.", 36);
  } else {
    int study = !demo->finding && demo->lab_page != 2;
    header(&row, "Research lab", study ? "Structure study" : "Sample 01",
           study ? "Sample 01 - structure" : "From: Material trail");
    pattern(&row);
    if (demo->finding && demo->lab_page == 5) {
      char stock[64];
      sketches(&row);
      snprintf(stock, sizeof(stock), "Lab supplies: %u unit%s left",
               demo->reagent, demo->reagent == 1 ? "" : "s");
      assert(native_text_width(font(32), stock) <= 392);
      label(&row, 600, 348, stock, 32, 3);
      explanation(&row, "Layered or fibrous: both are possible.",
                  "Neither is chosen. Other regions are unknown.", 36);
    } else if (demo->finding) {
      supplies(&row, demo, 172, 1);
      note(&row, demo, 274);
      explanation(&row, "Layered and fibrous forms are possible.",
                  "Neither is chosen. Other regions are unknown.", 36);
    } else if (study) {
      resources(&row, demo);
      if (demo->reagent) {
        explanation(&row, "What forms are possible", "for Sample 01?", 44);
      } else {
        /* Defensive display only: valid gameplay cannot reach zero-stock study. */
        explanation(&row, "This study needs supplies.",
                    "It uses 1 unit of lab supplies.", 36);
      }
    } else {
      supplies(&row, demo, 172, 0);
      note(&row, demo, 274);
      if (demo->event == 2) label(&row, 600, 344, "Other regions unknown", 32, 3);
      explanation(&row, "Its structure is still unknown.",
                  demo->event == 2 ? "Bands suggest studying its structure."
                                   : "Other regions are unknown.", 36);
    }
  }
  rail(&row, demo);
}
