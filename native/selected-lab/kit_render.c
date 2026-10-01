#include "home_view.h"
#include "core_art.h"
#include "expedition_render.h"
#include "kit.h"
#include "native_font.h"
#include "native_ui.h"
#include <string.h>

typedef struct {
  unsigned y, width;
  uint8_t *pixels;
} KitRow;
enum {
  BACKGROUND,
  TEXT,
  SECONDARY,
  BORDER,
  FOCUS,
  FIELD,
  DEEP,
  EDGE,
  PANEL,
  GLOW
};
static const uint8_t palette[][3] = {
    {25, 36, 43},    {214, 222, 226}, {183, 198, 205}, {29, 119, 191},
    {237, 197, 106}, {42, 51, 56},    {10, 17, 23},    {56, 100, 132},
    {29, 38, 45},    {77, 70, 48}};
static void fill(KitRow *row, int x, int y, int width, int height,
                 unsigned color) {
  if ((int)row->y < y || (int)row->y >= y + height)
    return;
  for (int at = x; at < x + width; ++at)
    if (at >= 0 && at < (int)row->width) {
      memcpy(row->pixels + at * 3,
             palette[color], 3);
    }
}
static void border(KitRow *row, int x, int y, int width, int height,
                   unsigned color) {
  fill(row, x, y, width, 2, color);
  fill(row, x, y + height - 2, width, 2, color);
  fill(row, x, y, 2, height, color);
  fill(row, x + width - 2, y, 2, height, color);
}
static void text(KitRow *row, int x, int y, const char *value, unsigned size,
                 unsigned color) {
  const NativeFont *font = &lab_fonts[0];
  for (unsigned i = 0; i < LAB_FONT_COUNT; ++i)
    if ((unsigned)lab_fonts[i].size == size)
      font = &lab_fonts[i];
  native_text_row(font, value, x, y, row->y, row->width, row->pixels, 0,
                  palette[color]);
}
static void heading(KitRow *row, int x, int y, const char *value,
                    unsigned size, unsigned color) {
  const NativeFont *selected = NULL;
  for (unsigned i = 0; i < LAB_HEADING_FONT_COUNT; ++i)
    if ((unsigned)lab_heading_narrow_fonts[i].size == size)
      selected = &lab_heading_narrow_fonts[i];
  if (!selected)
    return;
  native_text_row(selected, value, x, y, row->y, row->width, row->pixels, 0,
                  palette[color]);
}
static void resource(KitRow *row, unsigned icon, int x, int y, int primary) {
  CoreArtId id = (CoreArtId)((primary ? CORE_ART_DATA_PRIMARY
                                    : CORE_ART_DATA_COMPACT) + icon);
  core_art_row(id, x, y, row->y, row->width, row->pixels);
}
static void panel(KitRow *row, int x, int y, int width, int height) {
  core_art_panel_row(x, y, width, height, row->y, row->width, row->pixels);
}
static void action_focus(KitRow *row, int x, int y, int width, int height) {
  core_art_focus_row(x, y, width, height, row->y, row->width, row->pixels);
}
static void wrapped(KitRow *row, int x, int y, const char *value, int width,
                    unsigned size, unsigned color) {
  char line[128] = {0};
  size_t used = 0;
  while (*value) {
    const char *end = strchr(value, ' ');
    size_t length = end ? (size_t)(end - value) : strlen(value);
    char candidate[128];
    snprintf(candidate, sizeof(candidate), "%s%.*s", line, (int)length, value);
    const NativeFont *font = &lab_fonts[0];
    for (unsigned i = 0; i < LAB_FONT_COUNT; ++i)
      if ((unsigned)lab_fonts[i].size == size)
        font = &lab_fonts[i];
    if (used && native_text_width(font, candidate) > width) {
      text(row, x, y, line, size, color);
      y += (int)size + 4;
      used = 0;
    }
    if (used + length + 2 >= sizeof(line))
      break;
    memcpy(line + used, value, length);
    used += length;
    line[used++] = ' ';
    line[used] = 0;
    value += length;
    if (*value == ' ')
      ++value;
  }
  if (used)
    text(row, x, y, line, size, color);
}
static void lab_explore_row(const DeviceKit *kit, KitRow *row) {
  const GameState *game = &kit->lab->game;
  unsigned phase = kit->journal.phase;
  if (phase != KIT_ARRIVED && phase != KIT_COMMITTING &&
      !(kit->caller_valid && (phase == KIT_ACK_PENDING || phase == KIT_COMPLETE))) {
    ExpeditionReceivedView received = {0};
    kit_received_projection(kit, kit->received_selected, &received);
    expedition_received_row(&received, row->y, row->pixels);
    if (kit->failed) {
      fill(row, 24, 544, 976, 38, FIELD);
      text(row, 36, 552, "Storage unavailable. Received records preserved.", 18, FOCUS);
    }
    return;
  }
  ExpeditionFieldView field;
  int map_outing = kit_field_projection(kit, &field);
  char value[128];
  fill(row, 0, 0, 1024, 600, BACKGROUND);
  if (row->y < 112) {
    selected_lab_row(kit->lab, row->y, row->pixels);
    return;
  }
  int manifest = phase >= KIT_ARRIVED;
  int accepted = map_outing ? field.delivery_accepted : phase >= KIT_ACK_PENDING;
  if (manifest) {
    /*07's ownership comparison: incoming is never rendered as accepted stock. */
    panel(row, 32, 142, 302, 386);
    panel(row, 686, 142, 306, 386);
    heading(row, 50, 157, "FROM COMPANION", 26, TEXT);
    heading(row, 710, 157, "LAB STOCK", 26, TEXT);
    static const char *names[] = {"Data", "Energy", "Essence"};
    const unsigned stock[] = {game->data, game->energy, game->essence};
    for (unsigned i = 0; i < 3; ++i) {
      int y = 204 + (int)i * 101;
      resource(row, i, 49, y, 1);
      text(row, 161, y + 54, names[i], 22, SECONDARY);
      snprintf(value, sizeof(value), "%u", kit->journal.cargo[i] / GAME_SUPPLY_UNIT);
      heading(row, 161, y + 8, value, 32, TEXT);
      resource(row, i, 700, y, 1);
      text(row, 812, y + 54, names[i], 22, SECONDARY);
      snprintf(value, sizeof(value), "%u", stock[i] / GAME_SUPPLY_UNIT);
      heading(row, 812, y + 8, value, 32, TEXT);
    }
    wrapped(row, 365, 204, accepted ? "Supplies stored at the Lab"
                                      : "Supplies waiting at the Lab",
            286, 26, TEXT);
    wrapped(row, 365, 400, accepted ? "The haul is included in Lab stock."
                                     : "Store haul / End expedition",
            286, 18, SECONDARY);
    const GameSample *sample = kit_received_sample(kit);
    if (accepted && sample)
      snprintf(value, sizeof(value), "Sample recorded: %s", sample->id);
    else if (accepted)
      snprintf(value, sizeof(value), "Supplies saved / no sample recorded");
    else if (map_outing)
      snprintf(value, sizeof(value), "%s",
               field.capsule_count ? "Sealed sample waiting / contents unknown"
                                   : "Supplies only / no sample collected");
    else
      snprintf(value, sizeof(value), "%s",
               kit->journal.elapsed < GAME_EXPEDITION_SECONDS ? "Supplies only / no sample"
               : game->sample_count < GAME_MAX_SAMPLES ? "Sample ready to record"
                                                       : "Sample shelf full / supplies only");
    text(row, 48, 537, value, 18, SECONDARY);
  }
  if (kit->failed) {
    wrapped(row, manifest ? 365 : 48, manifest ? 314 : 509,
            accepted ? "Delivery committed. Receipt recovery needed."
                     : "Storage unavailable. Cargo preserved.",
            manifest ? 286 : 920, manifest ? 18 : 22, FOCUS);
  } else if (phase == KIT_ARRIVED) {
    fill(row, 371, 312, 282, 60, FIELD);
    action_focus(row, 376, 316, 272, 52);
    heading(row, 391, 326, "Confirm: accept haul", 26, FOCUS);
  } else {
    wrapped(row, manifest ? 365 : 48, manifest ? 314 : 509,
         phase == KIT_ACK_PENDING        ? "Waiting for Companion receipt"
         : phase == KIT_COMPLETE         ? "Companion receipt confirmed"
         : kit->journal.companion_online ? "Lab link available (simulation)"
                                         : "Companion offline",
         manifest ? 286 : 920, manifest ? 18 : 22, SECONDARY);
  }
  text(row, 48, 565,
       kit->caller_valid ? "Back: return to your previous screen"
                         : "Back: Lab overview",
       18, SECONDARY);
}
static void render_row(const DeviceKit *kit, unsigned device, unsigned y,
                       uint8_t *pixels) {
  if (device == KIT_LAB && !kit_lab_explore(kit)) {
    SelectedLabRenderContext context = {SELECTED_HAUL_NONE, {0, 0, 0}};
    if (kit->journal.phase == KIT_ARRIVED || kit->journal.phase == KIT_COMMITTING)
      context.haul = SELECTED_HAUL_WAITING;
    else if (kit->journal.phase == KIT_ACK_PENDING || kit->journal.phase == KIT_COMPLETE)
      context.haul = SELECTED_HAUL_STORED;
    memcpy(context.incoming, kit->journal.cargo, sizeof(context.incoming));
    selected_lab_row_with_context(kit->lab, &context, y, pixels);
    if (kit->normalization_pending) {
      KitRow row = {y, kit_width(device), pixels};
      fill(&row, 24, 548, 976, 38, FIELD);
      border(&row, 24, 548, 976, 38, FOCUS);
      text(&row, 36, 557,
           "Accept the existing haul before supply conversion can finish.", 18,
           TEXT);
    }
    return;
  }
  KitRow row = {y, kit_width(device), pixels};
  if (device == KIT_LAB) lab_explore_row(kit, &row);

}
static int word(FILE *output, unsigned value, unsigned bytes) {
  for (unsigned i = 0; i < bytes; ++i)
    if (fputc((int)((value >> (i * 8)) & 255u), output) == EOF)
      return 0;
  return 1;
}
static int bmp_rows(const DeviceKit *kit, unsigned device, FILE *output,
                    const uint8_t *frame) {
  if (device == KIT_DOCK && !frame) return 0;
  unsigned width = kit_width(device), height = kit_height(device),
           stride = (width * 3 + 3) & ~3u;
  uint8_t pixels[SELECTED_LAB_WIDTH * 3];
  if (fwrite("BM", 1, 2, output) != 2)
    return 0;
  unsigned fields[][2] = {{54 + stride * height, 4},
                          {0, 4},
                          {54, 4},
                          {40, 4},
                          {width, 4},
                          {height, 4},
                          {1, 2},
                          {24, 2},
                          {0, 4},
                          {stride * height, 4},
                          {2835, 4},
                          {2835, 4},
                          {0, 4},
                          {0, 4}};
  for (unsigned i = 0; i < sizeof(fields) / sizeof(fields[0]); ++i)
    if (!word(output, fields[i][0], fields[i][1]))
      return 0;
  for (unsigned y = height; y > 0; --y) {
    memset(pixels, 0, sizeof(pixels));
    if (frame)
      memcpy(pixels, frame + (y - 1) * width * 3, width * 3);
    else
      render_row(kit, device, y - 1, pixels);
    for (unsigned x = 0; x < width; ++x) {
      uint8_t swap = pixels[x * 3];
      pixels[x * 3] = pixels[x * 3 + 2];
      pixels[x * 3 + 2] = swap;
    }
    if (fwrite(pixels, 1, stride, output) != stride)
      return 0;
  }
  return !ferror(output);
}

int kit_bmp_ui(const DeviceKit *kit, unsigned device, FILE *output,
               NativeUiContext *context, int still) {
  if (!kit || !kit->lab || !output || device >= KIT_DEVICE_COUNT) return 0;
  if (device == KIT_LAB && kit->lab->page == V1_HOME) {
    SelectedLabRenderContext facts = {SELECTED_HAUL_NONE, {0,0,0}};
    if (kit->journal.phase == KIT_ARRIVED || kit->journal.phase == KIT_COMMITTING)
      facts.haul = SELECTED_HAUL_WAITING;
    else if (kit->journal.phase == KIT_ACK_PENDING || kit->journal.phase == KIT_COMPLETE)
      facts.haul = SELECTED_HAUL_STORED;
    memcpy(facts.incoming, kit->journal.cargo, sizeof(facts.incoming));
    LabHomeView view;
    if (!selected_lab_home_view(kit->lab, &facts, kit->normalization_pending, &view)) return 0;
    if (kit->failed) snprintf(view.warning, sizeof(view.warning), "%s", kit->lab->message);
    int temporary = !context;
    if (temporary) context = native_ui_create_device(KIT_LAB);
    if (!context) return 0;
    const uint8_t *frame = native_ui_home(context, &view);
    int result = frame && bmp_rows(kit, device, output, frame);
    if (temporary) native_ui_destroy(context);
    return result;
  }
  if (device == KIT_DOCK) {
    DockView view;
    if (!kit_dock_projection(kit, &view)) return 0;
    int temporary = !context;
    if (temporary) context = native_ui_create_device(KIT_DOCK);
    if (!context) return 0;
    const uint8_t *frame = native_ui_dock(context, &view);
    int result = frame && bmp_rows(kit, device, output, frame);
    if (temporary) native_ui_destroy(context);
    return result;
  }
  if (device == KIT_COMPANION &&
      ((kit->companion.page == COMP_MODES && kit->companion.mode == COMP_FRIENDS) ||
       kit->companion.page == COMP_FRIENDS || kit->companion.page == COMP_FRIEND_VISIT)) {
    CompanionResidentView resident;
    if (!kit_resident_projection(kit, &resident)) return 0;
    int temporary = !context;
    if (temporary) context = native_ui_create();
    if (!context) return 0;
    const uint8_t *frame = native_ui_resident(context, &resident);
    int result = frame && bmp_rows(kit, device, output, frame);
    if (temporary) native_ui_destroy(context);
    return result;
  }
  CompanionProbeView probe;
  if (device == KIT_COMPANION && kit_probe_projection(kit, &probe)) {
    int temporary = !context;
    if (temporary) context = native_ui_create();
    if (!context) return 0;
    const uint8_t *frame = native_ui_probe(context, &probe);
    int result = frame && bmp_rows(kit, device, output, frame);
    if (temporary) native_ui_destroy(context);
    return result;
  }
  if (device == KIT_COMPANION && (kit->companion.page == COMP_PROBE ||
      kit->companion.page == COMP_FIELD_SITE ||
      (kit->companion.page == COMP_MODES && kit->companion.mode == COMP_PROBE))) return 0;
  CompanionCargoView view;
  if (device != KIT_COMPANION || !kit_cargo_projection(kit, &view)) {
    if (device == KIT_COMPANION) return 0;
    /* Other-device frame requests must not cancel Companion presentation. */
    return bmp_rows(kit, device, output, NULL);
  }
  int temporary = !context;
  if (temporary) context = native_ui_create();
  if (!context) return 0;
  const uint8_t *frame = native_ui_cargo(context, &view, still);
  int result = frame && bmp_rows(kit, device, output, frame);
  if (temporary) native_ui_destroy(context);
  return result;
}
int kit_bmp(const DeviceKit *kit, unsigned device, FILE *output) {
  return kit_bmp_ui(kit, device, output, NULL, 1);
}
