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
static void pattern(LabRow *row) {
  rectangle(row, 32, 158, 532, 287, 1);
  label(row, 52, 168, "Sample pattern", 18, 3);
  /* Original Homecoming 20x20 occupancy: decorative specimen identity only. */
  for (int y = 0; y < 20; ++y)
    for (int x = 0; x < 20; ++x) {
      int value = (x * 17 + y * 29 + x * y * 3 + (y / 4) * 11) % 19;
      if (value < 8)
        rectangle(row, 178 + x * 10, 198 + y * 10, 7, 7, value < 5 ? 5 : 4);
    }
  for (int side = 0; side < 2; ++side) {
    int x = side ? 392 : 164;
    rectangle(row, x, 187, 17, 2, 5);
    rectangle(row, x, 411, 17, 2, 5);
    rectangle(row, side ? 407 : 164, 187, 2, 17, 5);
    rectangle(row, side ? 407 : 164, 396, 2, 17, 5);
  }
}
static void trail(LabRow *row) {
  rectangle(row, 32, 158, 532, 287, 1);
  label(row, 52, 176, "Material trail", 22, 2);
  /* A conceptual route, not a promised sensor or geographic map. */
  rectangle(row, 120, 268, 180, 3, 4);
  rectangle(row, 298, 268, 3, 95, 4);
  rectangle(row, 298, 360, 160, 3, 4);
  outline(row, 103, 252, 34, 34, 5);
  outline(row, 283, 252, 34, 34, 5);
  outline(row, 441, 344, 34, 34, 5);
}
static void cargo(LabRow *row) {
  rectangle(row, 32, 158, 532, 287, 1);
  label(row, 52, 176, "Sample 01 - sealed", 22, 2);
  outline(row, 244, 256, 104, 132, 5);
  rectangle(row, 270, 238, 52, 19, 4);
  rectangle(row, 265, 292, 62, 3, 5);
  rectangle(row, 265, 320, 62, 3, 5);
}
static void sketches(LabRow *row) {
  label(row, 602, 158, "Form sketches", 18, 3);
  for (int i = 0; i < 4; ++i) {
    int y = 204 + i * 21;
    rectangle(row, 614, y, 145, 8, 4);
    rectangle(row, 630, y + 8, 129, 2, 5);
  }
  /* Equal footprint, contrasting shape: illustrative alternatives only. */
  for (int fiber = 0; fiber < 7; ++fiber)
    for (int y = 0; y < 83; ++y) {
      int bend = y < 42 ? y / 5 : (83 - y) / 5;
      rectangle(row, 816 + fiber * 16 + bend, 204 + y, 3, 1, 5);
    }
  label(row, 610, 300, "Layered", 22, 2);
  label(row, 818, 300, "Fibrous", 22, 2);
  rectangle(row, 602, 342, 390, 1, 7);
}
static void rail(LabRow *row, const Demo *demo) {
  label(row, 32, 510, "Controls below", 18, 3);
  rectangle(row, 32, 537, 960, 1, 7);
  DemoAction actions[12];
  size_t count = demo_actions(demo, actions, 12);
  int left = 32, primary = 602;
  for (size_t i = 0; i < count; ++i) {
    if (strcmp(actions[i].device, "lab")) continue;
    int x = !strcmp(actions[i].name, "back") ? left : primary;
    if (!strcmp(actions[i].name, "inspect")) x = 300;
    label(row, x + 16, 556, actions[i].label, 19, 2);
  }
}
static void priority_rail(LabRow *row, const Demo *demo, int finding) {
  DemoAction actions[12];
  size_t count = demo_actions(demo, actions, 12);
  label(row, 32, 526, "Controls below", 18, 3);
  rectangle(row, 32, 547, 960, 1, 7);
  int action_right = 32;
  for (size_t i = 0; i < count; ++i) {
    if (strcmp(actions[i].device, "lab")) continue;
    int x = !strcmp(actions[i].name, "inspect") ? 320 : 32;
    label(row, x, 552, actions[i].label, 32, 2);
    int right = x + native_text_width(font(32), actions[i].label);
    if (right > action_right) action_right = right;
  }
  if (finding) {
    char stock[80];
    snprintf(stock, sizeof(stock), "Lab supplies: %u unit%s left",
             demo->reagent, demo->reagent == 1 ? "" : "s");
    int x = 992 - native_text_width(font(32), stock);
    assert(x >= action_right + 24);
    label(row, x, 552, stock, 32, 3);
  }
}
static void priority_outing(LabRow *row, const Demo *demo) {
  label(row, 32, 16, "Expeditions", 24, 3);
  label(row, 32, 68, "Material trail", 48, 2);
  rectangle(row, 32, 156, 520, 230, 1);
  rectangle(row, 120, 248, 180, 3, 4);
  rectangle(row, 298, 248, 3, 95, 4);
  rectangle(row, 298, 340, 160, 3, 4);
  outline(row, 103, 232, 34, 34, 5);
  outline(row, 283, 232, 34, 34, 5);
  outline(row, 441, 324, 34, 34, 5);
  /* Reuse the sealed cargo symbol and its bands for expected categories. */
  outline(row, 620, 172, 104, 132, 5);
  rectangle(row, 646, 156, 52, 17, 4);
  rectangle(row, 641, 208, 62, 3, 5);
  rectangle(row, 641, 236, 62, 3, 5);
  label(row, 746, 194, "Sample", 40, 2);
  rectangle(row, 620, 340, 38, 3, 5);
  rectangle(row, 620, 354, 38, 3, 5);
  label(row, 676, 330, "Lab supplies", 40, 2);
  label(row, 32, 414, "Gather a sample and", 44, 2);
  label(row, 32, 466, "supplies for the lab.", 44, 2);
  priority_rail(row, demo, 0);
}
static void priority_finding(LabRow *row, const Demo *demo) {
  label(row, 32, 16, "Research lab", 24, 3);
  label(row, 32, 68, "Sample 01", 48, 2);
  label(row, 32, 126, "From: Material trail", 28, 3);
  rectangle(row, 32, 172, 520, 214, 1);
  /* Same authored occupancy, cell step and filled square size; translation only. */
  for (int y = 0; y < 20; ++y)
    for (int x = 0; x < 20; ++x) {
      int value = (x * 17 + y * 29 + x * y * 3 + (y / 4) * 11) % 19;
      if (value < 8)
        rectangle(row, 192 + x * 10, 179 + y * 10, 7, 7, value < 5 ? 5 : 4);
    }
  /* Equal illustrative alternatives, retaining the existing shape marks. */
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
  const char *result = "Layered or fibrous: both are possible.";
  if (native_text_width(font(44), result) > 960)
    result = "Both forms are possible.";
  label(row, 32, 414, result, 44, 2);
  label(row, 32, 474, "Neither is chosen. Other regions are unknown.", 36, 3);
  priority_rail(row, demo, 1);
}
void demo_lab_render_row(const Demo *demo, unsigned y, uint8_t *pixels) {
  LabRow row = {pixels, y};
  char stock[80];
  rectangle(&row, 0, 0, 1024, 600, 0);
  if (demo->phase == 0 && demo->lab_page == 0) {
    priority_outing(&row, demo);
    return;
  }
  if (demo->phase == 4 && demo->finding && demo->lab_page == 5) {
    priority_finding(&row, demo);
    return;
  }
  label(&row, 32, 20, "CRITTER LAB", 18, 3);
  label(&row, 823, 20, "Research lab", 18, 3);
  rectangle(&row, 32, 52, 960, 1, 7);
  const char *title = demo->phase == 0 ? (demo->lab_page ? "Material trail" : "Expeditions")
    : demo->phase == 1 ? "Probe ready" : demo->phase == 2 ? "Out on the trail"
    : demo->phase == 3 ? "Incoming haul"
    : !demo->finding && (demo->lab_page == 3 || demo->lab_page == 4) ? "Structure study" : "Sample 01";
  label(&row, 32, 76, title, 34, 2);
  label(&row, 32, 119, demo->phase == 4 && !demo->finding && demo->lab_page != 2 && demo->lab_page != 5
    ? "Sample 01" : demo->phase == 3 || demo->phase == 4 ? "From: Material trail" : "Material trail", 18, 3);
  if (demo->phase < 3) {
    trail(&row);
    label(&row, 602, 158, "Outing details", 26, 2);
    label(&row, 602, 218, "A sample to investigate", 21, 2);
    label(&row, 602, 251, "and supplies for the lab.", 21, 2);
    if (demo->phase == 0) {
      label(&row, 602, 303, demo->lab_page ? "Load this outing onto your probe." : "Gather a sample and lab supplies", 21, 2);
      label(&row, 602, 336, demo->lab_page ? "Start it there when you're ready." : "on the Material trail.", 21, 2);
    }
    label(&row, 32, 478, demo->phase == 0 ? (demo->lab_page ? "Loading the outing does not start it." : "Choose an outing to explore.")
      : demo->phase == 1 ? "The outing is loaded. Start it on your probe."
      : "Your probe is gathering. Check it for the latest update.", 21, 3);
  } else if (demo->phase == 3) {
    cargo(&row);
    label(&row, 602, 158, "Haul manifest", 26, 2);
    label(&row, 602, 218, "Sample 01 - sealed", 21, 2);
    snprintf(stock, sizeof(stock), "Lab supplies: %u units", demo->reagent);
    label(&row, 602, 251, stock, 21, 2);
    label(&row, 602, 303, "Receive the haul to move the", 21, 2);
    label(&row, 602, 336, "sample and supplies into storage.", 21, 2);
    label(&row, 32, 478, "Receive the haul at the lab. Your probe will be empty.", 21, 3);
  } else {
    pattern(&row);
    if (demo->lab_page == 5) {
      label(&row, 602, 76, "Two possible forms", 26, 2);
      sketches(&row);
      label(&row, 602, 357, "Layered or fibrous:", 21, 2);
      label(&row, 602, 386, "both are possible.", 21, 2);
      label(&row, 602, 415, "No form has been chosen.", 21, 2);
      label(&row, 32, 478, "Other regions are still unknown.", 21, 3);
      snprintf(stock, sizeof(stock), "Lab supplies: %u unit%s left", demo->reagent, demo->reagent==1 ? "" : "s");
      label(&row, 720, 554, stock, 18, 3);
    } else if (demo->lab_page == 2 || demo->finding) {
      label(&row, 602, 158, demo->finding ? "Possible forms known" : "Not yet understood", 26, 2);
      label(&row, 602, 218, demo->finding ? "Layered and fibrous forms" : "The sample's structure", 21, 2);
      label(&row, 602, 251, demo->finding ? "are possible." : "is still unknown.", 21, 2);
      label(&row, 602, 303, "Other regions are still unknown.", 21, 2);
      snprintf(stock, sizeof(stock), "Lab supplies: %u unit%s", demo->reagent, demo->reagent==1 ? "" : "s");
      label(&row, 602, 356, stock, 21, 2);
      label(&row, 32, 478, demo->finding ? "View the finding to revisit what you discovered."
        : demo->event==2 ? "Repeated bands suggest looking at its structure."
        : "Review a study to investigate this sample.", 21, 3);
    } else {
      label(&row, 602, 158, "What forms are possible", 26, 2);
      label(&row, 602, 192, "for Sample 01?", 26, 2);
      label(&row, 602, 245, "Check the sample's structure", 21, 2);
      label(&row, 602, 276, "to find its possible forms.", 21, 2);
      label(&row, 602, 322, "Lab supplies / Uses 1 unit", 21, 2);
      snprintf(stock, sizeof(stock), "In stock: %u units", demo->reagent);
      label(&row, 602, 355, stock, 21, 2);
      snprintf(stock, sizeof(stock), "After study: %u unit%s", demo->reagent ? demo->reagent - 1 : 0, demo->reagent==2 ? "" : "s");
      label(&row, 602, 388, stock, 21, 2);
      label(&row, 32, 478, demo->reagent ? "Start the study to discover its possible forms." : "This study needs 1 unit of lab supplies.", 21, 3);
    }
  }
  rail(&row, demo);
}
