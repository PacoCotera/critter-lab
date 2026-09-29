#include "kit.h"
#include "native_font.h"
#include "overview_assets.h"
#include <string.h>
#include <time.h>

typedef struct {
  unsigned y, width;
  uint8_t *pixels;
  int mono;
} KitRow;
enum { BACKGROUND, TEXT, SECONDARY, BORDER, FOCUS, FIELD };
static const uint8_t palette[][3] = {{25, 36, 43},    {214, 222, 226},
                                     {183, 198, 205}, {29, 119, 191},
                                     {237, 197, 106}, {42, 51, 56}};
static void fill(KitRow *row, int x, int y, int width, int height,
                 unsigned color) {
  if ((int)row->y < y || (int)row->y >= y + height)
    return;
  for (int at = x; at < x + width; ++at)
    if (at >= 0 && at < (int)row->width) {
      if (row->mono)
        memset(row->pixels + at * 3,
               color == BACKGROUND || color == FIELD ? 255 : 0, 3);
      else
        memcpy(row->pixels + at * 3, palette[color], 3);
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
  const uint8_t black[] = {0, 0, 0};
  native_text_row(font, value, x, y, row->y, row->width, row->pixels, 0,
                  row->mono ? black : palette[color]);
}
static void art(KitRow *row, unsigned icon, int x, int y) {
  int at = (int)row->y - y;
  if (at < 0 || at >= OVERVIEW_SPRITE_HEIGHT)
    return;
  for (int col = 0; col < OVERVIEW_SPRITE_WIDTH; ++col) {
    if (x + col < 0 || x + col >= (int)row->width)
      continue;
    const uint8_t *pixel =
        overview_pixels[icon] + (at * OVERVIEW_SPRITE_WIDTH + col) * 4;
    memcpy(row->pixels + (x + col) * 3, pixel, 3);
  }
}
static void units(char *buffer, size_t size, uint32_t value) {
  snprintf(buffer, size, "%u units + %u%%", value / 100, value % 100);
}
static void companion_row(const DeviceKit *kit, KitRow *row) {
  const GameState *game = &kit->lab->game;
  const KitView *view = &kit->companion;
  char value[96];
  fill(row, 0, 0, 450, 600, BACKGROUND);
  border(row, 12, 12, 426, 576, BORDER);
  text(row, 28, 27, "BEECHO / COMPANION", 22, TEXT);
  const char *title = view->page == COMP_PROBE         ? "Probe"
                      : view->page == COMP_CARGO       ? "Cargo"
                      : view->page == COMP_SEND_REVIEW ? "Send haul?"
                      : view->page == COMP_FRIENDS     ? "Companions"
                                                       : "Choose a mode";
  text(row, 28, 69, title, 34, TEXT);
  fill(row, 24, 116, 402, 153, FIELD);
  unsigned icon = view->page == COMP_FRIENDS ? OVERVIEW_HABITAT
                  : view->page == COMP_CARGO || view->page == COMP_SEND_REVIEW
                      ? OVERVIEW_INCUBATOR
                      : OVERVIEW_EXPLORE;
  art(row, icon, 28, 121);
  int pending = kit->journal.phase >= KIT_WAITING &&
                kit->journal.phase <= KIT_ACK_PENDING;
  uint32_t cargo[] = {game->expedition_data, game->expedition_energy,
                      game->expedition_essence};
  if (pending)
    memcpy(cargo, kit->journal.cargo, sizeof(cargo));
  if (view->page == COMP_FRIENDS) {
    text(row, 184, 141, "Travel party", 22, TEXT);
    text(row, 184, 184, "Not assigned", 22, SECONDARY);
    text(row, 184, 220, "Visit the Lab", 18, SECONDARY);
  } else if (view->page == COMP_CARGO || view->page == COMP_SEND_REVIEW) {
    static const char *labels[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      snprintf(value, sizeof(value), "%s: %u + %u%%", labels[i], cargo[i] / 100,
               cargo[i] % 100);
      text(row, 181, 137 + (int)i * 40, value, 18, TEXT);
    }
  } else {
    text(row, 184, 140,
         pending                   ? "Haul sealed"
         : game->expedition_active ? "Gathering"
         : game->expedition_id[0]  ? "Haul ready"
                                   : "Ready to explore",
         22, TEXT);
    snprintf(value, sizeof(value), "%u / 60 seconds",
             pending ? kit->journal.elapsed : game->expedition_elapsed);
    text(row, 184, 184, value, 18, SECONDARY);
    snprintf(value, sizeof(value), "Carried: %u units",
             (cargo[0] + cargo[1] + cargo[2]) / 100);
    text(row, 184, 220, value, 18, SECONDARY);
  }
  text(row, 28, 286,
       kit->journal.companion_online ? "Wireless link: available (sim)"
                                     : "Wireless link: offline (sim)",
       18, SECONDARY);
  text(row, 28, 317, kit_stage(kit), 22, pending ? FOCUS : TEXT);
  unsigned count =
      view->page == COMP_MODES                                          ? 3
      : view->page == COMP_FRIENDS                                      ? 1
      : view->page == COMP_PROBE && !game->expedition_id[0] && !pending ? 3
                                                                        : 2;
  for (unsigned i = 0; i < count; ++i) {
    int y = 364 + (int)i * 43;
    if (view->focus == i) {
      border(row, 25, y - 6, 400, 38, FOCUS);
      text(row, 33, y, ">", 22, FOCUS);
    }
    text(row, 58, y, kit_option(kit, KIT_COMPANION, i), 22, TEXT);
  }
  const char *hint =
      view->page == COMP_SEND_REVIEW ? "Seals this haul. Lab must accept it."
      : view->page == COMP_FRIENDS   ? "Party assignment is not simulated yet."
                                     : "Back: modes   Confirm: selected action";
  text(row, 28, 514, hint, 18, SECONDARY);
  const char *message =
      kit->failed ? "Storage unavailable. Reload to recover." : view->message;
  char short_message[46];
  snprintf(short_message, sizeof(short_message), "%.44s", message);
  text(row, 28, 550, short_message, 18, FOCUS);
}
static void dock_row(const DeviceKit *kit, KitRow *row) {
  const KitView *view = &kit->dock;
  const KitJournal *journal = &kit->journal;
  char value[96];
  fill(row, 0, 0, 792, 272, BACKGROUND);
  border(row, 8, 8, 776, 256, TEXT);
  text(row, 24, 23, "BEECHO LAB / DOCK", 26, TEXT);
  text(row, 485, 28,
       journal->dock_online ? "Synced (simulation)" : "Offline / cached", 22,
       TEXT);
  fill(row, 24, 66, 744, 2, TEXT);
  if (view->page == 2) {
    text(row, 24, 89, "World summary print preview", 26, TEXT);
    text(row, 24, 133, "No physical printer or paper output is connected.", 22,
         TEXT);
  } else if (view->focus == 0) {
    snprintf(value, sizeof(value), "%u residents", journal->dock_residents);
    text(row, 24, 88, value, 32, TEXT);
    snprintf(value, sizeof(value), "%u samples", journal->dock_samples);
    text(row, 284, 88, value, 32, TEXT);
    snprintf(value, sizeof(value), "%u incubating", journal->dock_incubations);
    text(row, 530, 88, value, 32, TEXT);
    text(row, 24, 143, "Shared world summary / charging unavailable", 22, TEXT);
  } else if (view->focus == 1) {
    static const char *labels[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      text(row, 24 + (int)i * 248, 85, labels[i], 22, TEXT);
      units(value, sizeof(value), journal->dock_stock[i]);
      text(row, 24 + (int)i * 248, 122, value, 22, TEXT);
    }
  } else {
    text(row, 24, 88,
         journal->dock_online ? "Lab link available (simulated)"
                              : "Lab link unavailable; last snapshot retained",
         22, TEXT);
    text(row, 24, 122, "Cloud: not connected   Charging: not measured", 22,
         TEXT);
    text(row, 24, 155, "Radio protocol: unselected", 18, TEXT);
  }
  time_t local_stamp = (time_t)journal->dock_updated_at - 6 * 3600;
  struct tm *local_time = gmtime(&local_stamp);
  char stamp[32] = "unknown";
  if (journal->dock_updated_at && local_time)
    strftime(stamp, sizeof(stamp), "%H:%M:%S Mexico City", local_time);
  snprintf(value, sizeof(value), "%sSnapshot %s%s",
           view->page == 1 ? "OK: Back / " : "", stamp,
           journal->dock_online ? "" : " / stale");
  text(row, 24, 185, view->message[0] ? view->message : value, 18, TEXT);
  unsigned count = view->page == 2 ? 2 : 3;
  for (unsigned i = 0; i < count; ++i) {
    int x = 24 + (int)i * (view->page == 2 ? 450 : 248);
    if (view->focus == i)
      border(row, x - 4, 222, view->page == 2 && i == 0 ? 420 : 228, 32, TEXT);
    text(row, x + 8, 227, kit_option(kit, KIT_DOCK, i), 18, TEXT);
  }
}
static void lab_explore_row(const DeviceKit *kit, KitRow *row) {
  char value[96];
  fill(row, 0, 0, 1024, 600, BACKGROUND);
  border(row, 24, 24, 976, 552, BORDER);
  text(row, 48, 46, "COMPANION / WIRELESS HAUL", 34, TEXT);
  text(row, 48, 99,
       "Explore at the Companion. Research accepted findings here.", 22,
       SECONDARY);
  fill(row, 40, 153, 944, 204, FIELD);
  art(row, OVERVIEW_EXPLORE, 64, 180);
  int has_manifest = kit->journal.phase >= KIT_ARRIVED;
  text(row, 239, 166, has_manifest ? "Companion haul" : "No incoming haul", 32,
       TEXT);
  text(row, 239, 216,
       has_manifest ? kit->journal.haul_id : "Choose a route on the Companion",
       22, SECONDARY);
  if (has_manifest) {
    static const char *labels[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      snprintf(value, sizeof(value), "%s  %u + %u%%", labels[i],
               kit->journal.cargo[i] / 100, kit->journal.cargo[i] % 100);
      text(row, 239 + (int)i * 234, 278, value, 22, TEXT);
    }
    snprintf(value, sizeof(value), "Sample eligibility: %s",
             kit->journal.elapsed >= 60 ? "expedition complete"
                                        : "early return / resources only");
    text(row, 239, 318, value, 18, SECONDARY);
  }
  text(row, 48, 389, kit_stage(kit), 26, TEXT);
  text(row, 48, 434,
       kit->journal.companion_online
           ? "Companion link available (simulation)"
           : "Companion link offline / receipt waits here",
       22, SECONDARY);
  if (kit->journal.phase == KIT_ARRIVED) {
    border(row, 46, 491, 554, 45, FOCUS);
    text(row, 64, 501, "Confirm: accept this haul into Lab stock", 22, FOCUS);
  } else
    text(row, 48, 503, "Back: Lab overview", 22, SECONDARY);
  if (kit->failed)
    text(row, 48, 546, "Storage unavailable. No further transfer accepted.", 18,
         FOCUS);
}
static void render_row(const DeviceKit *kit, unsigned device, unsigned y,
                       uint8_t *pixels) {
  if (device == KIT_LAB && !kit_lab_explore(kit)) {
    selected_lab_row(kit->lab, y, pixels);
    return;
  }
  KitRow row = {y, kit_width(device), pixels, device == KIT_DOCK};
  if (device == KIT_COMPANION)
    companion_row(kit, &row);
  else if (device == KIT_DOCK)
    dock_row(kit, &row);
  else
    lab_explore_row(kit, &row);
  if (device == KIT_DOCK)
    for (unsigned x = 0; x < row.width; ++x) {
      uint8_t value = pixels[x * 3] >= 128 ? 255 : 0;
      memset(pixels + x * 3, value, 3);
    }
}
static int word(FILE *output, unsigned value, unsigned bytes) {
  for (unsigned i = 0; i < bytes; ++i)
    if (fputc((int)((value >> (i * 8)) & 255u), output) == EOF)
      return 0;
  return 1;
}
int kit_bmp(const DeviceKit *kit, unsigned device, FILE *output) {
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
