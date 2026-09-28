#include "selected_lab.h"
#include "assets.h"
#include "native_font.h"
#include <assert.h>
#include <string.h>

typedef struct { unsigned y; uint8_t *pixels; } SelectedRow;
enum { BASE, PANEL, INK, MUTED, BLUE, EDGE, DEEP, WARM, SAGE, ACTION };
static const uint8_t colors[][3] = {
  {25, 36, 43}, {35, 43, 48}, {214, 222, 226}, {183, 198, 205},
  {24, 143, 234}, {70, 140, 184}, {16, 26, 33}, {237, 197, 106},
  {163, 206, 159}, {41, 41, 34}
};

static void rectangle(SelectedRow *row, int x, int y, int width, int height, unsigned color) {
  if ((int)row->y < y || (int)row->y >= y + height) return;
  for (int column = x; column < x + width; ++column)
    if (column >= 0 && column < (int)SELECTED_LAB_WIDTH) memcpy(row->pixels + column * 3, colors[color], 3);
}

static void outline(SelectedRow *row, int x, int y, int width, int height, int weight, unsigned color) {
  rectangle(row, x, y, width, weight, color);
  rectangle(row, x, y + height - weight, width, weight, color);
  rectangle(row, x, y, weight, height, color);
  rectangle(row, x + width - weight, y, weight, height, color);
}

static void stepped(SelectedRow *row, int x, int y, int width, int height, int corner, unsigned color) {
  int position = (int)row->y - y;
  if (position < 0 || position >= height) return;
  int edge = position < height / 2 ? position : height - position - 1;
  int inset = edge < corner ? ((corner - edge + 3) / 4) * 4 : 0;
  rectangle(row, x + inset, y, width - 2 * inset, height, color);
}

static void panel(SelectedRow *row, int x, int y, int width, int height) {
  stepped(row, x - 4, y - 4, width + 8, height + 8, 16, DEEP);
  stepped(row, x, y, width, height, 12, BLUE);
  stepped(row, x + 5, y + 5, width - 10, height - 10, 8, PANEL);
  rectangle(row, x + 13, y + 5, width - 26, 2, EDGE);
  outline(row, x + 15, y + 15, width - 30, height - 30, 2, EDGE);
}

static const NativeFont *font(int size) {
  for (unsigned index = 0; index < LAB_FONT_COUNT; ++index)
    if (lab_fonts[index].size == size) return &lab_fonts[index];
  assert(!"Missing existing native Lab font size");
  return NULL;
}

static void label(SelectedRow *row, int x, int y, const char *text, int size, unsigned color) {
  native_text_row(font(size), text, x, y, row->y, SELECTED_LAB_WIDTH, row->pixels, 0, colors[color]);
}

static void sprite(SelectedRow *row, unsigned asset, int x, int y, unsigned width, unsigned height) {
  if ((int)row->y < y || (int)row->y >= y + (int)height) return;
  const SelectedSprite *source = &selected_sprites[asset];
  unsigned source_y = ((unsigned)((int)row->y - y) * source->height) / height;
  for (unsigned column = 0; column < width; ++column) {
    int destination = x + (int)column;
    if (destination < 0 || destination >= (int)SELECTED_LAB_WIDTH) continue;
    unsigned source_x = column * source->width / width;
    memcpy(row->pixels + destination * 3, source->pixels + (source_y * source->width + source_x) * 3, 3);
  }
}

static void focus(SelectedRow *row, int x, int y, int width, int height) {
  const int weight = 4, length = 22;
  rectangle(row, x, y, length, weight, WARM);
  rectangle(row, x, y, weight, length, WARM);
  rectangle(row, x + width - length, y, length, weight, WARM);
  rectangle(row, x + width - weight, y, weight, length, WARM);
  rectangle(row, x, y + height - weight, length, weight, WARM);
  rectangle(row, x, y + height - length, weight, length, WARM);
  rectangle(row, x + width - length, y + height - weight, length, weight, WARM);
  rectangle(row, x + width - weight, y + height - length, weight, length, WARM);
}

void selected_lab_row(const SelectedLab *lab, unsigned y, uint8_t pixels[SELECTED_LAB_WIDTH * 3]) {
  SelectedRow row = {y, pixels};
  rectangle(&row, 0, 0, SELECTED_LAB_WIDTH, SELECTED_LAB_HEIGHT, BASE);
  panel(&row, 34, 28, 953, 103);
  sprite(&row, SPRITE_SAMPLE, 55, 43, 79, 79);
  label(&row, 151, 42, "SAMPLE A", 40, INK);
  label(&row, 151, 89, "ID: A-01", 22, MUTED);
  sprite(&row, SPRITE_DATA, 470, 48, 49, 64);
  label(&row, 533, 45, "Data", 24, MUTED);
  label(&row, 533, 76, "2", 34, INK);
  sprite(&row, SPRITE_ENERGY, 636, 46, 58, 69);
  label(&row, 710, 45, "Energy", 24, MUTED);
  label(&row, 710, 76, "1", 34, INK);
  sprite(&row, SPRITE_ESSENCE, 802, 46, 68, 71);
  label(&row, 880, 45, "Essence", 22, MUTED);
  label(&row, 880, 76, "0", 34, INK);

  panel(&row, 40, 153, 280, 202);
  label(&row, 58, 164, "KNOWN FINDING: CROWN", 18, MUTED);
  rectangle(&row, 58, 199, 244, 137, DEEP);
  outline(&row, 58, 199, 244, 137, 2, EDGE);
  sprite(&row, SPRITE_CROWN, 105, 203, 154, 130);
  panel(&row, 40, 374, 280, 202);
  label(&row, 58, 385, "KNOWN FINDING: EYE-RING", 18, MUTED);
  rectangle(&row, 58, 420, 244, 137, DEEP);
  outline(&row, 58, 420, 244, 137, 2, EDGE);
  sprite(&row, SPRITE_EYE_RING, 128, 435, 111, 100);

  panel(&row, 344, 153, 643, 423);
  const char *title = lab->page == SELECTED_STUDY ? "MARKINGS STUDY" : lab->page == SELECTED_START_PREVIEW ? "STUDY PREVIEW" : lab->page == SELECTED_CROWN ? "CROWN REFERENCE" : "EYE-RING REFERENCE";
  assert(native_text_width(font(36), title) <= 596);
  label(&row, 367, 169, title, 36, INK);
  rectangle(&row, 366, 224, 599, 2, EDGE);
  rectangle(&row, 366, 437, 599, 2, EDGE);
  if (lab->page == SELECTED_STUDY) {
    sprite(&row, SPRITE_UNKNOWN, 383, 245, 250, 161);
    rectangle(&row, 656, 245, 2, 162, EDGE);
    label(&row, 677, 251, "Pale inheritance:", 22, MUTED);
    label(&row, 677, 281, "KNOWN", 24, SAGE);
    label(&row, 677, 323, "Adult conditions: MILD", 21, MUTED);
    label(&row, 677, 365, "Appearance: UNKNOWN", 21, SAGE);
  } else if (lab->page == SELECTED_START_PREVIEW) {
    sprite(&row, SPRITE_UNKNOWN, 383, 245, 250, 161);
    label(&row, 677, 250, "Native preview", 28, INK);
    label(&row, 677, 298, "Study not connected.", 22, MUTED);
    label(&row, 677, 342, "No Data spent.", 22, MUTED);
    label(&row, 677, 383, "Appearance unknown.", 21, MUTED);
  } else {
    sprite(&row, lab->page == SELECTED_CROWN ? SPRITE_CROWN : SPRITE_EYE_RING, 408, 250, 186, 160);
    label(&row, 677, 252, "Known finding", 28, INK);
    label(&row, 677, 300, "Sample A", 24, MUTED);
    label(&row, 677, 346, "Feature reference art.", 21, MUTED);
    label(&row, 677, 386, "No specimen portrait.", 21, MUTED);
  }

  rectangle(&row, 367, 466, 373, 82, DEEP);
  outline(&row, 367, 466, 373, 82, 2, EDGE);
  rectangle(&row, 594, 474, 2, 66, EDGE);
  label(&row, 380, 492, lab->page == SELECTED_STUDY ? "COST: 2 DATA" : "FREE PREVIEW", 22, MUTED);
  label(&row, 613, 477, "AVAILABLE:", 18, MUTED);
  label(&row, 613, 505, "2 DATA", 24, INK);
  stepped(&row, 775, 466, 184, 85, 20, DEEP);
  stepped(&row, 779, 470, 176, 77, 16, ACTION);
  outline(&row, 790, 479, 154, 59, 2, MUTED);
  const char *action = lab->page == SELECTED_STUDY ? "START" : "RETURN";
  label(&row, 867 - native_text_width(font(32), action) / 2, 490, action, 32, INK);
  if (lab->focus == SELECTED_START || lab->focus == SELECTED_RETURN) focus(&row, 775, 466, 184, 85);
  else if (lab->focus == SELECTED_CROWN_TARGET) focus(&row, 41, 154, 278, 200);
  else focus(&row, 41, 375, 278, 200);
}

static int word(FILE *output, unsigned value, unsigned bytes) {
  for (unsigned index = 0; index < bytes; ++index)
    if (fputc((int)((value >> (index * 8)) & 255u), output) == EOF) return 0;
  return 1;
}

int selected_lab_bmp(const SelectedLab *lab, FILE *output) {
  const unsigned stride = SELECTED_LAB_WIDTH * 3;
  uint8_t pixels[SELECTED_LAB_WIDTH * 3];
  if (fwrite("BM", 1, 2, output) != 2) return 0;
  unsigned fields[][2] = {{54 + stride * SELECTED_LAB_HEIGHT, 4}, {0, 4}, {54, 4}, {40, 4}, {SELECTED_LAB_WIDTH, 4}, {SELECTED_LAB_HEIGHT, 4}, {1, 2}, {24, 2}, {0, 4}, {stride * SELECTED_LAB_HEIGHT, 4}, {2835, 4}, {2835, 4}, {0, 4}, {0, 4}};
  for (unsigned index = 0; index < sizeof(fields) / sizeof(fields[0]); ++index)
    if (!word(output, fields[index][0], fields[index][1])) return 0;
  for (unsigned y = SELECTED_LAB_HEIGHT; y > 0; --y) {
    selected_lab_row(lab, y - 1, pixels);
    for (unsigned column = 0; column < SELECTED_LAB_WIDTH; ++column) {
      uint8_t swap = pixels[column * 3];
      pixels[column * 3] = pixels[column * 3 + 2];
      pixels[column * 3 + 2] = swap;
    }
    if (fwrite(pixels, 1, stride, output) != stride) return 0;
  }
  return !ferror(output);
}
