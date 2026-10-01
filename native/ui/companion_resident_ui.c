#include "companion_resident_ui.h"
#include "../selected-lab/ui_theme.h"
#include "../selected-lab/core_art.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

struct CompanionResidentUi {
  lv_obj_t *root, *status, *coat, *identity, *portrait, *portrait_pending;
  lv_obj_t *visits_label, *visits, *property, *count, *feedback;
  lv_obj_t *entry, *preview_hint, *footer, *empty_title, *empty_description;
  lv_obj_t *selected_mode;
  NativeUiFrame outer_frame, subject_frame, mode_frame;
  CompanionResidentView view;
  char visits_text[16], count_text[48];
};
static lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, uint32_t color,
    int x, int y, int width, int height, const char *text) {
  lv_obj_t *object = lv_label_create(parent);
  if (object) {
    native_ui_text(object, font, color);
    lv_obj_set_pos(object, x, y);
    lv_obj_set_size(object, width, height);
    lv_label_set_long_mode(object, LV_LABEL_LONG_MODE_WRAP);
    lv_label_set_text_static(object, text);
  }
  return object;
}
static int valid_view(const CompanionResidentView *view, const lv_image_dsc_t *image) {
  if (!view || view->portrait > RESIDENT_EMPTY_HABITAT || view->failed > 1 ||
      view->current > 1 || view->online > 1 ||
      (view->count && view->selected_index >= view->count) ||
      (!view->count && (view->identity[0] || view->visits || view->property[0] ||
                        view->portrait != RESIDENT_EMPTY_HABITAT)) ||
      (view->count && (!view->identity[0] || view->portrait == RESIDENT_EMPTY_HABITAT)))
    return 0;
  const char *strings[] = {view->identity, view->coat, view->property, view->status, view->feedback};
  const size_t sizes[] = {sizeof(view->identity), sizeof(view->coat), sizeof(view->property),
                          sizeof(view->status), sizeof(view->feedback)};
  for (unsigned index = 0; index < sizeof(strings) / sizeof(strings[0]); ++index)
    if (!memchr(strings[index], 0, sizes[index])) return 0;
  if (view->portrait != RESIDENT_PORTRAIT_PENDING && (!image ||
      image->header.w != (view->count ? 261u : 136u) ||
      image->header.h != (view->count ? 289u : 144u))) return 0;
  return 1;
}
CompanionResidentUi *companion_resident_ui_create(lv_obj_t *parent,
                                                const CompanionResidentFonts *fonts) {
  if (!parent || !fonts || !fonts->title || !fonts->body || !fonts->small ||
      !fonts->quantity || !fonts->action) return NULL;
  CompanionResidentUi *ui = calloc(1, sizeof(*ui));
  if (!ui) return NULL;
  lv_display_set_default(lv_obj_get_display(parent));
  ui->root = lv_obj_create(parent);
  if (!ui->root) goto failure;
  native_ui_surface(ui->root, CORE_ART_GRAPHITE_RGB, CORE_ART_BLUE_RGB, 0);
  lv_obj_set_size(ui->root, 450, 600);
  const char *modes[] = {"Probe", "Cargo", "Companions"};
  const int mode_x[] = {24, 159, 294};
  for (unsigned index = 0; index < 3; ++index) {
    lv_obj_t *mode = label(ui->root, fonts->small, index == 2 ? CORE_ART_FOCUS_RGB : CORE_ART_SECONDARY_RGB,
                           mode_x[index], 27, index == 2 ? 132 : 104, 26, modes[index]);
    if (!mode) goto failure;
    if (index == 2) ui->selected_mode = mode;
  }
  if (!label(ui->root, fonts->title, CORE_ART_INK_RGB, 28, 68, 394, 32, "Companions")) goto failure;
  ui->status = label(ui->root, fonts->small, CORE_ART_SECONDARY_RGB, 28, 101, 394, 30, "");
  ui->coat = label(ui->root, fonts->action, CORE_ART_INK_RGB, 32, 136, 261, 26, "");
  ui->identity = label(ui->root, fonts->small, CORE_ART_SECONDARY_RGB, 303, 139, 115, 72, "");
  ui->portrait = lv_image_create(ui->root);
  if (!ui->portrait) goto failure;
  lv_image_set_antialias(ui->portrait, false);
  ui->portrait_pending = label(ui->root, fonts->body, CORE_ART_SECONDARY_RGB,
                               42, 275, 241, 54, "Portrait pending");
  ui->visits_label = label(ui->root, fonts->small, CORE_ART_SECONDARY_RGB, 303, 216, 115, 24, "Visits");
  ui->visits = label(ui->root, fonts->quantity, CORE_ART_INK_RGB, 303, 242, 115, 36, "0");
  ui->property = label(ui->root, fonts->small, CORE_ART_INK_RGB, 303, 286, 115, 126, "");
  ui->count = label(ui->root, fonts->small, CORE_ART_SECONDARY_RGB, 303, 418, 115, 30, "");
  ui->empty_title = label(ui->root, fonts->action, CORE_ART_INK_RGB, 32, 151, 380, 52, "No revealed residents");
  ui->empty_description = label(ui->root, fonts->body, CORE_ART_INK_RGB,
      203, 220, 205, 104, "Reveal a resident at the Lab to meet here.");
  ui->feedback = label(ui->root, fonts->body, CORE_ART_SECONDARY_RGB, 28, 459, 394, 58, "");
  ui->entry = label(ui->root, fonts->action, CORE_ART_INK_RGB, 28, 524, 394, 26, "Down / Confirm: enter");
  ui->preview_hint = label(ui->root, fonts->small, CORE_ART_SECONDARY_RGB,
                            28, 551, 394, 20, "Browsing never sends or spends");
  ui->footer = label(ui->root, fonts->small, CORE_ART_SECONDARY_RGB, 28, 568, 394, 20, "Left/Right: modes");
  if (!ui->status || !ui->coat || !ui->identity || !ui->portrait_pending || !ui->visits_label ||
      !ui->visits || !ui->property || !ui->count || !ui->empty_title || !ui->empty_description ||
      !ui->feedback || !ui->entry || !ui->preview_hint || !ui->footer) goto failure;
  if (!native_ui_frame_init(&ui->outer_frame, ui->root, 426, 576, CORE_ART_BLUE_RGB) ||
      !native_ui_frame_init(&ui->subject_frame, ui->root, 402, 324, CORE_ART_BLUE_RGB) ||
      !native_ui_frame_init(&ui->mode_frame, ui->root, 136, 30, CORE_ART_FOCUS_RGB)) goto failure;
  lv_obj_set_pos(ui->outer_frame.object, 12, 12);
  lv_obj_set_pos(ui->subject_frame.object, 24, 132);
  lv_obj_set_pos(ui->mode_frame.object, 292, 25);
  lv_obj_set_hidden(ui->root, true);
  return ui;
failure:
  companion_resident_ui_destroy(ui);
  return NULL;
}
int companion_resident_ui_update(CompanionResidentUi *ui,
    const CompanionResidentView *view, const lv_image_dsc_t *image) {
  if (!ui || !valid_view(view, image)) return 0;
  ui->view = *view;
  view = &ui->view;
  lv_obj_set_hidden(ui->root, false);
  lv_label_set_text_static(ui->status, view->status);
  lv_label_set_text_static(ui->coat, view->coat);
  lv_label_set_text_static(ui->identity, view->identity);
  lv_label_set_text_static(ui->property, view->property);
  lv_label_set_text_static(ui->feedback, view->feedback);
  snprintf(ui->visits_text, sizeof(ui->visits_text), "%" PRIu32, view->visits);
  snprintf(ui->count_text, sizeof(ui->count_text), "%" PRIu32 " / %" PRIu32, view->selected_index + 1, view->count);
  lv_label_set_text_static(ui->visits, ui->visits_text);
  lv_label_set_text_static(ui->count, ui->count_text);
  lv_obj_t *resident_labels[] = {ui->coat, ui->identity, ui->visits_label, ui->visits, ui->property, ui->count};
  for (unsigned index = 0; index < sizeof(resident_labels) / sizeof(resident_labels[0]); ++index)
    lv_obj_set_hidden(resident_labels[index], !view->count);
  lv_obj_set_hidden(ui->empty_title, view->count != 0);
  lv_obj_set_hidden(ui->empty_description, view->count != 0);
  lv_obj_set_hidden(ui->portrait_pending, !view->count || view->portrait != RESIDENT_PORTRAIT_PENDING);
  lv_obj_set_hidden(ui->portrait, view->portrait == RESIDENT_PORTRAIT_PENDING);
  if (image && view->portrait != RESIDENT_PORTRAIT_PENDING) {
    lv_image_set_src(ui->portrait, image);
    lv_obj_set_pos(ui->portrait, view->count ? 32 : 43, view->count ? 160 : 208);
  }
  lv_obj_set_hidden(ui->entry, view->failed != 0);
  lv_obj_set_hidden(ui->preview_hint, view->failed != 0);
  lv_obj_set_hidden(ui->mode_frame.object, view->failed != 0);
  lv_obj_set_style_text_color(ui->selected_mode,
      lv_color_hex(view->failed ? CORE_ART_INK_RGB : CORE_ART_FOCUS_RGB), 0);
  lv_label_set_text_static(ui->footer, view->failed ? "Storage recovery required" : "Left/Right: modes");
  return 1;
}
void companion_resident_ui_hide(CompanionResidentUi *ui) {
  if (ui) lv_obj_set_hidden(ui->root, true);
}
void companion_resident_ui_destroy(CompanionResidentUi *ui) {
  if (!ui) return;
  if (ui->root) lv_obj_delete(ui->root);
  free(ui);
}
