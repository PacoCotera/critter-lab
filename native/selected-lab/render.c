#include "assets.h"
#include "core_art.h"
#include "native_font.h"
#include "overview_assets.h"
#include "selected_lab.h"
#include <assert.h>
#include <string.h>

typedef struct {
  unsigned y;
  uint8_t *pixels;
} SelectedRow;
enum {
  BASE,
  PANEL,
  INK,
  MUTED,
  BLUE,
  EDGE,
  DEEP,
  WARM,
  SAGE,
  ACTION,
  ART_FIELD,
  FOCUS_GLOW
};
static const uint8_t colors[][3] = {
    {25, 36, 43},    {29, 38, 45},   {214, 222, 226}, {183, 198, 205},
    {29, 119, 191},  {56, 100, 132}, {10, 17, 23},    {237, 197, 106},
    {163, 206, 159}, {41, 41, 34},   {42, 51, 56},    {77, 70, 48}};

static void rectangle(SelectedRow *row, int x, int y, int width, int height,
                      unsigned color) {
  if ((int)row->y < y || (int)row->y >= y + height)
    return;
  for (int column = x; column < x + width; ++column)
    if (column >= 0 && column < (int)SELECTED_LAB_WIDTH)
      memcpy(row->pixels + column * 3, colors[color], 3);
}

static void outline(SelectedRow *row, int x, int y, int width, int height,
                    int weight, unsigned color) {
  rectangle(row, x, y, width, weight, color);
  rectangle(row, x, y + height - weight, width, weight, color);
  rectangle(row, x, y, weight, height, color);
  rectangle(row, x + width - weight, y, weight, height, color);
}

static void panel(SelectedRow *row, int x, int y, int width, int height) {
  core_art_panel_row(x, y, width, height, row->y, SELECTED_LAB_WIDTH, row->pixels);
}

static const NativeFont *font(int size) {
  for (unsigned index = 0; index < LAB_FONT_COUNT; ++index)
    if (lab_fonts[index].size == size)
      return &lab_fonts[index];
  assert(!"Missing existing native Lab font size");
  return NULL;
}

static void label(SelectedRow *row, int x, int y, const char *text, int size,
                  unsigned color) {
  native_text_row(font(size), text, x, y, row->y, SELECTED_LAB_WIDTH,
                  row->pixels, 0, colors[color]);
}

static const NativeFont *heading_font(int size, int narrow) {
  const NativeFont *fonts =
      narrow ? lab_heading_narrow_fonts : lab_heading_fonts;
  for (unsigned index = 0; index < LAB_HEADING_FONT_COUNT; ++index)
    if (fonts[index].size == size)
      return &fonts[index];
  assert(!"Missing native Lab heading font size");
  return NULL;
}

static void heading(SelectedRow *row, int x, int y, const char *text, int size,
                    unsigned color) {
  native_text_row(heading_font(size, 1), text, x, y, row->y, SELECTED_LAB_WIDTH,
                  row->pixels, 0, colors[color]);
}

static void stock_amount(SelectedRow *row, int x, unsigned amount) {
  char whole[16];
  snprintf(whole, sizeof(whole), "%u", amount / 100);
  /* The valid five-digit unit cap must keep the same right-hand inset. */
  const NativeFont *bold = heading_font(26, 1);
  int top = 74 - bold->baseline;
  native_text_row(bold, whole, x, top, row->y, SELECTED_LAB_WIDTH, row->pixels,
                  0, colors[INK]);
  label(row, x, 75, amount == GAME_SUPPLY_UNIT ? "unit" : "units", 18, MUTED);
}

static void wrapped_label(SelectedRow *row, int x, int y, const char *text,
                          int size, unsigned color, int width) {
  char line[160] = {0};
  size_t used = 0;
  while (*text) {
    const char *end = strchr(text, ' ');
    size_t length = end ? (size_t)(end - text) : strlen(text);
    char candidate[160];
    snprintf(candidate, sizeof(candidate), "%s%.*s", line, (int)length, text);
    if (used && native_text_width(font(size), candidate) > width) {
      label(row, x, y, line, size, color);
      y += size + 5;
      used = 0;
      line[0] = 0;
    }
    if (used + length + 2 >= sizeof(line))
      break;
    memcpy(line + used, text, length);
    used += length;
    line[used++] = ' ';
    line[used] = 0;
    text += length;
    if (*text == ' ')
      ++text;
  }
  if (used)
    label(row, x, y, line, size, color);
}

/* Exact retained source pixels. Boxes position artwork; they never resample it. */
static void sprite(SelectedRow *row, unsigned asset, int x, int y,
                   unsigned width, unsigned height) {
  static const CoreArtId materials[] = {
      CORE_ART_DATA_COMPACT, CORE_ART_ENERGY_COMPACT, CORE_ART_ESSENCE_COMPACT,
      CORE_ART_SAMPLE_NEUTRAL, CORE_ART_CROWN_REFERENCE,
      CORE_ART_EYE_RING_REFERENCE, CORE_ART_SAMPLE_NEUTRAL};
  CoreArtId id = asset < SELECTED_SPRITE_COUNT ? materials[asset]
                 : asset == SELECTED_SPRITE_COUNT ? CORE_ART_PIP_PLAIN
                                                  : CORE_ART_PIP_MARKED;
  if (asset < 3 && width >= 65 && height >= 86)
    id = (CoreArtId)(CORE_ART_DATA_PRIMARY + asset);
  const CoreArtSprite *image = core_art_sprite(id);
  x += ((int)width - (int)image->width) / 2;
  y += ((int)height - (int)image->height) / 2;
  core_art_row(id, x, y, row->y, SELECTED_LAB_WIDTH, row->pixels);
}

/* Fit the two supported-form previews, preserving the master art and its ratio.
 * The normal sprite helper uses exact native footprints, not scaling slots. */
static void form_preview(SelectedRow *row, unsigned asset, int x, int y) {
  CoreArtId id = asset == SELECTED_SPRITE_COUNT ? CORE_ART_PIP_PLAIN
                                              : CORE_ART_PIP_MARKED;
  const CoreArtSprite *source = core_art_sprite(id);
  unsigned width = 181;
  unsigned height = source->height * width / source->width;
  int relative_row = (int)row->y - y;
  if (relative_row < 0 || (unsigned)relative_row >= height) return;
  unsigned source_row = (unsigned)relative_row * source->height / height;
  for (unsigned column = 0; column < width; ++column) {
    int destination = x + (int)column;
    if (destination < 0 || destination >= (int)SELECTED_LAB_WIDTH) continue;
    unsigned source_column = column * source->width / width;
    const uint8_t *rgba = source->rgba +
        (source_row * source->width + source_column) * 4;
    uint8_t *target = row->pixels + destination * 3;
    unsigned alpha = rgba[3];
    for (unsigned channel = 0; channel < 3; ++channel)
      target[channel] = (uint8_t)((rgba[channel] * alpha +
          target[channel] * (255u - alpha) + 127u) / 255u);
  }
}

void selected_lab_sprite_row(unsigned asset, int x, int y, unsigned width,
                             unsigned height, unsigned y_row,
                             uint8_t pixels[SELECTED_LAB_WIDTH * 3]) {
  SelectedRow row = {y_row, pixels};
  sprite(&row, asset, x, y, width, height);
}

int selected_lab_original_art(const GameIndividual *individual,
                               const GameIndividualMetadata *metadata,
                               unsigned *asset) {
  if (!individual || !metadata || !asset || !individual->revealed ||
      individual->art_pending || strcmp(individual->art_version, PIP_ART_VERSION) ||
      strcmp(metadata->original_art_version, PIP_ART_VERSION))
    return 0;
  for (unsigned id = CORE_ART_PIP_PLAIN; id <= CORE_ART_PIP_MARKED; ++id) {
    const CoreArtSprite *original = core_art_sprite((CoreArtId)id);
    char path[40];
    snprintf(path, sizeof(path), "design/v1-pip/%s.png", original->name);
    if (!strcmp(individual->art_id, path) &&
        !strcmp(metadata->original_art_sha256, original->source_sha256)) {
      *asset = id;
      return 1;
    }
  }
  return 0;
}

static void resident_portrait(SelectedRow *row, const GameState *game,
                               unsigned resident, int x, int y) {
  unsigned asset;
  if (resident < game->individual_count &&
      selected_lab_original_art(&game->individuals[resident],
                                 &game->individual_metadata[resident], &asset))
    core_art_row((CoreArtId)asset, x, y, row->y, SELECTED_LAB_WIDTH, row->pixels);
  else
    label(row, x, y + 110, "Portrait pending", 22, MUTED);
}

const char *selected_lab_resident_form_title(
    const GameIndividual *individual,
    const GameIndividualMetadata *metadata) {
  if (!individual || !metadata || !individual->revealed ||
      !individual->id[0] || !individual->source_sample_id[0] ||
      strcmp(metadata->reference_context, "pip:adult-rested-firm-ground-mild-v1"))
    return NULL;
  /* Creation saved the selected form under this mapping and context. Resident
   * views must not reconstruct candidates from mutable source research. */
  if (!strcmp(metadata->mapping_version, "pip-discovery-map-v1")) {
    static const struct { const char *id, *title; } forms[] = {
        {"A0", "Plain coat / pale variation carried"},
        {"A1", "Pale markings"},
        {"B0", "Steady / lower walking cost"},
        {"B1", "Burst-capable / baseline walking cost"}};
    for (unsigned form = 0; form < sizeof(forms) / sizeof(forms[0]); ++form)
      if (!strcmp(metadata->candidate_id, forms[form].id))
        return forms[form].title;
  } else if (!strcmp(metadata->mapping_version, "pip-proof-map-v1")) {
    if (!strcmp(metadata->candidate_id, "legacy-carried"))
      return "Plain coat / pale variation carried";
    if (!strcmp(metadata->candidate_id, "legacy-marked"))
      return "Pale markings";
  }
  return NULL;
}

static void focus(SelectedRow *row, int x, int y, int width, int height) {
  core_art_focus_row(x, y, width, height, row->y, SELECTED_LAB_WIDTH, row->pixels);
}

static unsigned known_topics(const SelectedLab *lab) {
  unsigned count = 0;
  for (unsigned sample = 0; sample < lab->game.sample_count; ++sample) {
    SelectedResearchView view;
    if (selected_lab_research_view(lab, sample, &view))
      count += view.completed_methods;
  }
  return count;
}

static const char *research_topic(const SelectedResearchMethod *entry) {
  if (!strcmp(entry->id, "heritage")) return "Inheritance";
  if (!strcmp(entry->id, "movement")) return "Movement";
  if (!strcmp(entry->id, "coat-comparison")) return "Coat";
  if (!strcmp(entry->id, "effort-comparison")) return "Effort";
  if (!strcmp(entry->id, "form.crown")) return "Crown";
  if (!strcmp(entry->id, "appearance.rings")) return "Eye rings";
  if (!strcmp(entry->id, "appearance.markings")) return "Markings";
  if (!strcmp(entry->id, "movement.drive")) return "Movement";
  if (!strcmp(entry->id, "movement.efficiency")) return "Effort";
  return entry->title;
}

static const char *research_purpose(const SelectedResearchMethod *entry) {
  if (!strcmp(entry->id, "heritage")) return "Investigate inheritance.";
  if (!strcmp(entry->id, "movement")) return "Investigate movement and effort.";
  if (!strcmp(entry->id, "coat-comparison")) return "Resolve how pale variation can show.";
  if (!strcmp(entry->id, "effort-comparison")) return "Compare energy use for the same walking action.";
  return "Investigate this reference feature.";
}

static void research_summary(SelectedRow *row, const SelectedLab *lab,
                              unsigned sample, int x, int y, int width) {
  SelectedResearchView view;
  if (!selected_lab_research_view(lab, sample, &view))
    return;
  char known[160] = "Known: ", missing[160] = "Still: ";
  unsigned known_count = 0, missing_count = 0;
  const char *suggested = NULL;
  for (unsigned method = 0; method < view.method_count; ++method) {
    SelectedResearchMethod entry;
    if (!selected_lab_research_method(lab, sample, method, &entry)) continue;
    char *line = entry.known ? known : missing;
    unsigned *count = entry.known ? &known_count : &missing_count;
    size_t used = strlen(line);
    snprintf(line + used, 160 - used, "%s%s", (*count)++ ? " / " : "", research_topic(&entry));
    if (!suggested && entry.useful && !entry.known) suggested = research_topic(&entry);
  }
  if (!known_count) strcpy(known, "Known: no findings yet");
  if (view.complete) strcpy(known, "Known: complete supported form");
  label(row, x, y, known, 18, SAGE);
  label(row, x, y + 22, view.complete ? "No unresolved reference knowledge." : missing, 18, MUTED);
  char next[112];
  if (lab->game.samples[sample].incubated)
    strcpy(next, "Sample used / research record stays.");
  else if (view.complete)
    strcpy(next, "Choose a supported form.");
  else if (suggested)
    snprintf(next, sizeof(next), "Suggested: %s", suggested);
  else
    strcpy(next, "Inspect your recorded findings.");
  wrapped_label(row, x, y + 44, next, 18, INK, width);
}

static void knowledge_rows(SelectedRow *row, const SelectedLab *lab,
                            unsigned sample, const SelectedResearchView *view,
                            int x, int y) {
  for (unsigned method = 0; method < view->method_count; ++method) {
    SelectedResearchMethod entry;
    if (!selected_lab_research_method(lab, sample, method, &entry)) continue;
    int yy = y + (int)method * 31;
    rectangle(row, x, yy, 3, 23, entry.known ? SAGE : EDGE);
    label(row, x + 13, yy, research_topic(&entry), 18, INK);
    label(row, x + 260, yy, entry.known ? "Known" : "Still to learn", 18, MUTED);
  }
  if (view->partial_p)
    label(row, x, y + 111, "Pale variation known / appearance unresolved", 18, MUTED);
}

static unsigned revealed_residents(const GameState *game) {
  unsigned count = 0;
  for (unsigned index = 0; index < game->individual_count; ++index)
    count += game->individuals[index].revealed;
  return count;
}

static unsigned preview_resident(const SelectedLab *lab) {
  const GameState *game = &lab->game;
  if (lab->resident < game->individual_count &&
      game->individuals[lab->resident].revealed)
    return lab->resident;
  for (unsigned index = 0; index < game->individual_count; ++index)
    if (game->individuals[index].revealed)
      return index;
  return 0;
}

static void progress(SelectedRow *row, int x, int y, int width,
                     unsigned elapsed, unsigned duration) {
  rectangle(row, x, y, width, 17, DEEP);
  outline(row, x, y, width, 17, 2, EDGE);
  if (elapsed > duration)
    elapsed = duration;
  rectangle(row, x + 3, y + 3, (width - 6) * (int)elapsed / (int)duration, 11,
            WARM);
}

/* The artist's final pixels are rendered at 1:1. Destination artwork is a
 * reusable metaphor; the adjacent live readouts alone describe game state. */
static void overview_sprite(SelectedRow *row, unsigned asset, int x, int y) {
  int source_y = (int)row->y - y;
  if (asset >= OVERVIEW_SPRITE_COUNT || source_y < 0 ||
      source_y >= OVERVIEW_SPRITE_HEIGHT)
    return;
  for (int column = 0; column < OVERVIEW_SPRITE_WIDTH; ++column) {
    int destination = x + column;
    if (destination < 0 || destination >= (int)SELECTED_LAB_WIDTH)
      continue;
    const uint8_t *source = overview_pixels[asset] +
                            (source_y * OVERVIEW_SPRITE_WIDTH + column) * 4;
    uint8_t *target = row->pixels + destination * 3;
    unsigned alpha = source[3];
    for (unsigned channel = 0; channel < 3; ++channel)
      target[channel] = (uint8_t)((source[channel] * alpha +
                                   target[channel] * (255u - alpha) + 127u) /
                                  255u);
  }
}

static void home_overview(SelectedRow *row, const SelectedLab *lab,
                          const SelectedLabRenderContext *context) {
  const GameState *game = &lab->game;
  char text[96];
  unsigned topics = known_topics(lab);
  unsigned residents = revealed_residents(game);
  overview_sprite(row, OVERVIEW_EXPLORE, 272, 207);
  overview_sprite(row, OVERVIEW_RESEARCH, 642, 207);
  overview_sprite(row, OVERVIEW_INCUBATOR, 272, 377);
  overview_sprite(row, OVERVIEW_HABITAT, 642, 377);

  label(row, 422, 223, "EXPLORE", 18, MUTED);
  heading(row, 422, 255,
          context && context->haul == SELECTED_HAUL_WAITING ? "Supplies waiting"
          : context && context->haul == SELECTED_HAUL_STORED ? "Supplies stored"
          : lab->kit_mode ? "Received records"
          : game->expedition_active ? "Gathering"
          : game->expedition_id[0]
              ? (game_transfer_available(game) ? "Haul ready" : "Paused")
              : "At the Lab",
          26, INK);
  if (context && context->haul != SELECTED_HAUL_NONE) {
    label(row, 422, 292, context->haul == SELECTED_HAUL_WAITING
              ? "Open Explore to accept" : "Expedition ended", 18, MUTED);
    unsigned total = context->incoming[0] + context->incoming[1] + context->incoming[2];
    snprintf(text, sizeof(text), "%u incoming unit%s", total / GAME_SUPPLY_UNIT,
             total == GAME_SUPPLY_UNIT ? "" : "s");
    label(row, 422, 318, context->haul == SELECTED_HAUL_WAITING
              ? text : "Stored in Lab stock", 18, MUTED);
  } else if (lab->kit_mode) {
    snprintf(text, sizeof(text), "%u received outing%s", game->received_count,
             game->received_count == 1 ? "" : "s");
    label(row, 422, 292, text, 18, MUTED);
    label(row, 422, 318, "Open the expedition log", 18, MUTED);
  } else if (game->expedition_id[0]) {
    snprintf(text, sizeof(text), "%u / 60 seconds", game->expedition_elapsed);
    label(row, 422, 292, text, 18, MUTED);
    label(row, 422, 318,
          game->expedition_active         ? "Expedition active"
          : game_transfer_available(game) ? "Ready to return"
          : game->expedition_elapsed < GAME_EXPEDITION_SECONDS
              ? "Continue on Companion"
              : "Finish on Companion",
          18, MUTED);
  } else {
    label(row, 422, 292, "No expedition", 18, MUTED);
    label(row, 422, 318, "Choose a route", 18, MUTED);
  }

  label(row, 792, 223, "RESEARCH", 18, MUTED);
  snprintf(text, sizeof(text), "%u sample%s", game->sample_count,
           game->sample_count == 1 ? "" : "s");
  heading(row, 792, 255, text, 26, INK);
  snprintf(text, sizeof(text), "%u finding%s", topics, topics == 1 ? "" : "s");
  label(row, 792, 292, text, 18, MUTED);
  if (game->sample_count && lab->sample < game->sample_count)
    label(row, 792, 318, game->samples[lab->sample].id, 18, MUTED);
  else
    label(row, 792, 318, "Awaiting samples", 18, MUTED);

  rectangle(row, 276, 361, 696, 1, EDGE);
  label(row, 422, 391, "INCUBATOR", 18, MUTED);
  heading(row, 422, 423,
          game->incubation_ready    ? "Ready to open"
          : game->incubation_active ? "Incubating"
                                    : "Resting",
          26, INK);
  if (game->incubation_active || game->incubation_ready) {
    snprintf(text, sizeof(text), "%u / %u seconds", game->incubation_elapsed,
             GAME_INCUBATION_SECONDS);
    label(row, 422, 460, text, 18, MUTED);
    label(row, 422, 486, "Of active play", 18, MUTED);
  } else
    label(row, 422, 460, "No incubation", 18, MUTED);

  label(row, 792, 391, "HABITAT", 18, MUTED);
  snprintf(text, sizeof(text), "%u resident%s", residents,
           residents == 1 ? "" : "s");
  heading(row, 792, 423, text, 26, INK);
  if (residents) {
    const GameIndividual *individual =
        &game->individuals[preview_resident(lab)];
    label(row, 792, 460, individual->id, 18, MUTED);
    snprintf(text, sizeof(text), "%u visit%s together", individual->care_visits,
             individual->care_visits == 1 ? "" : "s");
    label(row, 792, 486, text, 18, MUTED);
  } else
    label(row, 792, 460, "No Beecho revealed", 18, MUTED);
}

static void landing_strip(SelectedRow *row, const char *text) {
  rectangle(row, 320, 448, 622, 2, EDGE);
  rectangle(row, 320, 459, 622, 44, DEEP);
  rectangle(row, 320, 459, 3, 44, BLUE);
  label(row, 334, 469, text, 18, MUTED);
}

static void home_landing(SelectedRow *row, const SelectedLab *lab,
                         const SelectedLabRenderContext *context) {
  const GameState *game = &lab->game;
  char text[96];
  if (lab->focus == 0) {
    home_overview(row, lab, context);
    return;
  }
  if (lab->focus == 1 && context && context->haul != SELECTED_HAUL_NONE) {
    heading(row, 320, 218, context->haul == SELECTED_HAUL_WAITING
                ? "SUPPLIES WAITING AT THE LAB" : "SUPPLIES STORED AT THE LAB",
            32, INK);
    label(row, 320, 272, context->haul == SELECTED_HAUL_WAITING
              ? "Open Explore to accept this haul." : "Expedition ended.", 22, INK);
    const char *names[] = {"DATA", "ENERGY", "ESSENCE"};
    for (unsigned i = 0; i < 3; ++i) {
      int x = 324 + (int)i * 215;
      sprite(row, i, x, 327, 90, 100);
      label(row, x + 101, 345, names[i], 18, MUTED);
      snprintf(text, sizeof(text), "%u", context->incoming[i] / GAME_SUPPLY_UNIT);
      heading(row, x + 101, 375, text, 26, INK);
    }
    landing_strip(row, context->haul == SELECTED_HAUL_WAITING
                   ? "Incoming supplies are separate from Lab stock."
                   : "The accepted haul is included in Lab stock.");
    return;
  }
  if (lab->focus == 1) {
    if (lab->kit_mode) {
      heading(row, 320, 218, "RECEIVED EXPEDITIONS", 32, INK);
      snprintf(text, sizeof(text), "%u expedition record%s received", game->received_count,
               game->received_count == 1 ? "" : "s");
      label(row, 320, 285, text, 22, INK);
      overview_sprite(row, OVERVIEW_EXPLORE, 330, 335);
      label(row, 500, 360, "Recorded routes and findings", 22, INK);
      label(row, 500, 400, "Accepted supplies and samples", 18, MUTED);
      landing_strip(row, "Confirm: open the received expedition log");
      return;
    }
    heading(row, 320, 218,
            game->expedition_active  ? "EXPEDITION GATHERING"
            : game->expedition_id[0] ? "HAUL READY TO RETURN"
                                     : "NO EXPEDITION ACTIVE",
            32, game->expedition_id[0] ? WARM : INK);
    if (!game->expedition_id[0]) {
      label(row, 320, 285, "Choose an expedition route.", 22, INK);
      label(row, 320, 335, "Possible finds", 18, MUTED);
      const char *finds[] = {"DATA", "ENERGY", "ESSENCE"};
      for (unsigned index = 0; index < 3; ++index) {
        int x = 330 + (int)index * 188;
        sprite(row, index, x, 374, 40, 53);
        label(row, x + 50, 392, finds[index], 18, INK);
      }
    } else {
      snprintf(text, sizeof(text), "%u / 60 s of active play",
               game->expedition_elapsed);
      label(row, 320, 277, text, 22, INK);
      progress(row, 320, 320, 622, game->expedition_elapsed, 60);
      const unsigned cargo[] = {game->expedition_data, game->expedition_energy,
                                game->expedition_essence};
      const char *names[] = {"DATA", "ENERGY", "ESSENCE"};
      for (unsigned index = 0; index < 3; ++index) {
        int x = 324 + (int)index * 215;
        sprite(row, index, x, 374, 40, 53);
        label(row, x + 51, 375, names[index], 18, MUTED);
        snprintf(text, sizeof(text), "%u units",
                 cargo[index] / GAME_SUPPLY_UNIT);
        label(row, x + 51, 405, text, 18, INK);
      }
    }
    if (game->expedition_id[0]) {
      unsigned cargo = game->expedition_data + game->expedition_energy +
                       game->expedition_essence;
      snprintf(text, sizeof(text), "Collected %u / 40 units",
               cargo / GAME_SUPPLY_UNIT);
      landing_strip(row, text);
    } else
      landing_strip(row, "No cargo loaded");

  } else if (lab->focus == 2) {
    overview_sprite(row, OVERVIEW_RESEARCH, 326, 247);
    snprintf(text, sizeof(text), "%u retained sample%s", game->sample_count,
             game->sample_count == 1 ? "" : "s");
    heading(row, 505, 255, text, 32, INK);
    snprintf(text, sizeof(text), "%u findings recorded", known_topics(lab));
    label(row, 505, 306, text, 22, INK);
    label(row, 505, 360, "Choose a sample in Research.", 18, MUTED);
    landing_strip(row, game->sample_count ? "Each sample has its own evidence and next investigations."
                                          : "Bring a sample to the Lab to begin.");
  } else if (lab->focus == 3) {
    if (game->incubation_active || game->incubation_ready)
      sprite(row, SPRITE_SAMPLE, 326, 247, 155, 155);
    heading(row, game->incubation_active || game->incubation_ready ? 505 : 320,
            245,
            game->incubation_ready    ? "READY TO OPEN"
            : game->incubation_active ? "INCUBATING"
                                      : "NO INCUBATION",
            32, game->incubation_active || game->incubation_ready ? WARM : INK);
    if (game->incubation_active || game->incubation_ready) {
      snprintf(text, sizeof(text), "%u / %u s of active play",
               game->incubation_elapsed, GAME_INCUBATION_SECONDS);
      label(row, 505, 303, text, 22, INK);
      progress(row, 505, 351, 435, game->incubation_elapsed,
               GAME_INCUBATION_SECONDS);
      if (game->incubation_sample < game->sample_count) {
        snprintf(text, sizeof(text), "Source: %s",
                 game->samples[game->incubation_sample].id);
        label(row, 505, 395, text, 18, MUTED);
      }
    } else {
      int prepared = 0;
      for (unsigned sample = 0; sample < game->sample_count; ++sample)
        {
          SelectedResearchView view;
          prepared |= selected_lab_research_view(lab, sample, &view) &&
                      view.complete && !game->samples[sample].incubated;
        }
      label(row, 320, 303,
            prepared ? "A researched sample is ready to prepare."
                     : "Complete the supported reference knowledge first.",
            22, INK);
      landing_strip(
          row, prepared
                   ? "Choose the sample in Research to see its requirements."
                   : "Prepare incubation from Research.");
    }

  } else if (lab->focus == 4) {
    unsigned count = revealed_residents(game);
    if (count) {
      unsigned selected = preview_resident(lab);
      const GameIndividual *individual = &game->individuals[selected];
      resident_portrait(row, game, selected, 312, 220);
      heading(row, 585, 255, individual->id, 26, WARM);
      label(row, 585, 310,
            individual->expression.pale_markings ? "Pale markings"
                                                 : "Plain coat",
            22, INK);
      snprintf(text, sizeof(text), "%u revealed resident%s", count,
               count == 1 ? "" : "s");
      label(row, 585, 356, text, 18, MUTED);
      snprintf(text, sizeof(text), "%u visit%s together",
               individual->care_visits,
               individual->care_visits == 1 ? "" : "s");
      label(row, 585, 395, text, 18, INK);
    } else {
      heading(row, 320, 255, "NO REVEALED RESIDENTS", 32, INK);
      label(row, 320, 314, "A Beecho appears here after reveal.", 22, INK);
    }
  }
}

void selected_lab_row_with_context(const SelectedLab *lab,
                                  const SelectedLabRenderContext *context,
                                  unsigned y,
                                  uint8_t pixels[SELECTED_LAB_WIDTH * 3]) {
  SelectedRow row = {y, pixels};
  const GameState *game = &lab->game;
  int home = lab->page == V1_HOME;
  rectangle(&row, 0, 0, 1024, 600, BASE);
  panel(&row, 24, home ? 24 : 18, 976, home ? 100 : 94);
  /* The shared Home header needs breathing room for the three-line stock
   *
   * readout. Retain its outer stepped frame without an internal crossing rule.
   */
  if (home)
    rectangle(&row, 37, 37, 950, 75, PANEL);
  heading(&row, 46, home ? 36 : 32, "BEECHO LAB", 34, INK);
  label(&row, 47, home ? 84 : 77, "LAB STOCK", 18, MUTED);
  unsigned stock[] = {game->data, game->energy, game->essence};
  const char *names[] = {"DATA", "ENERGY", "ESSENCE"};
  for (unsigned i = 0; i < 3; i++) {
    int x = home ? 402 + (int)i * 196 : 420 + (int)i * 180;
    sprite(&row, i, x, home ? 41 : 35, 50, 58);
    label(&row, x + (home ? 62 : 53), home ? 45 : 32, names[i], 18, MUTED);
    if (home) {
      char amount[32];
      snprintf(amount, sizeof(amount), "%u", stock[i] / 100);
      heading(&row, x + 62, 64, amount, 26, INK);
      label(&row, x + 62, 87, stock[i] == GAME_SUPPLY_UNIT ? "unit" : "units",
            18, MUTED);
    } else
      stock_amount(&row, x + 54, stock[i]);
  }
  panel(&row, 24, home ? 140 : 134, home ? 208 : 330, home ? 416 : 406);
  panel(&row, home ? 248 : 376, home ? 140 : 134, home ? 752 : 624,
        home ? 416 : 406);
  if (home && lab->focus == 0)
    rectangle(&row, 264, 156, 720, 384, ART_FIELD);
  const char *titles[] = {"WORKBENCH", "EXPEDITION",        "CARGO",
                          "SAMPLES",   "RESEARCH",          "DISCOVERY",
                          "SUPPORTED FORMS",  "INCUBATOR",         "HELLO, BEECHO",
                          "HABITAT",   "RESEARCH PLAN",     "DISCARD ITEMS",
                          "RESIDENTS",  "RECORDED FINDINGS", "SAMPLE FINDING",
                          "START INCUBATION?"};
  static const char *home_headings[] = {
      "Overview - Lab", "Overview - Explore", "Overview - Research",
      "Overview - Incubator", "Overview - Habitat"};
  SelectedResearchMethod current_method;
  int finding_page = lab->page == V1_FINDING || lab->page == V1_LIBRARY_FINDING;
  int form_finding = 0;
  const char *page_title = finding_page &&
      selected_lab_research_method(lab, lab->sample, lab->study, &current_method)
          ? current_method.title : titles[lab->page];
  heading(&row, home ? 276 : 394, home ? 164 : 152,
          home ? home_headings[lab->focus % 5]
          : page_title,
          34, INK);
  unsigned count = selected_lab_options(lab);
  unsigned first = lab->focus >= 6 ? lab->focus - 5 : 0;
  for (unsigned i = first; i < count && i < first + 6; i++) {
    int yy = home ? 185 + (int)i * 68
             : lab->page == V1_CREATE ? 159 + (int)(i - first) * 110
                                      : 159 + (int)(i - first) * 58;
    if (i == lab->focus) {
      int x = home ? 36 : 39;
      int width = home ? 184 : 299;
      int height = home ? 54 : lab->page == V1_CREATE ? 98 : 46;
      if (home)
        outline(&row, x - 4, yy - 8, width + 8, height + 8, 3, FOCUS_GLOW);
      rectangle(&row, x, yy - 4, width, height, PANEL);
      focus(&row, x - 2, yy - 6, width + 4, height + 4);
    }
    unsigned sample, study;
    if (lab->page == V1_LIBRARY &&
        selected_lab_library_entry(lab, i, &sample, &study)) {
      label(&row, 53, yy, game->samples[sample].id, 18,
            i == lab->focus ? WARM : MUTED);
      SelectedResearchMethod entry;
      selected_lab_research_method(lab, sample, study, &entry);
      label(&row, 53, yy + 23, entry.title, 18,
            i == lab->focus ? WARM : INK);
    } else if (lab->page == V1_CREATE) {
      wrapped_label(&row, 53, yy + 10, selected_lab_option(lab, i), 18,
                    i == lab->focus ? WARM : INK, 273);
    } else {
      label(&row, home ? 56 : 53, yy + (home ? 10 : 7),
            selected_lab_option(lab, i), home ? 22 : 18,
            i == lab->focus ? WARM : INK);
    }
  }
  char text[100];
  if (lab->page == V1_HOME) {
    home_landing(&row, lab, context);
  } else if (lab->page == V1_EXPEDITION || lab->page == V1_CARGO) {
    label(&row, 402, 205,
          game->expedition_active ? "PROBE / GATHERING"
          : game->expedition_id[0]
              ? (game->expedition_elapsed >= 60
                     ? "EXPEDITION COMPLETE / RETURN WITH HAUL"
                     : "EXPEDITION PAUSED / CONTINUE OR SEND")
              : "CHOOSE YOUR EXPEDITION",
          24, WARM);
    unsigned cargo[] = {game->expedition_data, game->expedition_energy,
                        game->expedition_essence};
    for (unsigned i = 0; i < 3; i++) {
      int x = 416 + (int)i * 179;
      sprite(&row, i, x + 25, 265, 65, 86);
      snprintf(text, sizeof(text), "%u units", cargo[i] / 100);
      label(&row, x + 21, 367, text, 28, INK);
    }
    snprintf(text, sizeof(text), "%u / 60 s  |  Sample: %s",
             game->expedition_elapsed,
             game->sample_count >= GAME_MAX_SAMPLES ? "shelf full"
             : game->expedition_elapsed >= 60       ? "found"
                                                    : "scanning");
    label(&row, 411, 443, text, 18, MUTED);
    unsigned total = cargo[0] + cargo[1] + cargo[2];
    snprintf(text, sizeof(text), "Collected %u / 40 units",
             total / GAME_SUPPLY_UNIT);
    label(&row, 415, 503, text, 18, MUTED);
    outline(&row, 411, 470, 550, 20, 2, EDGE);
    rectangle(&row, 415, 474, (int)(542 * (total > 4000 ? 4000 : total) / 4000),
              12, BLUE);
  } else if (lab->page == V1_SAMPLES) {
    if (lab->focus == 0 || lab->focus > game->sample_count) {
      overview_sprite(&row, OVERVIEW_RESEARCH, 420, 247);
      snprintf(text, sizeof(text), "%u sample%s retained", game->sample_count,
               game->sample_count == 1 ? "" : "s");
      heading(&row, 588, 256, text, 32, INK);
      unsigned awaiting = 0, ready = 0, used = 0;
      for (unsigned sample = 0; sample < game->sample_count; ++sample) {
        SelectedResearchView view;
        if (game->samples[sample].incubated) ++used;
        else if (selected_lab_research_view(lab, sample, &view) && view.complete) ++ready;
        else ++awaiting;
      }
      snprintf(text, sizeof(text), "%u awaiting research", awaiting);
      label(&row, 588, 314, text, 22, INK);
      snprintf(text, sizeof(text), "%u ready to prepare", ready);
      label(&row, 588, 354, text, 22, INK);
      snprintf(text, sizeof(text), "%u used / records retained", used);
      label(&row, 588, 394, text, 22, INK);
      label(&row, 420, 478, "Select a sample to see its findings.", 18, MUTED);
    } else {
      unsigned sample = lab->focus - 1;
      SelectedResearchView view;
      sprite(&row, SPRITE_SAMPLE, 424, 223, 54, 54);
      label(&row, 500, 230, game->samples[sample].id, 22, INK);
      if (selected_lab_research_view(lab, sample, &view)) {
        knowledge_rows(&row, lab, sample, &view, 416, 300);
        research_summary(&row, lab, sample, 416, 449, 535);
      }
    }
  } else if (lab->page == V1_STUDIES && lab->focus >= selected_lab_options(lab) - 1) {
    SelectedResearchView view;
    overview_sprite(&row, OVERVIEW_RESEARCH, 423, 252);
    label(&row, 588, 241, game->samples[lab->sample].id, 22, INK);
    if (selected_lab_research_view(lab, lab->sample, &view)) {
      wrapped_label(&row, 588, 300, view.complete ? "Complete supported forms are ready to compare."
                                                 : "Some reference knowledge is still unresolved.", 22, INK, 360);
      research_summary(&row, lab, lab->sample, 416, 449, 535);
    }
  } else if (lab->page == V1_DISCARD_REVIEW) {
    unsigned values[] = {game->expedition_data, game->expedition_energy,
                         game->expedition_essence};
    sprite(&row, lab->discard_resource, 600, 250, 96, 128);
    snprintf(text, sizeof(text), "Discard 10 %s items?",
             names[lab->discard_resource]);
    label(&row, 420, 412, text, 24, WARM);
    label(&row, 420, 462,
          values[lab->discard_resource] >= 1000
              ? "This frees 10 cargo units. It cannot be recovered."
              : "Fewer than 10 whole items of this kind.",
          18, INK);
  } else if (lab->page == V1_STUDIES || lab->page == V1_FINDING ||
             lab->page == V1_LIBRARY_FINDING || lab->page == V1_STUDY_REVIEW) {
    unsigned method = lab->page == V1_STUDIES ? lab->focus : lab->study;
    SelectedResearchMethod entry;
    SelectedResearchView view;
    if (selected_lab_research_method(lab, lab->sample, method, &entry) &&
        selected_lab_research_view(lab, lab->sample, &view)) {
      PipSupportedCandidate carried, marked;
      int illustrated_forms = finding_page && !strcmp(entry.id, "coat-comparison") &&
          view.complete && selected_lab_candidate(lab, lab->sample, 0, &carried) &&
          selected_lab_candidate(lab, lab->sample, 1, &marked);
      label(&row, 416, illustrated_forms ? 195 : 203,
            game->samples[lab->sample].id, 18, MUTED);
      if (lab->page == V1_FINDING || lab->page == V1_LIBRARY_FINDING) {
        if (view.legacy && method < 2) {
          label(&row, 416, 249, "RECORDED FINDING", 22, SAGE);
          sprite(&row, method == 1 ? SPRITE_EYE_RING : SPRITE_CROWN, 421, 300, 115, 115);
          label(&row, 416, 425, "Reference feature", 18, MUTED);
          wrapped_label(&row, 578, 293, entry.finding ? entry.finding : "No finding disclosed.", 22, INK, 374);
          research_summary(&row, lab, lab->sample, 416, 449, 535);
        } else {
          if (illustrated_forms) {
            form_finding = 1;
            /* Complete supported alternatives, never an early founder reveal. */
            label(&row, 416, 219, "Complete reference / no unresolved knowledge", 18, SAGE);
            label(&row, 416, 247, "Plain coat / pale carried", 18, INK);
            label(&row, 700, 247, "Pale markings / expressed", 18, INK);
            form_preview(&row, SELECTED_SPRITE_COUNT + carried.expression.pale_markings,
                         456, 274);
            form_preview(&row, SELECTED_SPRITE_COUNT + marked.expression.pale_markings,
                         740, 274);
            wrapped_label(&row, 416, 482,
                          entry.finding ? entry.finding : "No finding disclosed.",
                          18, INK, 552);
            label(&row, 416, 510,
                  game->samples[lab->sample].incubated
                      ? "Sample used / research record stays."
                      : "Choose a supported form.",
                  16, MUTED);
          } else {
            CoreArtId context = (!strcmp(entry.id, "movement") ||
                                 !strcmp(entry.id, "movement.drive")) ? CORE_ART_RESEARCH_MOVEMENT
                : (!strcmp(entry.id, "effort-comparison") ||
                   !strcmp(entry.id, "movement.efficiency")) ? CORE_ART_RESEARCH_EFFORT
                                                        : CORE_ART_RESEARCH_INHERITANCE;
            const CoreArtSprite *illustration = core_art_sprite(context);
            /* Blank evidence boards depict tools, not this sample's results. */
            label(&row, 416, 225, "Research context", 18, MUTED);
            core_art_row(context, 416 + (184 - (int)illustration->width) / 2,
                         249 + (195 - (int)illustration->height) / 2,
                         row.y, SELECTED_LAB_WIDTH, row.pixels);
            wrapped_label(&row, 615, 249,
                          entry.finding ? entry.finding : "No finding disclosed.",
                          22, INK, 342);
            if (!strcmp(entry.id, "coat-comparison") && entry.finding) {
              rectangle(&row, 615, 337, 160, 93, ART_FIELD);
              rectangle(&row, 794, 337, 166, 93, ART_FIELD);
              label(&row, 626, 345, "Plain coat", 18, INK);
              wrapped_label(&row, 626, 374, "Pale variation carried", 18, MUTED, 137);
              label(&row, 805, 345, "Pale markings", 18, INK);
              wrapped_label(&row, 805, 374, "Appearance expressed", 18, MUTED, 143);
            }
            research_summary(&row, lab, lab->sample, 416, 449, 535);
          }
        }
      } else {
        heading(&row, 416, 243, entry.title, 26, INK);
        label(&row, 416, 283, entry.known ? "Recorded / inspect freely"
                            : research_purpose(&entry), 18, MUTED);
        overview_sprite(&row, OVERVIEW_RESEARCH, 416, 317);
        const unsigned costs[] = {entry.cost_data, entry.cost_energy, entry.cost_essence};
        for (unsigned i = 0; i < 3; ++i) {
          int yy = 315 + (int)i * 53;
          sprite(&row, i, 589, yy, 48, 53);
          snprintf(text, sizeof(text), "%u / %u", entry.known || !entry.useful ? 0 : costs[i] / GAME_SUPPLY_UNIT,
                   stock[i] / GAME_SUPPLY_UNIT);
          label(&row, 655, yy + 15, text, 22, INK);
        }
        label(&row, 791, 321, "Cost / Lab stock", 18, MUTED);
        wrapped_label(&row, 416, 485, entry.known || !entry.useful ? "Confirm inspects recorded knowledge."
            : lab->page == V1_STUDY_REVIEW ? "Start research spends the listed resources."
                                          : "Confirm reviews this investigation.", 18, INK, 535);
      }
    }
  } else if (lab->page == V1_LIBRARY) {
    unsigned sample, study;
    int discoveries = selected_lab_library_entry(lab, 0, &sample, &study);
    label(&row, 416, 252,
          discoveries ? "Your recorded discoveries" : "No discoveries yet",
          24, INK);
    label(&row, 416, 310, "Research topics to record findings here.", 22,
          MUTED);
    label(&row, 416, 366, discoveries ? "Select a finding to inspect it freely."
                                         : "Return to research to begin.", 22, INK);
  } else if (lab->page == V1_CREATE || lab->page == V1_CREATE_REVIEW) {
    int review = lab->page == V1_CREATE_REVIEW;
    PipSupportedCandidate candidate;
    int disclosed = review ? selected_lab_creation_draft(lab, &candidate)
                            : selected_lab_candidate(lab, lab->sample, lab->focus, &candidate);
    label(&row, 416, 208, game->samples[lab->sample].id, 18, MUTED);
    if (disclosed) {
      sprite(&row, SELECTED_SPRITE_COUNT + candidate.expression.pale_markings,
             405, 235, 261, 289);
      label(&row, 689, 238, review ? "SELECTED FORM" : "SUPPORTED PREVIEW", 18, SAGE);
      wrapped_label(&row, 689, 275, candidate.title, 22, INK, 278);
      snprintf(text, sizeof(text), "Reference: %s", candidate.id);
      label(&row, 689, 346, text, 18, MUTED);
      label(&row, 689, 378, "Cost: 5 of each supply", 18, INK);
      if (review)
        wrapped_label(&row, 689, 409, "Uses this sample; research record stays.",
                      18, INK, 278);
      else {
        label(&row, 689, 409, "Drafting spends nothing.", 18, INK);
        label(&row, 689, 435, "Confirm: review form", 18, INK);
      }
      unsigned shortage = 0;
      for (unsigned i = 0; i < 3; ++i)
        if (stock[i] < 500) {
          snprintf(text, sizeof(text), "%s: %u / 5", names[i], stock[i] / GAME_SUPPLY_UNIT);
          label(&row, 689, 460 + (int)shortage * 22, text, 18, WARM);
          ++shortage;
        }
    } else {
      overview_sprite(&row, OVERVIEW_RESEARCH, 423, 260);
      wrapped_label(&row, 588, 281, "Complete this sample's reference knowledge before choosing a form.", 22, INK, 360);
      research_summary(&row, lab, lab->sample, 416, 449, 535);
    }
  } else if (lab->page == V1_INCUBATION) {
    overview_sprite(&row, OVERVIEW_INCUBATOR, 597, 236);
    if (game->incubation_active) {
      label(&row, 417, 209, game->samples[game->incubation_sample].id, 18,
            MUTED);
      snprintf(text, sizeof(text), "%u / %u s active play",
               game->incubation_elapsed, GAME_INCUBATION_SECONDS);
      label(&row, 469, 458, text, 22, WARM);
    }
    label(&row, 424, 406,
          game->incubation_ready    ? "READY / OPEN WHEN YOU CHOOSE"
          : game->incubation_active ? "INCUBATING"
                                    : "No incubation yet",
          24, game->incubation_ready ? SAGE : INK);
  } else {
    int visible =
        game->individual_count && game->individuals[lab->resident].revealed;
    const GameIndividual *individual = &game->individuals[lab->resident];
    label(&row, 415, 210,
          visible                    ? individual->id
          : lab->page == V1_CRITTERS ? "No revealed residents yet"
                                     : "Your habitat awaits",
          22, INK);
    if (visible) {
      resident_portrait(&row, game, lab->resident, 411, 235);
      label(&row, 686, 248,
            individual->expression.pale_markings ? "Pale markings"
                                                 : "Plain coat",
            28, WARM);
      label(&row, 686, 297, "Crown frill / pale eye rings", 18, INK);
      const char *form_title = selected_lab_resident_form_title(
          individual, &game->individual_metadata[lab->resident]);
      label(&row, 686, 327, "Selected form", 18, MUTED);
      wrapped_label(&row, 686, 349,
                    form_title ? form_title : "Form reference unavailable",
                    22, INK, 278);
      label(&row, 686, 412, "Source sample", 18, MUTED);
      label(&row, 686, 436, individual->source_sample_id, 18, INK);
      if (lab->page == V1_HABITAT) {
        snprintf(text, sizeof(text), "Visits together: %u",
                 individual->care_visits);
        label(&row, 686, 487, text, 18, SAGE);
      } else if (lab->page == V1_REVEAL)
        label(&row, 686, 487, "Ready to meet you.", 18, SAGE);
    } else
      label(&row, 414, 439, "Research your first sample to begin.", 22, INK);
  }

  if (lab->page == V1_HOME && !lab->storage_error) {
    char footer[112];
    if (lab->focus == 0)
      strcpy(footer, "Up/down: preview workspaces");
    else
      snprintf(footer, sizeof(footer),
               "Up/down: preview | Confirm: enter %s | Back: Overview",
               selected_lab_focus(lab));
    label(&row, 30, 572, footer, 18, MUTED);
  } else {
    if (lab->message[0]) {
      /* Feedback stays in the workpiece without hiding a saved form finding. */
      if (form_finding) {
        rectangle(&row, 416, 508, 560, 23, ART_FIELD);
        label(&row, 416, 510, lab->message, 16,
              lab->storage_error ? WARM : INK);
      } else {
        rectangle(&row, 416, 480, 560, 49, ART_FIELD);
        wrapped_label(&row, 416, 486, lab->message, 18,
                      lab->storage_error ? WARM : INK, 552);
      }
    }
    label(&row, 30, 557,
          "Up/down: focus | Right: inspect | Confirm: act | Back: return",
          18, MUTED);
  }
}

void selected_lab_row(const SelectedLab *lab, unsigned y,
                      uint8_t pixels[SELECTED_LAB_WIDTH * 3]) {
  selected_lab_row_with_context(lab, NULL, y, pixels);
}

static int word(FILE *output, unsigned value, unsigned bytes) {
  for (unsigned index = 0; index < bytes; ++index)
    if (fputc((int)((value >> (index * 8)) & 255u), output) == EOF)
      return 0;
  return 1;
}

int selected_lab_bmp(const SelectedLab *lab, FILE *output) {
  const unsigned stride = SELECTED_LAB_WIDTH * 3;
  uint8_t pixels[SELECTED_LAB_WIDTH * 3];
  if (fwrite("BM", 1, 2, output) != 2)
    return 0;
  unsigned fields[][2] = {{54 + stride * SELECTED_LAB_HEIGHT, 4},
                          {0, 4},
                          {54, 4},
                          {40, 4},
                          {SELECTED_LAB_WIDTH, 4},
                          {SELECTED_LAB_HEIGHT, 4},
                          {1, 2},
                          {24, 2},
                          {0, 4},
                          {stride * SELECTED_LAB_HEIGHT, 4},
                          {2835, 4},
                          {2835, 4},
                          {0, 4},
                          {0, 4}};
  for (unsigned index = 0; index < sizeof(fields) / sizeof(fields[0]); ++index)
    if (!word(output, fields[index][0], fields[index][1]))
      return 0;
  for (unsigned y = SELECTED_LAB_HEIGHT; y > 0; --y) {
    selected_lab_row(lab, y - 1, pixels);
    for (unsigned column = 0; column < SELECTED_LAB_WIDTH; ++column) {
      uint8_t swap = pixels[column * 3];
      pixels[column * 3] = pixels[column * 3 + 2];
      pixels[column * 3 + 2] = swap;
    }
    if (fwrite(pixels, 1, stride, output) != stride)
      return 0;
  }
  return !ferror(output);
}
