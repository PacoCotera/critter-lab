#include "native_ui.h"
#include "home_view.h"
#include "research_view.h"
#include "action_view.h"
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


void selected_lab_row_with_context(const SelectedLab *lab,
                                  const SelectedLabRenderContext *context,
                                  unsigned y,
                                  uint8_t pixels[SELECTED_LAB_WIDTH * 3]) {
  /* Home owns a retained LVGL tree and is exported through the frame API.
   * This legacy row API is intentionally unavailable for migrated pages. */
  if (lab->page == V1_HOME || selected_lab_is_research_page(lab->page) || selected_lab_is_action_page(lab->page)) { memset(pixels, 0, SELECTED_LAB_WIDTH * 3); return; }
  (void)context;
  SelectedRow row = {y, pixels};
  const GameState *game = &lab->game;
  rectangle(&row, 0, 0, 1024, 600, BASE);
  panel(&row, 24, 18, 976, 94);
  heading(&row, 46, 32, "BEECHO LAB", 34, INK);
  label(&row, 47, 77, "LAB STOCK", 18, MUTED);
  unsigned stock[] = {game->data, game->energy, game->essence};
  const char *names[] = {"DATA", "ENERGY", "ESSENCE"};
  for (unsigned i = 0; i < 3; i++) {
    int x = 420 + (int)i * 180;
    sprite(&row, i, x, 35, 50, 58);
    label(&row, x + 53, 32, names[i], 18, MUTED);
    stock_amount(&row, x + 54, stock[i]);
  }
  panel(&row, 24, 134, 330, 406);
  panel(&row, 376, 134, 624, 406);
  const char *titles[] = {"WORKBENCH", "EXPEDITION", "CARGO", "SAMPLES",
      "RESEARCH", "DISCOVERY", "SUPPORTED FORMS", "INCUBATOR", "HELLO, BEECHO",
      "HABITAT", "RESEARCH PLAN", "DISCARD ITEMS", "RESIDENTS",
      "RECORDED FINDINGS", "SAMPLE FINDING", "START INCUBATION?"};
  SelectedResearchMethod current_method;
  int finding_page = lab->page == V1_FINDING || lab->page == V1_LIBRARY_FINDING;
  const char *page_title = finding_page &&
      selected_lab_research_method(lab, lab->sample, lab->study, &current_method)
          ? current_method.title : titles[lab->page];
  heading(&row, 394, 152, page_title, 34, INK);
  unsigned count = selected_lab_options(lab);
  unsigned first = lab->focus >= 6 ? lab->focus - 5 : 0;
  for (unsigned i = first; i < count && i < first + 6; i++) {
    int yy = 159 + (int)(i - first) * 58;
    if (i == lab->focus) {
      int x = 39;
      int width = 299;
      int height = 46;
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
    } else {
      label(&row, 53, yy + 7,
            selected_lab_option(lab, i), 18,
            i == lab->focus ? WARM : INK);
    }
  }
  char text[100];
  if (lab->page == V1_EXPEDITION || lab->page == V1_CARGO) {
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
  }

  {
    if (lab->message[0]) {
      rectangle(&row, 416, 480, 560, 49, ART_FIELD);
      wrapped_label(&row, 416, 486, lab->message, 18,
                    lab->storage_error ? WARM : INK, 552);
    }
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
  if (!lab || !output) return 0;
  NativeUiContext *context = NULL;
  const uint8_t *frame = NULL;
  if (lab->page == V1_HOME) {
    LabHomeView view;
    if (!selected_lab_home_view(lab, NULL, 0, &view)) return 0;
    context = native_ui_create_device(KIT_LAB);
    if (!context) return 0;
    frame = native_ui_home(context, &view);
    if (!frame) { native_ui_destroy(context); return 0; }
  }
  if (selected_lab_is_research_page(lab->page)) {
    LabResearchView view;
    if (!selected_lab_research_projection(lab, 0, &view)) return 0;
    context = native_ui_create_device(KIT_LAB);
    if (!context) return 0;
    frame = native_ui_research(context, &view);
    if (!frame) { native_ui_destroy(context); return 0; }
  }
  if (selected_lab_is_action_page(lab->page)) {
    LabActionView view;
    if (!selected_lab_action_projection(lab, 0, &view)) return 0;
    context = native_ui_create_device(KIT_LAB);
    if (!context) return 0;
    frame = native_ui_actions(context, &view);
    if (!frame) { native_ui_destroy(context); return 0; }
  }
  const unsigned stride = SELECTED_LAB_WIDTH * 3;
  uint8_t pixels[SELECTED_LAB_WIDTH * 3];
  if (fwrite("BM", 1, 2, output) != 2)
    goto failure;
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
      goto failure;
  for (unsigned y = SELECTED_LAB_HEIGHT; y > 0; --y) {
    if (frame) memcpy(pixels, frame + (y - 1) * stride, stride);
    else selected_lab_row(lab, y - 1, pixels);
    for (unsigned column = 0; column < SELECTED_LAB_WIDTH; ++column) {
      uint8_t swap = pixels[column * 3];
      pixels[column * 3] = pixels[column * 3 + 2];
      pixels[column * 3 + 2] = swap;
    }
    if (fwrite(pixels, 1, stride, output) != stride)
      goto failure;
  }
  { int success = !ferror(output); native_ui_destroy(context); return success; }
failure:
  native_ui_destroy(context);
  return 0;
}
