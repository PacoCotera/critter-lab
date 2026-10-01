#include "lab_research_ui.h"
#include "../selected-lab/ui_theme.h"
#include "../selected-lab/core_art.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LabResearchUi {
  lv_obj_t *root, *stock[3], *units[3], *rows[6], *row_details[6];
  lv_obj_t *title, *sample, *heading, *body, *art, *portraits[2], *captions[2];
  lv_obj_t *finding, *summary[3], *topics[5], *topic_status[5], *partial;
  lv_obj_t *costs[3], *cost_art[3], *cost_title, *counts[3];
  lv_obj_t *alternatives[2], *footer, *message;
  NativeUiFrame header, rail, workpiece, focus;
  LabHomeFonts fonts;
  LabResearchView view;
  const lv_image_dsc_t *images[18];
  char stock_text[3][24], cost_text[3][48], count_text[3][64];
};
static lv_obj_t *surface(lv_obj_t *parent, int x, int y, int w, int h, uint32_t color) {
  lv_obj_t *object = lv_obj_create(parent);
  if (!object) return NULL;
  native_ui_surface(object, color, 0, 0);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_size(object, w, h);
  return object;
}
static lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, int x, int y,
    int w, int h, uint32_t color, const char *value) {
  lv_obj_t *object = lv_label_create(parent);
  if (!object) return NULL;
  native_ui_text(object, font, color);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_size(object, w, h);
  lv_label_set_long_mode(object, LV_LABEL_LONG_MODE_WRAP);
  lv_label_set_text_static(object, value);
  return object;
}
static lv_obj_t *image(lv_obj_t *parent, const lv_image_dsc_t *source, int x, int y) {
  lv_obj_t *object = lv_image_create(parent);
  if (!object) return NULL;
  lv_obj_set_clickable(object, false);
  lv_image_set_antialias(object, false);
  lv_image_set_src(object, source);
  lv_obj_set_pos(object, x, y);
  return object;
}
LabResearchUi *lab_research_ui_create(lv_obj_t *parent, const LabHomeFonts *fonts,
    const lv_image_dsc_t *const images[18]) {
  if (!parent || !fonts || !images) return NULL;
  LabResearchUi *ui = calloc(1, sizeof(*ui));
  if (!ui) return NULL;
  ui->fonts = *fonts;
  for (unsigned index = 0; index < 18; ++index) {
    if (!images[index]) goto failure;
    ui->images[index] = images[index];
  }
  ui->root = surface(parent, 0, 0, 1024, 600, CORE_ART_GRAPHITE_RGB);
  if (!ui->root ||
      !native_ui_frame_init(&ui->header, ui->root, 976, 100, CORE_ART_BLUE_RGB) ||
      !native_ui_frame_init(&ui->rail, ui->root, 330, 416, CORE_ART_BLUE_RGB) ||
      !native_ui_frame_init(&ui->workpiece, ui->root, 624, 416, CORE_ART_BLUE_RGB) ||
      !native_ui_frame_init(&ui->focus, ui->root, 302, 56, CORE_ART_FOCUS_RGB)) goto failure;
  lv_obj_set_pos(ui->header.object, 24, 24);
  lv_obj_set_pos(ui->rail.object, 24, 140);
  lv_obj_set_pos(ui->workpiece.object, 376, 140);
  native_ui_surface(ui->focus.object, 0x292922, CORE_ART_FOCUS_RGB, 0);
  native_ui_frame_size(&ui->focus, 302, 56);
  lv_obj_set_style_shadow_color(ui->focus.object, lv_color_hex(0xc58f32), 0);
  lv_obj_set_style_shadow_width(ui->focus.object, 10, 0);
  lv_obj_set_style_shadow_opa(ui->focus.object, 40, 0);
  if (!surface(ui->root, 37, 37, 950, 75, CORE_ART_FIELD_RGB)) goto failure;
  if (!label(ui->root, fonts->title, 46, 35, 330, 42, CORE_ART_INK_RGB, "BEECHO LAB") ||
      !label(ui->root, fonts->small, 47, 83, 330, 24, CORE_ART_SECONDARY_RGB, "LAB STOCK")) goto failure;
  const char *names[] = {"DATA", "ENERGY", "ESSENCE"};
  for (unsigned index = 0; index < 3; ++index) {
    int x = 402 + (int)index * 196;
    if (!image(ui->root, images[4+index], x+(50-(int)images[4+index]->header.w)/2,
        41+(58-(int)images[4+index]->header.h)/2) ||
        !label(ui->root, fonts->small, x+62, 39, 126, 24, CORE_ART_SECONDARY_RGB, names[index])) goto failure;
    ui->stock[index] = label(ui->root, fonts->status, x+62, 61, 126, 32, CORE_ART_INK_RGB, "");
    ui->units[index] = label(ui->root, fonts->small, x+62, 89, 126, 22, CORE_ART_SECONDARY_RGB, "");
    ui->cost_art[index] = image(ui->root, images[4+index], 600, 332+(int)index*56);
    ui->costs[index] = label(ui->root, fonts->body, 665, 343+(int)index*56, 284, 32, CORE_ART_INK_RGB, "");
    ui->counts[index] = label(ui->root, fonts->body, 588, 315+(int)index*42, 380, 34, CORE_ART_INK_RGB, "");
    if (!ui->stock[index] || !ui->units[index] || !ui->cost_art[index] || !ui->costs[index] || !ui->counts[index]) goto failure;
  }
  for (unsigned row = 0; row < 6; ++row) {
    ui->rows[row] = label(ui->root, fonts->small, 50, 169+(int)row*62, 286, 48, CORE_ART_INK_RGB, "");
    ui->row_details[row] = label(ui->root, fonts->small, 50, 194+(int)row*62, 286, 28, CORE_ART_SECONDARY_RGB, "");
    if (!ui->rows[row] || !ui->row_details[row]) goto failure;
  }
  ui->title = label(ui->root, fonts->status, 398, 153, 580, 38, CORE_ART_INK_RGB, "");
  ui->sample = label(ui->root, fonts->small, 402, 197, 574, 25, CORE_ART_SECONDARY_RGB, "");
  ui->heading = label(ui->root, fonts->status, 416, 245, 560, 70, CORE_ART_INK_RGB, "");
  ui->body = label(ui->root, fonts->small, 416, 282, 560, 54, CORE_ART_SECONDARY_RGB, "");
  ui->art = image(ui->root, images[1], 424, 315);
  ui->finding = label(ui->root, fonts->body, 610, 247, 356, 164, CORE_ART_INK_RGB, "");
  ui->cost_title = label(ui->root, fonts->small, 665, 307, 290, 28, CORE_ART_SECONDARY_RGB, "Cost / Lab stock");
  ui->partial = label(ui->root, fonts->small, 416, 400, 556, 28, CORE_ART_SECONDARY_RGB,
      "Pale variation known / appearance unresolved");
  for (unsigned index = 0; index < 2; ++index) {
    ui->portraits[index] = image(ui->root, images[11+index], 405+(int)index*294, 247);
    ui->captions[index] = label(ui->root, fonts->small, 405+(int)index*294, 219, 275, 27, CORE_ART_INK_RGB, "");
    ui->alternatives[index] = label(ui->root, fonts->small, 614+(int)index*178, 349, 169, 90,
        CORE_ART_SECONDARY_RGB, index ? "Pale markings\nAppearance expressed" : "Plain coat\nPale variation carried");
    if (!ui->portraits[index] || !ui->captions[index] || !ui->alternatives[index]) goto failure;
  }
  for (unsigned index = 0; index < 5; ++index) {
    ui->topics[index] = label(ui->root, fonts->small, 430, 276+(int)index*31, 250, 26, CORE_ART_INK_RGB, "");
    ui->topic_status[index] = label(ui->root, fonts->small, 706, 276+(int)index*31, 244, 26, CORE_ART_SECONDARY_RGB, "");
    if (!ui->topics[index] || !ui->topic_status[index]) goto failure;
  }
  for (unsigned index = 0; index < 3; ++index) {
    ui->summary[index] = label(ui->root, fonts->small, 416, 458+(int)index*26, 552, 48,
        index == 0 ? CORE_ART_SAVED_RGB : index == 1 ? CORE_ART_SECONDARY_RGB : CORE_ART_INK_RGB, "");
    if (!ui->summary[index]) goto failure;
  }
  ui->footer = label(ui->root, fonts->small, 30, 569, 964, 27, CORE_ART_SECONDARY_RGB, "");
  ui->message = label(ui->root, fonts->small, 398, 560, 588, 38, CORE_ART_INK_RGB, "");
  if (!ui->title || !ui->sample || !ui->heading || !ui->body || !ui->art || !ui->finding ||
      !ui->cost_title || !ui->partial || !ui->footer || !ui->message) goto failure;
  return ui;
failure:
  lab_research_ui_destroy(ui);
  return NULL;
}
void lab_research_ui_destroy(LabResearchUi *ui) {
  if (!ui) return;
  if (ui->root) lv_obj_delete(ui->root);
  free(ui);
}
void lab_research_ui_hide(LabResearchUi *ui) {
  if (ui) lv_obj_set_hidden(ui->root, true);
}
static int terminated(const char *text, size_t size) { return memchr(text, 0, size) != NULL; }
static int valid(const LabResearchView *view) {
  if ((unsigned)view->page > LAB_RESEARCH_LIBRARY_FINDING ||
      (unsigned)view->detail > LAB_RESEARCH_RECORDS || (unsigned)view->art > LAB_RESEARCH_ART_PAIR ||
      !view->option_count || view->option_count > LAB_RESEARCH_OPTIONS || view->focus >= view->option_count ||
      view->topic_count > LAB_RESEARCH_TOPICS || view->known_method > 1 || view->useful > 1 ||
      view->legacy > 1 || view->complete > 1 || view->partial_p > 1 || view->used > 1 ||
      view->show_alternatives > 1 || view->storage_error > 1 || view->suspended > 1) return 0;
  if ((view->page == LAB_RESEARCH_SAMPLES && view->detail != LAB_RESEARCH_COLLECTION && view->detail != LAB_RESEARCH_KNOWLEDGE) ||
      (view->page == LAB_RESEARCH_STUDIES && view->detail != LAB_RESEARCH_PLAN && view->detail != LAB_RESEARCH_PREPARATION) ||
      (view->page == LAB_RESEARCH_REVIEW && view->detail != LAB_RESEARCH_PLAN) ||
      ((view->page == LAB_RESEARCH_FINDING || view->page == LAB_RESEARCH_LIBRARY_FINDING) && view->detail != LAB_RESEARCH_DISCOVERY) ||
      (view->page == LAB_RESEARCH_LIBRARY && view->detail != LAB_RESEARCH_RECORDS)) return 0;
  if (view->show_alternatives && (view->detail != LAB_RESEARCH_DISCOVERY ||
      !view->known_method || view->legacy || view->art != LAB_RESEARCH_ART_INHERITANCE)) return 0;
  if ((view->art == LAB_RESEARCH_ART_CROWN || view->art == LAB_RESEARCH_ART_EYE_RING) &&
      (view->detail != LAB_RESEARCH_DISCOVERY || !view->legacy || !view->known_method)) return 0;
#define CHECK_STRING(field) if (!terminated(view->field, sizeof(view->field))) return 0
  CHECK_STRING(title); CHECK_STRING(sample_id); CHECK_STRING(heading); CHECK_STRING(body);
  CHECK_STRING(finding); CHECK_STRING(known); CHECK_STRING(missing); CHECK_STRING(next);
  CHECK_STRING(message); CHECK_STRING(footer);
#undef CHECK_STRING
  for (unsigned index = 0; index < view->option_count; ++index)
    if (!terminated(view->options[index], sizeof(view->options[index])) ||
        !terminated(view->option_details[index], sizeof(view->option_details[index]))) return 0;
  for (unsigned index = 0; index < view->topic_count; ++index)
    if (view->topic_known[index] > 1 || !terminated(view->topics[index], sizeof(view->topics[index]))) return 0;
  for (unsigned index = 0; index < 2; ++index) {
    if ((unsigned)view->portraits[index] > LAB_RESEARCH_PORTRAIT_MARKED ||
        !terminated(view->portrait_caption[index], sizeof(view->portrait_caption[index]))) return 0;
    if (view->art != LAB_RESEARCH_ART_PAIR && view->portraits[index] != LAB_RESEARCH_PORTRAIT_NONE) return 0;
  }
  if (view->art == LAB_RESEARCH_ART_PAIR &&
      (view->detail != LAB_RESEARCH_DISCOVERY || !view->complete || !view->known_method || view->partial_p ||
       !view->portraits[0] || !view->portraits[1] ||
       (view->page != LAB_RESEARCH_FINDING && view->page != LAB_RESEARCH_LIBRARY_FINDING))) return 0;
  return 1;
}
int lab_research_ui_update(LabResearchUi *ui, const LabResearchView *view) {
  if (!ui || !view || !valid(view)) return 0;
  ui->view = *view;
  view = &ui->view;
  lv_obj_set_hidden(ui->root, false);
  int pair = view->art == LAB_RESEARCH_ART_PAIR;
  int knowledge = view->detail == LAB_RESEARCH_KNOWLEDGE;
  int plan = view->detail == LAB_RESEARCH_PLAN;
  int collection = view->detail == LAB_RESEARCH_COLLECTION;
  int discovery = view->detail == LAB_RESEARCH_DISCOVERY;
  unsigned visible_rows = view->page == LAB_RESEARCH_LIBRARY ? 3 : 6;
  unsigned row_height = view->page == LAB_RESEARCH_LIBRARY ? 112 : 62;
  unsigned first = view->focus >= visible_rows ? view->focus - visible_rows + 1 : 0;
  native_ui_frame_size(&ui->focus, 302, (int)row_height-6);
  for (unsigned row = 0; row < 6; ++row) {
    unsigned option = first + row;
    int visible = row < visible_rows && option < view->option_count;
    lv_obj_set_hidden(ui->rows[row], !visible);
    lv_obj_set_hidden(ui->row_details[row], !visible || !view->option_details[option < view->option_count ? option : 0][0]);
    if (visible) {
      lv_label_set_text_static(ui->rows[row], view->options[option]);
      lv_label_set_text_static(ui->row_details[row], view->option_details[option]);
      lv_obj_set_style_text_color(ui->rows[row], lv_color_hex(option == view->focus ? CORE_ART_FOCUS_RGB : CORE_ART_INK_RGB), 0);
      lv_obj_set_style_text_color(ui->row_details[row], lv_color_hex(option == view->focus ? CORE_ART_FOCUS_RGB : CORE_ART_SECONDARY_RGB), 0);
      lv_obj_set_pos(ui->rows[row], 50, 169+(int)(row*row_height));
      lv_obj_set_height(ui->rows[row], 48);
      lv_obj_set_pos(ui->row_details[row], 50, 218+(int)(row*row_height));
      lv_obj_set_height(ui->row_details[row], 48);
    }
  }
  lv_obj_set_pos(ui->focus.object, 38, 157+(int)((view->focus-first)*row_height));
  lv_label_set_text_static(ui->title, view->title);
  lv_label_set_text_static(ui->sample, view->sample_id);
  lv_label_set_text_static(ui->heading, view->heading);
  lv_label_set_text_static(ui->body, view->body);
  lv_obj_set_hidden(ui->heading, discovery || knowledge);
  lv_obj_set_hidden(ui->body, pair || knowledge || collection);
  lv_obj_set_pos(ui->heading, collection || view->detail == LAB_RESEARCH_RECORDS ? 588 : 416, 245);
  lv_obj_set_width(ui->heading, collection || view->detail == LAB_RESEARCH_RECORDS ? 380 : 552);
  int right_body = view->detail == LAB_RESEARCH_PREPARATION || view->detail == LAB_RESEARCH_RECORDS;
  lv_obj_set_pos(ui->body, right_body ? 588 : 416,
      view->detail == LAB_RESEARCH_PREPARATION ? 286 : view->detail == LAB_RESEARCH_RECORDS ? 330 : discovery ? 226 : 284);
  lv_obj_set_width(ui->body, right_body ? 380 : 552);
  lv_obj_set_height(ui->body, right_body ? 106 : 54);
  const int art_slots[] = {-1, 10, 1, 13, 14, 15, 16, 17, -1};
  int slot = art_slots[view->art];
  lv_obj_set_hidden(ui->art, slot < 0);
  if (slot >= 0) {
    lv_image_set_src(ui->art, ui->images[slot]);
    int x = knowledge ? 422 : discovery ? 416+(184-(int)ui->images[slot]->header.w)/2 : 424;
    int y = knowledge ? 220 : discovery ? 249+(195-(int)ui->images[slot]->header.h)/2 : plan ? 319 : 273;
    lv_obj_set_pos(ui->art, x, y);
  }
  lv_label_set_text_static(ui->finding, view->finding);
  lv_obj_set_hidden(ui->finding, !discovery);
  lv_obj_set_pos(ui->finding, pair ? 46 : 610, pair ? 243 : 247);
  lv_obj_set_size(ui->finding, pair ? 288 : 356, pair ? 148 : view->show_alternatives ? 94 : 178);
  lv_obj_set_style_text_font(ui->finding, pair ? ui->fonts.small : ui->fonts.body, 0);
  const char *summaries[] = {view->known, view->missing, view->next};
  for (unsigned index = 0; index < 3; ++index) {
    lv_label_set_text_static(ui->summary[index], summaries[index]);
    lv_obj_set_hidden(ui->summary[index], !summaries[index][0] ||
        (plan && index < 2) || ((collection || view->detail == LAB_RESEARCH_RECORDS) && index < 2));
    lv_obj_set_pos(ui->summary[index], pair ? 46 : 416, pair ? 397+(int)index*44 : 448+(int)index*32);
    lv_obj_set_size(ui->summary[index], pair ? 288 : 552, pair ? 44 : 32);
  }
  for (unsigned index = 0; index < 2; ++index) {
    lv_obj_set_hidden(ui->portraits[index], !pair);
    lv_obj_set_hidden(ui->captions[index], !pair);
    lv_obj_set_hidden(ui->alternatives[index], !view->show_alternatives);
    if (pair) {
      lv_image_set_src(ui->portraits[index], ui->images[view->portraits[index] == LAB_RESEARCH_PORTRAIT_MARKED ? 12 : 11]);
      lv_label_set_text_static(ui->captions[index], view->portrait_caption[index]);
    }
  }
  for (unsigned index = 0; index < 5; ++index) {
    int visible = knowledge && index < view->topic_count;
    lv_obj_set_hidden(ui->topics[index], !visible);
    lv_obj_set_hidden(ui->topic_status[index], !visible);
    if (visible) {
      lv_label_set_text_static(ui->topics[index], view->topics[index]);
      lv_label_set_text_static(ui->topic_status[index], view->topic_known[index] ? "Known" : "Still to learn");
      lv_obj_set_style_text_color(ui->topic_status[index], lv_color_hex(view->topic_known[index] ? CORE_ART_SAVED_RGB : CORE_ART_SECONDARY_RGB), 0);
    }
  }
  lv_obj_set_hidden(ui->partial, !knowledge || !view->partial_p);
  lv_obj_set_pos(ui->partial, 416, 276+(int)view->topic_count*31+4);
  lv_obj_set_hidden(ui->cost_title, !plan);
  const unsigned counts[] = {view->awaiting, view->ready, view->used_records};
  const char *count_names[] = {"awaiting research", "ready to prepare", "used / records retained"};
  for (unsigned index = 0; index < 3; ++index) {
    snprintf(ui->stock_text[index], sizeof(ui->stock_text[index]), "%u", view->stock[index]);
    lv_label_set_text_static(ui->stock[index], ui->stock_text[index]);
    lv_label_set_text_static(ui->units[index], view->stock[index] == 1 ? "unit" : "units");
    lv_obj_set_hidden(ui->cost_art[index], !plan);
    lv_obj_set_hidden(ui->costs[index], !plan);
    snprintf(ui->cost_text[index], sizeof(ui->cost_text[index]), "%u / %u", view->costs[index], view->stock[index]);
    lv_label_set_text_static(ui->costs[index], ui->cost_text[index]);
    lv_obj_set_hidden(ui->counts[index], !collection);
    snprintf(ui->count_text[index], sizeof(ui->count_text[index]), "%u %s", counts[index], count_names[index]);
    lv_label_set_text_static(ui->counts[index], ui->count_text[index]);
  }
  lv_label_set_text_static(ui->message, view->message);
  lv_obj_set_hidden(ui->message, !view->message[0]);
  lv_obj_set_style_text_color(ui->message, lv_color_hex(view->storage_error ? CORE_ART_FOCUS_RGB : CORE_ART_INK_RGB), 0);
  lv_obj_set_width(ui->footer, view->message[0] ? 330 : 964);
  lv_label_set_text_static(ui->footer, view->message[0] ? "Back: return" : view->footer);
  return 1;
}
