#include "assets.h"
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
static void resource(KitRow *row, unsigned icon, int x, int y, unsigned scale) {
  const SelectedSprite *source = &selected_sprites[icon];
  /* Integer source scale preserves the approved pixels across device views. */
  selected_lab_sprite_row(icon, x, y, source->width * scale,
                          source->height * scale, row->y, row->pixels);
}
static void panel(KitRow *row, int x, int y, int width, int height) {
  fill(row, x + 4, y + 6, width, height, DEEP);
  fill(row, x, y, width, height, PANEL);
  border(row, x, y, width, height, BORDER);
  fill(row, x + 8, y + 4, width - 16, 2, EDGE);
  fill(row, x + 4, y + 10, 2, height - 20, EDGE);
}
static void progress(KitRow *row, int x, int y, int width, int height,
                     unsigned amount, unsigned total) {
  fill(row, x, y, width, height, DEEP);
  unsigned filled = total ? (unsigned)width * amount / total : 0;
  if (filled > (unsigned)width)
    filled = (unsigned)width;
  fill(row, x, y, (int)filled, height, BORDER);
  fill(row, x, y, (int)filled, 2, EDGE);
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
static void resource_names(char *value, size_t capacity, const char *prefix,
                           unsigned mask) {
  static const char *names[] = {"Data", "Energy", "Essence"};
  snprintf(value, capacity, "%s", prefix);
  int first = 1;
  for (unsigned i = 0; i < 3; ++i) {
    if (!(mask & (1u << i)))
      continue;
    size_t used = strlen(value);
    snprintf(value + used, capacity - used, "%s%s", first ? "" : " / ",
             names[i]);
    first = 0;
  }
}
static void companion_row(const DeviceKit *kit, KitRow *row) {
  const GameState *game = &kit->lab->game;
  const KitView *view = &kit->companion;
  unsigned page = view->page == COMP_MODES ? view->mode : view->page;
  int selector = view->page == COMP_MODES;
  int reserved =
      kit->journal.phase >= KIT_WAITING && kit->journal.phase <= KIT_COMMITTING;
  int receipt = kit->journal.phase == KIT_ACK_PENDING;
  int details = page == COMP_CARGO || page == COMP_SEND_REVIEW;
  int review = page == COMP_SEND_REVIEW;
  char value[128];
  uint32_t cargo[] = {game->expedition_data, game->expedition_energy,
                      game->expedition_essence};
  if (reserved)
    memcpy(cargo, kit->journal.cargo, sizeof(cargo));
  unsigned total = cargo[0] + cargo[1] + cargo[2];
  int has_run = reserved || game->expedition_id[0];
  unsigned elapsed = reserved ? kit->journal.elapsed : game->expedition_elapsed;
  fill(row, 0, 0, 450, 600, BACKGROUND);
  panel(row, 12, 12, 426, 576);
  text(row, 28, 26, "BEECHO / COMPANION", 18, SECONDARY);
  text(row, 28, 55,
       details                ? "CARGO"
       : page == COMP_FRIENDS ? "COMPANIONS"
                              : "PROBE",
       26, TEXT);
  static const char *modes[] = {"Probe", "Cargo", "Companions"};
  for (unsigned i = 0; i < 3; ++i) {
    int x = 28 + (int)i * 132;
    if (view->mode == i) {
      fill(row, x - 4, 84, 128, 25, FIELD);
      fill(row, x, 109, 118, 3, BORDER);
      if (selector) {
        int pressed = 0;
        for (unsigned button = 0; button < 9; ++button)
          pressed |=
              view->gestures[button].held && view->gestures[button].allowed;
        if (pressed)
          fill(row, x - 3, 83, 126, 27, GLOW);
        border(row, x - 4, 82, 128, 30, pressed ? FOCUS : EDGE);
      }
    }
    text(row, x + 4, 88, modes[i], 18, view->mode == i ? TEXT : SECONDARY);
  }
  fill(row, 28, 112, 394, 2, EDGE);
  int action_top = 502;
  if (page == COMP_FRIENDS) {
    fill(row, 28, 126, 394, 172, FIELD);
    art(row, OVERVIEW_HABITAT, 40, 140);
    text(row, 191, 159, "Travel party", 22, TEXT);
    text(row, 191, 198, "Not assigned", 18, SECONDARY);
    wrapped(row, 28, 318, "Party assignment is not simulated yet.", 390, 18,
            SECONDARY);
  } else if (details) {
    text(row, 28, 125,
         review     ? "To Lab"
         : reserved ? "Reserved for transfer"
                    : "Collected items",
         22, TEXT);
    if (has_run) {
      text(row, 28, 158, kit_route(kit), 18, SECONDARY);
      text(row, 28, 182, kit_expedition_status(kit), 18, SECONDARY);
      snprintf(value, sizeof(value), "%u / %u sec", elapsed,
               GAME_EXPEDITION_SECONDS);
      text(row, 300, 158, value, 18, SECONDARY);
      progress(row, 228, 187, 186, 8, elapsed, GAME_EXPEDITION_SECONDS);
    }
    static const char *names[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      int y = (review ? 240 : 224) + (int)i * 62;
      const SelectedSprite *asset = &selected_sprites[i];
      resource(row, i, 37 + (54 - (int)asset->width) / 2, y, 1);
      text(row, 111, y + 13, names[i], 22, TEXT);
      snprintf(value, sizeof(value), "%u", cargo[i] / GAME_SUPPLY_UNIT);
      text(row, 337, y + 11, value, 26, TEXT);
    }
    if (review) {
      text(row, 28, 429,
           elapsed < GAME_EXPEDITION_SECONDS ? "Supplies only"
           : game->sample_count < GAME_MAX_SAMPLES
               ? "Sample ready to record"
               : "Sample shelf full / supplies only",
           18, SECONDARY);
      if (has_run && elapsed < GAME_EXPEDITION_SECONDS)
        text(row, 28, 456, "Gathering progress stays on Companion", 18,
             SECONDARY);
    } else {
      snprintf(value, sizeof(value), "Free space: %u / 40 units",
               (GAME_CARGO_CAPACITY - total) / GAME_SUPPLY_UNIT);
      text(row, 28, 412, value, 18, SECONDARY);
      if (reserved || receipt || kit->journal.phase == KIT_COMPLETE)
        wrapped(row, 28, 440, kit_stage(kit), 390, 18, SECONDARY);
      else if (!game_transfer_available(game))
        text(row, 28, 440, "No items to send", 18, SECONDARY);
      else
        text(row, 28, 440,
             kit->journal.companion_online ? "Lab link available"
                                           : "Lab offline",
             18, SECONDARY);
    }
  } else {
    fill(row, 28, 125, 394, 148, FIELD);
    art(row, OVERVIEW_EXPLORE, 29, 127);
    wrapped(row, 181, 133, has_run ? kit_route(kit) : "Ready to explore", 229,
            22, TEXT);
    text(row, 181, 195, kit_expedition_status(kit), 18, SECONDARY);
    if (has_run) {
      snprintf(value, sizeof(value), "%u / %u sec", elapsed,
               GAME_EXPEDITION_SECONDS);
      text(row, 181, 223, value, 18, SECONDARY);
      progress(row, 181, 255, 229, 12, elapsed, GAME_EXPEDITION_SECONDS);
    }
    text(row, 28, 281, reserved ? "Reserved for transfer" : "Collected", 18,
         SECONDARY);
    static const char *names[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      int x = 28 + (int)i * 132;
      const SelectedSprite *asset = &selected_sprites[i];
      resource(row, i, x + (51 - (int)asset->width) / 2, 306, 1);
      snprintf(value, sizeof(value), "%u", cargo[i] / GAME_SUPPLY_UNIT);
      text(row, x + 61, 318, value, 26, TEXT);
      text(row, x + 4, 361, names[i], 18, SECONDARY);
    }
    if (reserved || receipt) {
      wrapped(row, 28, 413, kit_stage(kit), 390, 18, SECONDARY);
    } else if (game->expedition_id[0] &&
               game->expedition_elapsed < GAME_EXPEDITION_SECONDS) {
      unsigned remaining = game_gather_remaining_ms(game);
      int preparing =
          game->expedition_active && !game_gather_capacity_blocked(game);
      text(row, 28, 395, preparing ? "Next attempt" : "Saved attempt", 18,
           SECONDARY);
      if (!preparing)
        strcpy(value, "Paused");
      else if (remaining)
        snprintf(value, sizeof(value), "in %u sec", (remaining + 999) / 1000);
      else
        strcpy(value, "Ready");
      text(row, 300, 395, value, 18, TEXT);
      progress(row, 28, 426, 386, 8, GAME_GATHER_ATTEMPT_MS - remaining,
               GAME_GATHER_ATTEMPT_MS);
      unsigned due = game_gather_due_mask(game);
      if (due != 7) {
        resource_names(value, sizeof(value), "Next: ", due);
        text(row, 28, 440, value, 18, SECONDARY);
      }
      if (game->gather_last_attempted_mask) {
        if (game->gather_last_awarded_mask)
          resource_names(value, sizeof(value),
                         "Last attempt: ", game->gather_last_awarded_mask);
        else
          strcpy(value, "Last attempt: no items found");
        text(row, 28, due == 7 ? 440 : 464, value, 18, TEXT);
      }
    } else {
      text(row, 28, 405,
           kit->journal.companion_online ? "Lab link available" : "Lab offline",
           18, SECONDARY);
      action_top = has_run ? 482 : 440;
    }
  }
  unsigned count =
      selector || kit->failed ? 0 : kit_option_count(kit, KIT_COMPANION);
  for (unsigned i = 0; i < count; ++i) {
    int y = action_top + (int)i * (has_run ? 32 : 38);
    if (view->focus == i) {
      fill(row, 24, y - 4, 402, 31,
           view->gestures[8].held && view->gestures[8].allowed ? FIELD : GLOW);
      border(row, 26, y - 2, 398, 27, FOCUS);
      text(row, 34, y, ">", 18, FOCUS);
    }
    text(row, 58, y, kit_option(kit, KIT_COMPANION, i), 18, TEXT);
  }
  if (kit->failed) {
    fill(row, 24, 496, 402, 57, FIELD);
    wrapped(row, 28, 502, "Storage unavailable. Cargo preserved.", 390, 18,
            FOCUS);
  } else if (selector) {
    text(row, 28, 503, "Left / Right: change mode", 22, TEXT);
    text(row, 28, 538,
         page == COMP_FRIENDS ? "Party controls are not ready yet"
                              : "Down / Confirm: choose an action",
         18, SECONDARY);
  }
  text(row, 28, 565,
       selector           ? "Browsing never sends or spends"
       : review           ? "Confirm: send  /  Back: keep cargo"
       : view->task_depth ? "Back: return to the previous view"
                          : "Up / Down: choose  /  Back: modes",
       18, SECONDARY);
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
      snprintf(value, sizeof(value), "%u %s", journal->dock_stock[i] / 100,
               journal->dock_stock[i] == 100 ? "unit" : "units");
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
  const GameState *game = &kit->lab->game;
  unsigned phase = kit->journal.phase;
  char value[128];
  fill(row, 0, 0, 1024, 600, BACKGROUND);
  if (row->y < 112) {
    selected_lab_row(kit->lab, row->y, row->pixels);
    return;
  }
  int manifest = phase >= KIT_ARRIVED;
  int accepted = phase >= KIT_ACK_PENDING;
  panel(row, 24, 134, 976, 412);
  text(row, 48, 158,
       manifest ? (accepted ? "Haul saved in Lab" : "Haul arrived")
                : "Expeditions",
       34, TEXT);
  text(row, 48, 204, manifest ? "From Companion" : "Gather with your Companion",
       22, SECONDARY);
  if (manifest) {
    text(row, 48, 246, accepted ? "Added to Lab" : "To Lab - not in stock yet",
         22, TEXT);
    static const char *names[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      int x = 60 + (int)i * 308;
      const SelectedSprite *asset = &selected_sprites[i];
      resource(row, i, x + (108 - (int)asset->width * 2) / 2, 289, 2);
      text(row, x + 123, 298, names[i], 22, SECONDARY);
      snprintf(value, sizeof(value), "%u %s",
               kit->journal.cargo[i] / GAME_SUPPLY_UNIT,
               kit->journal.cargo[i] == GAME_SUPPLY_UNIT ? "unit" : "units");
      text(row, x + 123, 337, value, 26, TEXT);
    }
    if (kit->journal.elapsed < GAME_EXPEDITION_SECONDS)
      text(row, 48, 416, "Gathering progress stays on Companion", 22,
           SECONDARY);
    const GameSample *sample = kit_received_sample(kit);
    if (accepted && sample) {
      snprintf(value, sizeof(value), "Sample recorded: %s", sample->id);
    } else if (accepted) {
      snprintf(value, sizeof(value), "Supplies saved / no sample recorded");
    } else {
      snprintf(value, sizeof(value), "%s",
               kit->journal.elapsed < GAME_EXPEDITION_SECONDS ? "Supplies only"
               : game->sample_count < GAME_MAX_SAMPLES
                   ? "Sample ready to record"
                   : "Sample shelf full / supplies only");
    }
    text(row, 48, 467, value, 22, TEXT);
  } else {
    fill(row, 48, 250, 156, 156, FIELD);
    art(row, OVERVIEW_EXPLORE, 58, 256);
    text(row, 236, 255,
         game->expedition_id[0] ? kit_route(kit) : "No expedition", 32, TEXT);
    text(row, 236, 302,
         game->expedition_id[0] ? kit_expedition_status(kit)
                                : "Choose a route on Companion",
         22, SECONDARY);
    if (game->expedition_id[0]) {
      progress(row, 236, 347, 690, 16, game->expedition_elapsed,
               GAME_EXPEDITION_SECONDS);
      snprintf(value, sizeof(value), "%u / %u active seconds",
               game->expedition_elapsed, GAME_EXPEDITION_SECONDS);
      text(row, 236, 375, value, 18, SECONDARY);
    }
    text(row, 48, 423, "On Companion", 22, SECONDARY);
    uint32_t cargo[] = {game->expedition_data, game->expedition_energy,
                        game->expedition_essence};
    static const char *names[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      snprintf(value, sizeof(value), "%s %u %s", names[i],
               cargo[i] / GAME_SUPPLY_UNIT,
               cargo[i] == GAME_SUPPLY_UNIT ? "unit" : "units");
      text(row, 48 + (int)i * 308, 461, value, 22, TEXT);
    }
  }
  if (phase == KIT_ARRIVED) {
    fill(row, 45, 502, 382, 35, GLOW);
    border(row, 48, 504, 376, 31, FOCUS);
    text(row, 64, 509, "Confirm: accept haul", 22, FOCUS);
  } else {
    text(row, 48, 509,
         phase == KIT_ACK_PENDING        ? "Waiting for Companion receipt"
         : phase == KIT_COMPLETE         ? "Companion receipt confirmed"
         : kit->journal.companion_online ? "Lab link available (simulation)"
                                         : "Companion offline",
         22, SECONDARY);
  }
  text(row, 48, 565,
       kit->caller_valid ? "Back: return to your previous screen"
                         : "Back: Lab overview",
       18, SECONDARY);
  if (kit->failed)
    text(row, 48, 546, "Storage unavailable. Cargo preserved.", 18, FOCUS);
}
static void render_row(const DeviceKit *kit, unsigned device, unsigned y,
                       uint8_t *pixels) {
  if (device == KIT_LAB && !kit_lab_explore(kit)) {
    selected_lab_row(kit->lab, y, pixels);
    if (kit->normalization_pending) {
      KitRow row = {y, kit_width(device), pixels, 0};
      fill(&row, 24, 548, 976, 38, FIELD);
      border(&row, 24, 548, 976, 38, FOCUS);
      text(&row, 36, 557,
           "Accept the existing haul before supply conversion can finish.", 18,
           TEXT);
    }
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
