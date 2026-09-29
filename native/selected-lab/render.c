#include "assets.h"
#include "native_font.h"
#include "pip_art.h"
#include "selected_lab.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned y;
  uint8_t *pixels;
} SelectedRow;
enum { BASE, PANEL, INK, MUTED, BLUE, EDGE, DEEP, WARM, SAGE, ACTION };
static const uint8_t colors[][3] = {
    {25, 36, 43},    {35, 43, 48},   {214, 222, 226}, {183, 198, 205},
    {24, 143, 234},  {70, 140, 184}, {16, 26, 33},    {237, 197, 106},
    {163, 206, 159}, {41, 41, 34}};

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

static void stepped(SelectedRow *row, int x, int y, int width, int height,
                    int corner, unsigned color) {
  int position = (int)row->y - y;
  if (position < 0 || position >= height)
    return;
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

/* Source PNGs remain untouched. The renderer removes only matte pixels
 * connected to the perimeter and close to that source's corner color. */
static const uint8_t *sprite_matte(unsigned asset,
                                   const SelectedSprite *source) {
  static uint8_t *masks[SELECTED_SPRITE_COUNT + 2];
  static int prepared[SELECTED_SPRITE_COUNT + 2];
  if (prepared[asset])
    return masks[asset];
  prepared[asset] = 1;
  size_t count = (size_t)source->width * source->height;
  uint8_t *mask = calloc(count, 1);
  unsigned *queue = malloc(count * sizeof(*queue));
  if (!mask || !queue) {
    free(mask);
    free(queue);
    return NULL;
  }
  size_t head = 0, tail = 0;
  for (unsigned y = 0; y < source->height; ++y) {
    for (unsigned x = 0; x < source->width; ++x) {
      if (x && y && x + 1 < source->width && y + 1 < source->height)
        continue;
      unsigned pixel = y * source->width + x;
      int matte = 1;
      for (unsigned c = 0; c < 3; ++c)
        if (abs((int)source->pixels[pixel * 3 + c] - source->pixels[c]) > 14)
          matte = 0;
      if (matte && !mask[pixel]) {
        mask[pixel] = 1;
        queue[tail++] = pixel;
      }
    }
  }
  while (head < tail) {
    unsigned pixel = queue[head++], x = pixel % source->width,
             y = pixel / source->width;
    unsigned neighbors[4] = {
        x ? pixel - 1 : pixel, x + 1 < source->width ? pixel + 1 : pixel,
        y ? pixel - source->width : pixel,
        y + 1 < source->height ? pixel + source->width : pixel};
    for (unsigned n = 0; n < 4; ++n) {
      unsigned next = neighbors[n];
      if (mask[next])
        continue;
      int matte = 1;
      for (unsigned c = 0; c < 3; ++c)
        if (abs((int)source->pixels[next * 3 + c] - source->pixels[c]) > 14)
          matte = 0;
      if (matte) {
        mask[next] = 1;
        queue[tail++] = next;
      }
    }
  }
  free(queue);
  masks[asset] = mask;
  return mask;
}

static void sprite(SelectedRow *row, unsigned asset, int x, int y,
                   unsigned width, unsigned height) {
  const SelectedSprite *source =
      asset < SELECTED_SPRITE_COUNT
          ? &selected_sprites[asset]
          : &pip_sprites[asset - SELECTED_SPRITE_COUNT];
  const uint8_t *matte = sprite_matte(asset, source);
  unsigned fitted_width = width;
  unsigned fitted_height = width * source->height / source->width;
  if (fitted_height > height) {
    fitted_height = height;
    fitted_width = height * source->width / source->height;
  }
  x += (int)(width - fitted_width) / 2;
  y += (int)(height - fitted_height) / 2;
  if ((int)row->y < y || (int)row->y >= y + (int)fitted_height)
    return;
  unsigned source_y =
      (unsigned)((int)row->y - y) * source->height / fitted_height;
  for (unsigned column = 0; column < fitted_width; ++column) {
    int destination = x + (int)column;
    if (destination < 0 || destination >= (int)SELECTED_LAB_WIDTH)
      continue;
    unsigned source_x = column * source->width / fitted_width;
    if (matte && matte[source_y * source->width + source_x])
      continue;
    memcpy(row->pixels + destination * 3,
           source->pixels + (source_y * source->width + source_x) * 3, 3);
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

void selected_lab_row(const SelectedLab *lab, unsigned y,
                      uint8_t pixels[SELECTED_LAB_WIDTH * 3]) {
  SelectedRow row = {y, pixels};
  const GameState *game = &lab->game;
  rectangle(&row, 0, 0, 1024, 600, BASE);
  panel(&row, 24, 18, 976, 94);
  label(&row, 46, 32, "BEECHO LAB", 34, INK);
  label(&row, 47, 77, "FIELD / DISCOVERY / LIFE", 18, MUTED);
  unsigned stock[] = {game->data, game->energy, game->essence};
  const char *names[] = {"DATA", "ENERGY", "ESSENCE"};
  for (unsigned i = 0; i < 3; i++) {
    int x = 480 + (int)i * 167;
    sprite(&row, i, x, 35, 40, 53);
    label(&row, x + 53, 34, names[i], 18, MUTED);
    char amount[24];
    snprintf(amount, sizeof(amount), "%u.%02u", stock[i] / 1000,
             (stock[i] % 1000) / 10);
    label(&row, x + 54, 63, amount, 28, INK);
  }
  panel(&row, 24, 134, 330, 406);
  panel(&row, 376, 134, 624, 406);
  const char *titles[] = {"WORKBENCH", "EXPEDITION",        "CARGO",
                          "SAMPLES",   "RESEARCH",          "DISCOVERY",
                          "INCUBATE",  "INCUBATOR",         "HELLO, BEECHO",
                          "HABITAT",   "RESEARCH PLAN",     "DISCARD PACK",
                          "CRITTERS",  "RECORDED FINDINGS", "SAMPLE FINDING"};
  label(&row, 396, 152,
        lab->page == V1_FINDING || lab->page == V1_LIBRARY_FINDING
            ? pip_study(lab->study)->title
            : titles[lab->page],
        34, INK);
  unsigned count = selected_lab_options(lab);
  unsigned first = lab->focus >= 6 ? lab->focus - 5 : 0;
  for (unsigned i = first; i < count && i < first + 6; i++) {
    int yy = 159 + (int)(i - first) * 58;
    if (i == lab->focus) {
      rectangle(&row, 39, yy - 2, 299, 46, DEEP);
      focus(&row, 37, yy - 4, 303, 50);
    }
    unsigned sample, study;
    if (lab->page == V1_LIBRARY &&
        selected_lab_library_entry(lab, i, &sample, &study)) {
      label(&row, 53, yy, game->samples[sample].id, 18,
            i == lab->focus ? WARM : MUTED);
      label(&row, 53, yy + 23, pip_study(study)->title, 18,
            i == lab->focus ? WARM : INK);
    } else {
      label(&row, 53, yy + 7, selected_lab_option(lab, i), 18,
            i == lab->focus ? WARM : INK);
    }
  }
  char text[100];
  if (lab->page == V1_HOME) {
    sprite(&row, SPRITE_SAMPLE, 432, 221, 155, 155);
    label(&row, 613, 231, "A mystery to bring home", 24, INK);
    label(&row, 613, 275, "A life to discover", 24, WARM);
    label(&row, 414, 430, "Explore. Research. Meet your Beecho.", 24, INK);
  } else if (lab->page == V1_EXPEDITION || lab->page == V1_CARGO) {
    label(&row, 402, 205,
          game->expedition_active  ? "PROBE / GATHERING"
          : game->expedition_id[0] ? "SURVEY COMPLETE / RETURN WITH HAUL"
                                   : "CHOOSE YOUR EXPEDITION",
          24, WARM);
    unsigned cargo[] = {game->expedition_data, game->expedition_energy,
                        game->expedition_essence};
    for (unsigned i = 0; i < 3; i++) {
      int x = 416 + (int)i * 179;
      sprite(&row, i, x + 25, 265, 65, 86);
      snprintf(text, sizeof(text), "+ %u%%", (cargo[i] % GAME_PACK_SIZE) / 10);
      label(&row, x + 21, 367, text, 28, INK);
      snprintf(text, sizeof(text), "%u pack%s", cargo[i] / GAME_PACK_SIZE,
               cargo[i] / GAME_PACK_SIZE == 1 ? "" : "s");
      label(&row, x + 7, 410, text, 22, MUTED);
    }
    snprintf(text, sizeof(text), "%u / 60 s  |  Sample: %s",
             game->expedition_elapsed,
             game->sample_count >= GAME_MAX_SAMPLES ? "shelf full"
             : game->expedition_elapsed >= 60       ? "found"
                                                    : "scanning");
    label(&row, 411, 443, text, 18, MUTED);
    label(&row, 417, 391, "of next pack", 18, MUTED);
    label(&row, 596, 391, "of next pack", 18, MUTED);
    label(&row, 775, 391, "of next pack", 18, MUTED);
    unsigned total = cargo[0] + cargo[1] + cargo[2];
    snprintf(text, sizeof(text), "Cargo %.2f / 4 units", total / 1000.0);
    label(&row, 415, 503, text, 18, MUTED);
    outline(&row, 411, 470, 550, 20, 2, EDGE);
    rectangle(&row, 415, 474, (int)(542 * (total > 4000 ? 4000 : total) / 4000),
              12, BLUE);
  } else if (lab->page == V1_SAMPLES) {
    sprite(&row, SPRITE_SAMPLE, 602, 235, 155, 155);
    snprintf(text, sizeof(text), "%u samples retained", game->sample_count);
    label(&row, 452, 432, text, 28, INK);
  } else if (lab->page == V1_STUDIES && lab->focus == 5) {
    unsigned discovered = 0;
    for (unsigned i = 0; i < 5; i++)
      discovered += (game->samples[lab->sample].decoded_studies >> i) & 1u;
    sprite(&row, SPRITE_SAMPLE, 597, 240, 150, 150);
    snprintf(text, sizeof(text), "%u / 5 discoveries", discovered);
    label(&row, 483, 407, text, 28, WARM);
    label(&row, 414, 465,
          game->samples[lab->sample].incubated
              ? "This sample already has a Beecho."
          : discovered == 5 ? "Ready to choose a complete form."
                            : "Research every topic to prepare a form.",
          22, INK);
  } else if (lab->page == V1_DISCARD_REVIEW) {
    unsigned values[] = {game->expedition_data, game->expedition_energy,
                         game->expedition_essence};
    sprite(&row, lab->discard_resource, 600, 250, 96, 128);
    snprintf(text, sizeof(text), "Discard 1 %s pack?",
             names[lab->discard_resource]);
    label(&row, 420, 412, text, 24, WARM);
    label(&row, 420, 462,
          values[lab->discard_resource] >= 1000
              ? "This frees one cargo unit. It cannot be recovered."
              : "No complete pack of this resource yet.",
          18, INK);
  } else if (lab->page == V1_STUDIES || lab->page == V1_FINDING ||
             lab->page == V1_LIBRARY_FINDING || lab->page == V1_STUDY_REVIEW) {
    unsigned study = lab->page == V1_STUDIES ? lab->focus : lab->study;
    const PipStudy *entry = pip_study(study);
    int known =
        (game->samples[lab->sample].decoded_studies & (1u << study)) != 0;
    label(&row, 402, 203, game->samples[lab->sample].id, 18, MUTED);
    if (lab->page == V1_FINDING || lab->page == V1_LIBRARY_FINDING) {
      label(&row, 416, 252, "SAMPLE FINDING", 22, SAGE);
      if (study < 2)
        sprite(&row, study == 0 ? SPRITE_CROWN : SPRITE_EYE_RING, 437, 300, 115,
               115);
      wrapped_label(&row, study < 2 ? 585 : 421, study < 2 ? 300 : 311,
                    entry->finding, 24, INK, study < 2 ? 360 : 525);
      unsigned found = 0;
      for (unsigned i = 0; i < 5; i++)
        found += (game->samples[lab->sample].decoded_studies >> i) & 1u;
      snprintf(text, sizeof(text), "%u / 5 topics discovered", found);
      label(&row, 421, 476, text, 22, SAGE);
    } else {
      label(&row, 416, 248, entry->title, 28, WARM);
      label(&row, 416, 292,
            known ? "Recorded / inspect freely"
                  : "Unknown / ready to investigate",
            22, INK);
      unsigned costs[] = {entry->cost_data, entry->cost_energy,
                          entry->cost_essence};
      for (unsigned i = 0; i < 3; i++) {
        int x = 418 + (int)i * 181;
        sprite(&row, i, x, 343, 36, 48);
        snprintf(text, sizeof(text), "%.1f / %.2f",
                 known ? 0.0 : costs[i] / 1000.0, stock[i] / 1000.0);
        label(&row, x + 44, 355, text, 18,
              known || stock[i] >= costs[i] ? INK : WARM);
        if (!known && stock[i] < costs[i]) {
          snprintf(text, sizeof(text), "Need %.2f",
                   (costs[i] - stock[i]) / 1000.0);
          label(&row, x, 409, text, 18, WARM);
        }
      }
      label(&row, 419, 445, "Cost / available packs", 18, MUTED);
      label(&row, 419, 485,
            known ? "Confirm inspects this finding."
            : lab->page == V1_STUDY_REVIEW
                ? "Start research spends the listed resources."
                : "Confirm reviews the research plan.",
            18, INK);
    }
  } else if (lab->page == V1_LIBRARY) {
    label(&row, 420, 252,
          game->sample_count ? "Your recorded discoveries"
                             : "No discoveries yet",
          24, INK);
    label(&row, 420, 310, "Research topics to record findings here.", 22,
          MUTED);
    label(&row, 420, 366, "Select a finding to inspect it freely.", 22, INK);
  } else if (lab->page == V1_CREATE) {
    sprite(&row, SPRITE_SAMPLE, 448, 240, 128, 128);
    label(&row, 605, 235, "Genome decoded", 28, SAGE);
    label(&row, 605, 282, "Choose a supported form", 22, INK);
    label(&row, 410, 414, "1 sample + 0.5 of each resource", 24, INK);
    unsigned shortage = 0;
    for (unsigned i = 0; i < 3; i++)
      if (stock[i] < 500) {
        snprintf(text, sizeof(text), "Need %.2f %s", (500 - stock[i]) / 1000.0,
                 names[i]);
        label(&row, 410, 454 + (int)shortage * 23, text, 18, WARM);
        shortage++;
      }
    if (!shortage)
      label(&row, 410, 464, "Confirm starts one incubation.", 22, WARM);
  } else if (lab->page == V1_INCUBATION) {
    sprite(&row, SPRITE_SAMPLE, 602, 244, 128, 128);
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
      sprite(&row, SELECTED_SPRITE_COUNT + individual->expression.pale_markings,
             411, 235, 261, 289);
      label(&row, 686, 248,
            individual->expression.pale_markings ? "Pale markings"
                                                 : "Plain coat",
            28, WARM);
      label(&row, 686, 297, "Crown frill", 22, INK);
      label(&row, 686, 332, "Pale eye rings", 22, INK);
      label(&row, 686, 379, "Source sample", 18, MUTED);
      label(&row, 686, 404, individual->source_sample_id, 18, INK);
      if (lab->page == V1_HABITAT) {
        snprintf(text, sizeof(text), "Visits together: %u",
                 individual->care_visits);
        label(&row, 686, 465, text, 18, SAGE);
      } else if (lab->page == V1_REVEAL)
        label(&row, 686, 465, "Ready to meet you.", 18, SAGE);
    } else
      label(&row, 414, 439, "Research your first sample to begin.", 22, INK);
  }

  label(&row, 30, 557,
        lab->message[0]
            ? lab->message
            : "Up/down: focus | Right: inspect | Confirm: act | Back: return",
        18, lab->storage_error ? WARM : MUTED);
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
