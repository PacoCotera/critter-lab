#include "core_art.h"
#include "expedition_render.h"
#include "kit.h"
#include "native_font.h"
#include "native_ui.h"
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
static void heading(KitRow *row, int x, int y, const char *value,
                    unsigned size, unsigned color) {
  const NativeFont *selected = NULL;
  for (unsigned i = 0; i < LAB_HEADING_FONT_COUNT; ++i)
    if ((unsigned)lab_heading_narrow_fonts[i].size == size)
      selected = &lab_heading_narrow_fonts[i];
  if (!selected)
    return;
  const uint8_t black[] = {0, 0, 0};
  native_text_row(selected, value, x, y, row->y, row->width, row->pixels, 0,
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
static void resource(KitRow *row, unsigned icon, int x, int y, int primary) {
  CoreArtId id = (CoreArtId)((primary ? CORE_ART_DATA_PRIMARY
                                    : CORE_ART_DATA_COMPACT) + icon);
  core_art_row(id, x, y, row->y, row->width, row->pixels);
}
static void category(KitRow *row, CoreArtId id, int x, int y) {
  core_art_row(id, x, y, row->y, row->width, row->pixels);
}
static void panel(KitRow *row, int x, int y, int width, int height) {
  core_art_panel_row(x, y, width, height, row->y, row->width, row->pixels);
}
static void action_focus(KitRow *row, int x, int y, int width, int height) {
  core_art_focus_row(x, y, width, height, row->y, row->width, row->pixels);
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
/* Expedition layouts use top-of-ink coordinates like the reviewed study.
 * Keep the existing legacy screen anchor unchanged. */
static void field_label(KitRow *row, int x, int y, const char *value,
                         unsigned size, unsigned color, int bold) {
  const NativeFont *fonts = bold ? lab_heading_fonts : lab_fonts;
  unsigned count = bold ? LAB_HEADING_FONT_COUNT : LAB_FONT_COUNT;
  const NativeFont *font = &fonts[0];
  for (unsigned i = 0; i < count; ++i)
    if ((unsigned)fonts[i].size == size) font = &fonts[i];
  int ink_top = font->size;
  for (const unsigned char *at = (const unsigned char *)value; *at; ++at) {
    unsigned index = *at >= 32 && *at <= 126 ? *at - 32 : '?' - 32;
    const NativeGlyph *glyph = &font->glyphs[index];
    if (glyph->height && glyph->top < ink_top) ink_top = glyph->top;
  }
  native_text_row(font, value, x, y - ink_top, row->y, row->width,
                  row->pixels, 0, palette[color]);
}
static void field_text(KitRow *row, int x, int y, const char *value,
                        unsigned size, unsigned color) {
  field_label(row, x, y, value, size, color, 0);
}
static void field_heading(KitRow *row, int x, int y, const char *value,
                           unsigned size, unsigned color) {
  field_label(row, x, y, value, size, color, 1);
}
static void field_cargo_row(const DeviceKit *kit, KitRow *row,
                            const ExpeditionFieldView *field, unsigned page,
                            int selector) {
  const KitView *view = &kit->companion;
  int review = page == COMP_SEND_REVIEW;
  int sealed = kit->journal.phase >= KIT_WAITING &&
               kit->journal.phase <= KIT_ACK_PENDING;
  int accepted = field->delivery_accepted;
  int acknowledged = accepted && kit->journal.phase == KIT_COMPLETE;
  char value[128];
  fill(row, 0, 0, 450, 600, BACKGROUND);
  panel(row, 12, 12, 426, 576);
  field_heading(row, 29, 28, review ? "Return to Lab" : accepted ? "Cargo empty"
                                                   : sealed ? "Expedition sent"
                                                                  : "Cargo", 24, TEXT);
  field_text(row, 30, 59, review ? "Review freezes gathering"
                  : acknowledged ? "Expedition ended / choose a new outing"
                                  : kit_stage(kit), 15, SECONDARY);
  static const char *modes[] = {"Probe", "Cargo", "Companions"};
  static const int positions[] = {28, 151, 275};
  for (unsigned i = 0; i < 3; ++i) {
    int selected = view->mode == i;
    if (selector && selected)
      action_focus(row, positions[i] - 4, 78, i == 2 ? 143 : 98, 27);
    field_heading(row, positions[i], 82, modes[i], 18, selected ? TEXT : SECONDARY);
    if (selected) fill(row, positions[i], 98, i == 2 ? 138 : 90, 2, BORDER);
  }
  panel(row, 24, 113, 402, 281);
  field_heading(row, 42, 135, "Current cargo", 23, TEXT);
  unsigned total = 0;
  for (unsigned i = 0; i < 3; ++i) {
    int x = 49 + (int)i * 126;
    unsigned count = field->earned[i];
    total += count;
    resource(row, i, x, 175, 0);
    snprintf(value, sizeof(value), "%u", count);
    field_heading(row, x + 57, 177, value, 27, TEXT);
    field_text(row, x, 237, i == 0 ? "Data" : i == 1 ? "Energy" : "Essence", 18, SECONDARY);
  }
  fill(row, 42, 270, 364, 1, EDGE);
  unsigned capsules = field->capsule_count;
  if (capsules) {
    category(row, CORE_ART_SAMPLE_NEUTRAL, 49, 283);
    field_heading(row, 120, 289, "1 sealed sample", 22, TEXT);
    field_text(row, 120, 321, "Contents unknown", 18, SECONDARY);
  } else {
    field_heading(row, 49, 294, "No sample in cargo", 22, TEXT);
  }
  if (accepted) {
    snprintf(value, sizeof(value), "Delivery record: %u Data / %u Energy / %u Essence",
             field->sent[0], field->sent[1], field->sent[2]);
    field_text(row, 42, 324, value, 14, SECONDARY);
    field_text(row, 42, 342, field->sent_capsule_count ? "1 sample delivered to Lab"
                                                   : "Supplies-only delivery", 14, SECONDARY);
  }
  snprintf(value, sizeof(value), "Supplies %u / %u / Capsules %u / %u", total,
           GAME_CARGO_CAPACITY / GAME_SUPPLY_UNIT, capsules, field->capsule_capacity);
  field_text(row, 42, 363, value, 16, SECONDARY);
  if (review) {
    field_text(row, 28, 414, capsules ? "Send earned items and the sample."
                              : "Send earned items to the Lab.", 18, TEXT);
    field_text(row, 28, 442, "Seals the outing / gathering stops.", 18, SECONDARY);
  } else if (accepted) {
    field_text(row, 28, 414, acknowledged ? "Delivery complete / cargo transferred."
                                         : "Lab accepted / receipt pending.", 18, TEXT);
    field_text(row, 28, 442, acknowledged ? "Choose a new outing on Probe."
                                        : "This expedition cannot resume.", 18, SECONDARY);
  } else if (sealed) {
    field_text(row, 28, 414, kit->journal.phase == KIT_WAITING
                               ? "Sent / waiting for the Lab."
                               : "At Lab / awaiting acceptance.", 18, TEXT);
    field_text(row, 28, 442, "This expedition cannot resume.", 18, SECONDARY);
  } else {
    const char *message = view->message[0] ? view->message : "Whole items only / preparation stays here.";
    wrapped(row, 28, 414, message, 390, 18, SECONDARY);
  }
  unsigned count = selector || kit->failed ? 0 : kit_option_count(kit, KIT_COMPANION);
  for (unsigned i = 0; i < count && i < 2; ++i) {
    int top = 479 + (int)i * 32;
    if (i == view->focus) action_focus(row, 29, top, 392, 30);
    field_heading(row, 45, top + 8, kit_option(kit, KIT_COMPANION, i), 20,
            i == view->focus ? FOCUS : TEXT);
  }
  if (selector) {
    field_text(row, 28, 491, "Left / Right: change mode", 22, TEXT);
    field_text(row, 28, 525, "Down / Confirm: enter Cargo", 18, SECONDARY);
  }
  field_text(row, 28, 556, selector ? "Browsing never sends or spends"
                    : review ? "Confirm: choose / Back: keep exploring"
                             : "Up/Down: choose / Back: modes", 16, SECONDARY);
  if (kit->failed) {
    fill(row, 24, 493, 402, 64, FIELD);
    wrapped(row, 28, 505, accepted ? "Cargo transferred. Delivery record needs recovery."
                                 : "Storage unavailable. Cargo preserved.", 390, 18, FOCUS);
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
  int ended = (receipt || kit->journal.phase == KIT_COMPLETE) && !game->expedition_id[0];
  int details = page == COMP_CARGO || page == COMP_SEND_REVIEW;
  int review = page == COMP_SEND_REVIEW;
  int discard = page == COMP_DISCARD_CLASS || page == COMP_DISCARD_QUANTITY ||
                page == COMP_DISCARD_REVIEW;
  int finish = page == COMP_FINISH_REVIEW;
  int friends = page == COMP_FRIENDS || page == COMP_FRIEND_VISIT;
  int friend_visit = page == COMP_FRIEND_VISIT;
  char value[128];
  uint32_t cargo[] = {game->expedition_data, game->expedition_energy,
                      game->expedition_essence};
  ExpeditionFieldView field;
  int map_outing = kit_field_projection(kit, &field);
  if (map_outing && game->expedition_id[0] && !reserved && !receipt &&
      (page == COMP_PROBE || page == COMP_FIELD_SITE)) {
    expedition_field_row(&field, row->y, row->pixels);
    if (selector) {
      action_focus(row, 24, 78, 98, 27);
      field_heading(row, 28, 82, "Probe", 18, FOCUS);
      fill(row, 24, 474, 402, 103, BACKGROUND);
      text(row, 28, 490, "Left / Right: change mode", 22, TEXT);
      text(row, 28, 525, "Down / Confirm: enter Probe", 18, SECONDARY);
      text(row, 28, 557, "Confirm: enter selected mode", 16, SECONDARY);
    }
    if (kit->failed) {
      fill(row, 24, 500, 402, 76, FIELD);
      wrapped(row, 28, 507, "Storage unavailable. Progress preserved.", 390, 18, FOCUS);
    }
    return;
  }
  if (map_outing && (details || reserved || receipt) && !discard && !finish && !friends) {
    field_cargo_row(kit, row, &field, page, selector);
    return;
  }
  if (reserved)
    memcpy(cargo, kit->journal.cargo, sizeof(cargo));
  unsigned total = cargo[0] + cargo[1] + cargo[2];
  int has_run = reserved || game->expedition_id[0];
  unsigned elapsed = reserved ? kit->journal.elapsed : game->expedition_elapsed;
  fill(row, 0, 0, 450, 600, BACKGROUND);
  panel(row, 12, 12, 426, 576);
  text(row, 28, 26, "BEECHO / COMPANION", 18, SECONDARY);
  heading(row, 28, 48,
         details || discard     ? "CARGO"
       : friends ? "COMPANIONS"
                              : "PROBE",
       34, TEXT);
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
        action_focus(row, x - 4, 82, 128, 30);
      }
    }
    text(row, x + 4, 88, modes[i], 18, view->mode == i ? TEXT : SECONDARY);
  }
  fill(row, 28, 112, 394, 2, EDGE);
  int action_top = 490;
  if (discard || finish) {
    static const char *names[] = {"Data", "Energy", "Essence"};
    text(row, 28, 134,
         finish ? "End this expedition?"
         : page == COMP_DISCARD_CLASS ? "Choose item kind"
         : page == COMP_DISCARD_QUANTITY ? "Choose whole items"
                                         : "Discard these items?",
         22, TEXT);
    if (finish) {
      wrapped(row, 28, 204, "This ends the expedition without a sample. Nothing is sent to the Lab.",
              390, 22, TEXT);
      wrapped(row, 28, 320, "Your next supply attempts keep their progress.",
              390, 18, SECONDARY);
    } else {
      unsigned resource_index = page == COMP_DISCARD_CLASS ? view->focus % 3
                                                          : view->discard_resource;
      resource(row, resource_index, 50, 208, 1);
      text(row, 187, 220, names[resource_index], 22, TEXT);
      snprintf(value, sizeof(value), "Carried: %u %s", cargo[resource_index] / GAME_SUPPLY_UNIT, names[resource_index]);
      text(row, 28, 346, value, 18, SECONDARY);
      if (page == COMP_DISCARD_REVIEW) {
        snprintf(value, sizeof(value), "Discard %u %s?",
                 view->discard_quantity / GAME_SUPPLY_UNIT, names[resource_index]);
        text(row, 28, 384, value, 22, TEXT);
        text(row, 28, 417, "These items cannot be recovered.", 18, SECONDARY);
      } else {
        text(row, 28, 384, "Choose how many to keep or discard.", 18, SECONDARY);
      }
    }
    action_top = 470;
  } else if (friends) {
    const KitResidentProjection *record = kit_selected_resident(kit);
    unsigned count = kit_resident_count(kit);
    time_t updated = (time_t)kit_residents_updated_at(kit) - 6 * 3600;
    struct tm *snapshot = gmtime(&updated);
    char stamp[16] = "unknown";
    if (kit_residents_updated_at(kit) && snapshot)
      strftime(stamp, sizeof(stamp), "%H:%M", snapshot);
    snprintf(value, sizeof(value), "%s / %s %s",
             kit->journal.companion_online ? "Lab connected" : "Offline",
             kit_resident_cache_current(kit) ? "Updated" : "Last Lab update",
             stamp);
    text(row, 28, 128, value, 18, SECONDARY);
    if (record && count) {
      if (!selector && !friend_visit)
        action_focus(row, 26, 154, 398, 35);
      heading(row, 39, 160,
              record->individual.expression.pale_markings ? "Pale markings" : "Plain coat",
              26, TEXT);
      text(row, 280, 165, record->individual.id, 18, SECONDARY);
      unsigned asset;
      if (selected_lab_original_art(&record->individual, &record->metadata, &asset))
        core_art_row((CoreArtId)asset, 28, 194, row->y, row->width, row->pixels);
      else
        wrapped(row, 45, 282, "Portrait pending", 235, 22, SECONDARY);
      text(row, 303, 195, "Visits", 18, SECONDARY);
      snprintf(value, sizeof(value), "%u", record->individual.care_visits);
      heading(row, 303, 223, value, 32, TEXT);
      const char *form_title = selected_lab_resident_form_title(
          &record->individual, &record->metadata);
      const char *property = NULL;
      if (form_title && !strcmp(record->metadata.candidate_id, "B1"))
        property = "Burst capable / Baseline walking energy";
      else if (form_title && !strcmp(record->metadata.candidate_id, "B0"))
        property = "Steady / Lower walking energy";
      if (property)
        wrapped(row, 303, 267, property, 116, 18, TEXT);
      int feedback_y = property ? 386 : 278;
      int visit_available = kit_resident_visit_available(kit);
      int saved_message = !strncmp(view->message, "Visit saved", 11);
      const char *feedback;
      if (!visit_available) {
        feedback = reserved || receipt ? "Finish transfer before visiting."
                   : kit_resident_cache_current(kit) ? "Visit unavailable for this resident."
                   : saved_message ? "Previous visit saved in Lab. Reconnect."
                                   : "Reconnect to the Lab to spend time together.";
      } else if (saved_message)
        feedback = "You spent time together. Visit saved.";
      else
        feedback = view->message[0] ? view->message : "Spend time together.";
      wrapped(row, 303, feedback_y, feedback, 116, 18,
              visit_available ? TEXT : SECONDARY);
      if (!friend_visit && !selector) {
        snprintf(value, sizeof(value), "%u / %u residents", view->focus + 1, count);
        text(row, 28, 489, value, 18, SECONDARY);
        heading(row, 28, 516, "Confirm: view this critter", 26, TEXT);
      }
    } else {
      heading(row, 28, 177, "NO REVEALED RESIDENTS", 26, TEXT);
      art(row, OVERVIEW_HABITAT, 37, 251);
      wrapped(row, 203, 251, "Reveal a resident at the Lab to meet here.", 215, 22, TEXT);
    }
    action_top = 490;
  } else if (details) {
    heading(row, 28, 125,
         ended      ? "Cargo empty"
         : review     ? "To Lab"
         : reserved ? "Reserved for transfer"
                    : "Collected items",
         26, TEXT);
    if (has_run || receipt || kit->journal.phase == KIT_COMPLETE) {
      text(row, 28, 158, ended ? "Expedition ended" : kit_route(kit), 18, SECONDARY);
      const char *status = ended ? "Supplies stored at the Lab" : kit_expedition_status(kit);
      if (map_outing && !ended)
        status = field.capsule_count ? "Sealed sample / contents unknown"
                                    : "Supplies only / no sample collected";
      else if (review)
        status = elapsed < GAME_EXPEDITION_SECONDS ? "Supplies only / no sample"
                 : game->sample_count < GAME_MAX_SAMPLES ? "Sample ready to record"
                                                        : "Sample shelf full / supplies only";
      text(row, 28, 182, status, 18, SECONDARY);
      if (!ended && !map_outing) {
        snprintf(value, sizeof(value), "%u / %u sec", elapsed,
                 GAME_EXPEDITION_SECONDS);
        text(row, 300, 158, value, 18, SECONDARY);
        if (!review)
          progress(row, 228, 187, 186, 8, elapsed, GAME_EXPEDITION_SECONDS);
      }
    }
    panel(row, 26, 215, 398, 190);
    static const char *names[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      int x = 28 + (int)i * 132;
      if (i)
        fill(row, x, 231, 1, 155, EDGE);
      resource(row, i, x + 12, 249, 1);
      text(row, x + 13, 363, names[i], 18, SECONDARY);
      snprintf(value, sizeof(value), "%u", cargo[i] / GAME_SUPPLY_UNIT);
      heading(row, x + 81, 220, value, 32, TEXT);
    }
    if (review) {
      wrapped(row, 28, 420,
              "Send stops gathering. Lab acceptance stores the haul and ends the expedition.",
              390, 18, SECONDARY);
    } else {
      snprintf(value, sizeof(value), "Free space: %u / 40 units",
               (GAME_CARGO_CAPACITY - total) / GAME_SUPPLY_UNIT);
      text(row, 28, 414, value, 18, SECONDARY);
      if (reserved || receipt || (kit->journal.phase == KIT_COMPLETE && !has_run))
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
    /* Authored fictional setting; facts remain in separate live zones. */
    core_art_row(CORE_ART_PROBE_PLACE, 25, 116, row->y, row->width, row->pixels);
    if (has_run) {
      text(row, 28, 344, kit_route(kit), 18, TEXT);
      snprintf(value, sizeof(value), "%u / %u sec", elapsed, GAME_EXPEDITION_SECONDS);
      text(row, 302, 344, value, 18, SECONDARY);
      progress(row, 28, 366, 394, 2, elapsed, GAME_EXPEDITION_SECONDS);
      static const char *names[] = {"Data", "Energy", "Essence"};
      for (unsigned i = 0; i < 3; ++i) {
        int x = 28 + (int)i * 132;
        const CoreArtSprite *asset = core_art_sprite((CoreArtId)(CORE_ART_DATA_COMPACT + i));
        resource(row, i, x + (51 - (int)asset->width) / 2, 368, 0);
        snprintf(value, sizeof(value), "%u", cargo[i] / GAME_SUPPLY_UNIT);
        heading(row, x + 64, 386, value, 32, TEXT);
        text(row, x + 4, 417, names[i], 18, SECONDARY);
      }
      if (reserved || receipt) {
        wrapped(row, 28, 456, kit_stage(kit), 390, 18, SECONDARY);
      } else if (game->expedition_elapsed < GAME_EXPEDITION_SECONDS) {
        unsigned remaining = game_gather_remaining_ms(game);
        if (game->expedition_active && game_gather_capacity_blocked(game)) {
          unsigned needed = game_gather_required_slots(game);
          snprintf(value, sizeof(value), "Need %u free cargo units", needed ? needed : 1);
        } else if (!game->expedition_active) {
          strcpy(value, "Paused / next supply attempt saved");
        } else {
          snprintf(value, sizeof(value), "Next try in %u sec / may find supplies",
                   (remaining + 999) / 1000);
        }
        text(row, 28, 439, value, 18, SECONDARY);
        if (game->gather_last_attempted_mask) {
          if (game->gather_last_awarded_mask)
            resource_names(value, sizeof(value), "Last: ", game->gather_last_awarded_mask);
          else
            strcpy(value, "Last attempt: no items found");
          text(row, 28, 463, value, 18, TEXT);
        }
      } else {
        text(row, 28, 457, kit_expedition_status(kit), 18, SECONDARY);
      }
      action_top = 490;
    } else {
      heading(row, 28, 360, ended ? "EXPEDITION ENDED" : "READY TO EXPLORE", 26, TEXT);
      text(row, 28, 402, ended ? "Cargo empty / supplies stored at the Lab"
                              : "Choose an expedition to begin.", 18, SECONDARY);
      action_top = 445;
    }
  }
  unsigned count =
      selector || kit->failed || (page == COMP_FRIENDS && kit_resident_count(kit))
          ? 0 : kit_option_count(kit, KIT_COMPANION);
  unsigned first = discard && view->focus >= 2 ? view->focus - 1 : 0;
  unsigned visible_count = discard ? 2 : count;
  for (unsigned i = first; i < count && i < first + visible_count; ++i) {
    int y = action_top + (int)(i - first) * (has_run || discard || finish || friend_visit ? 32 : 38);
    if (view->focus == i) {
      fill(row, 24, y - 4, 402, 31,
           FIELD);
      action_focus(row, 26, y - 2, 398, 27);
      text(row, 34, y, ">", 18, FOCUS);
    }
    const char *option = kit_option(kit, KIT_COMPANION, i);
    if (page == COMP_DISCARD_REVIEW && i == 0) {
      static const char *names[] = {"Data", "Energy", "Essence"};
      snprintf(value, sizeof(value), "Discard %u %s",
               view->discard_quantity / GAME_SUPPLY_UNIT,
               names[view->discard_resource]);
      option = value;
    }
    heading(row, 58, y - 3, option, 26,
            friend_visit && i == 0 && !kit_resident_visit_available(kit) ? SECONDARY : TEXT);
  }
  if (kit->failed) {
    fill(row, 24, 496, 402, 57, FIELD);
    wrapped(row, 28, 502, "Storage unavailable. Cargo preserved.", 390, 18,
            FOCUS);
  } else if (selector) {
    text(row, 28, 490, "Left / Right: change mode", 22, TEXT);
    text(row, 28, 524,
         "Down / Confirm: choose an action",
         18, SECONDARY);
  } else if (view->message[0] && !friends) {
    int result_y = discard || finish ? 413 : details ? 439 : 276;
    fill(row, 24, result_y, 402, discard || finish || details ? 46 : 64, FIELD);
    wrapped(row, 28, result_y + 3, view->message, 390, 18, TEXT);
  }
  text(row, 28, 553,
       selector           ? "Browsing never sends or spends"
       : page == COMP_FRIENDS ? kit_resident_count(kit)
                                   ? "Up/Down: resident / Back: modes"
                                   : "Confirm: Probe / Back: modes"
       : friend_visit     ? "Confirm: choose / Back: residents"
       : discard          ? "Up/Down: choose / Back: keep items"
       : finish           ? "Confirm: choose / Back: keep exploring"
       : review           ? "Confirm: choose / Back: keep cargo"
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
       kit_dock_cache_current(kit) ? "Synced (simulation)"
       : journal->dock_online ? "Cached / stale" : "Offline / cached", 22,
       TEXT);
  fill(row, 24, 66, 744, 2, TEXT);
  if (view->page == 2) {
    text(row, 24, 89, "World summary print preview", 26, TEXT);
    text(row, 24, 133, "No physical printer or paper output is connected.", 22,
         TEXT);
  } else if (view->focus == 0) {
    const CoreArtId icons[] = {CORE_ART_RESIDENTS_MONO, CORE_ART_SAMPLES_MONO,
                               CORE_ART_INCUBATING_MONO};
    const unsigned amounts[] = {journal->dock_residents, journal->dock_samples,
                                 journal->dock_incubations};
    const char *names[] = {"Residents", "Samples", "Incubating"};
    for (unsigned i = 0; i < 3; ++i) {
      int x = 24 + (int)i * 248;
      category(row, icons[i], x, 89);
      text(row, x + 47, 87, names[i], 22, TEXT);
      snprintf(value, sizeof(value), "%u", amounts[i]);
      text(row, x + 47, 117, value, 32, TEXT);
    }
    snprintf(value, sizeof(value), "Visits together: %u", kit_dock_visits(kit));
    text(row, 24, 153, value, 18, TEXT);
  } else if (view->focus == 1) {
    static const char *labels[] = {"Data", "Energy", "Essence"};
    for (unsigned i = 0; i < 3; ++i) {
      int x = 24 + (int)i * 248;
      category(row, (CoreArtId)(CORE_ART_DATA_MONO + i), x, 89);
      text(row, x + 47, 85, labels[i], 22, TEXT);
      snprintf(value, sizeof(value), "%u %s", journal->dock_stock[i] / 100,
               journal->dock_stock[i] == 100 ? "unit" : "units");
      text(row, x + 47, 122, value, 22, TEXT);
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
           kit_dock_cache_current(kit) ? "" : " / stale");
  text(row, 24, 178, value, 18, TEXT);
  if (view->message[0])
    text(row, 24, 201, view->message, 18, TEXT);
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
static int bmp_rows(const DeviceKit *kit, unsigned device, FILE *output,
                    const uint8_t *frame) {
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
  CompanionCargoView view;
  if (device != KIT_COMPANION || !kit_cargo_projection(kit, &view)) {
    /* Other-device frame requests must not cancel Companion presentation. */
    if (device == KIT_COMPANION) native_ui_cancel(context);
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
