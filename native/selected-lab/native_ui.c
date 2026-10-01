#include "native_ui.h"
#include "ui_assets.h"
#include "ui_theme.h"
#include "probe_ui.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum { CARGO_WIDTH = 450, CARGO_HEIGHT = 600, DRAW_ROWS = 60, FADE_MS = 120 };
struct NativeUiContext {
  lv_display_t *display;
  NativeProbeUi *probe;
  lv_obj_t *cargo_root;
  lv_group_t *actions;
  lv_obj_t *screen, *title, *context, *quantity[3], *capsule, *detail;
  lv_obj_t *capacity, *feedback, *buttons[2], *button_text[2], *capsule_image;
  NativeUiFrame outer_frame, subject_frame, focus_frame, halo_frame;
  lv_font_t title_font, body_font, small_font, quantity_font, action_font;
  NativeUiImage images[4];
  uint8_t *rgb, *draw;
  char quantity_text[3][16], receipt_text[192];
  CompanionCargoView previous;
  int has_previous, failed, pending;
  unsigned elapsed;
};
static unsigned context_count;

static void flush_rgb(lv_display_t *display, const lv_area_t *area, uint8_t *pixels) {
  NativeUiContext *context = lv_display_get_user_data(display);
  int width = area->x2 - area->x1 + 1;
  int height = area->y2 - area->y1 + 1;
  uint32_t stride = lv_draw_buf_width_to_stride((unsigned)width, LV_COLOR_FORMAT_RGB888);
  if (area->x1 < 0 || area->y1 < 0 || area->x2 >= CARGO_WIDTH ||
      area->y2 >= CARGO_HEIGHT || width <= 0 || height <= 0 ||
      (uint64_t)stride * (unsigned)height > CARGO_WIDTH * DRAW_ROWS * 3u) {
    context->failed = 1;
  } else {
    for (int row = 0; row < height; ++row) {
      uint8_t *target = context->rgb + ((area->y1 + row) * CARGO_WIDTH + area->x1) * 3;
      const uint8_t *source = pixels + (unsigned)row * stride;
      /* LVGL calls this RGB888 but its native bytes are B,G,R. */
      for (int column = 0; column < width; ++column) {
        target[column * 3] = source[column * 3 + 2];
        target[column * 3 + 1] = source[column * 3 + 1];
        target[column * 3 + 2] = source[column * 3];
      }
    }
  }
  /* This releases LVGL's buffer; it never acknowledges a visible Kit frame. */
  lv_display_flush_ready(display);
}
static lv_obj_t *surface(lv_obj_t *parent, int width, int height) {
  lv_obj_t *object = lv_obj_create(parent);
  if (object) {
    native_ui_surface(object, CORE_ART_FIELD_RGB, CORE_ART_BLUE_RGB, 0);
    lv_obj_set_size(object, width, height);
  }
  return object;
}
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
static int compose(NativeUiContext *context) {
  lv_display_set_default(context->display);
  context->screen = lv_display_get_screen_active(context->display);
  native_ui_surface(context->screen, CORE_ART_GRAPHITE_RGB, CORE_ART_BLUE_RGB, 0);
  context->cargo_root = surface(context->screen, 450, 600);
  if (!context->cargo_root) return 0;
  native_ui_surface(context->cargo_root, CORE_ART_GRAPHITE_RGB, CORE_ART_BLUE_RGB, 0);
  /* Applying the surface style removes LVGL's local size styles. Restore the
   * full display bounds before composing children, or Cargo clips to130px. */
  lv_obj_set_size(context->cargo_root, CARGO_WIDTH, CARGO_HEIGHT);
  lv_obj_t *rim = surface(context->cargo_root, 426, 576);
  if (!rim) return 0;
  lv_obj_set_pos(rim, 12, 12);
  lv_obj_t *content = surface(context->cargo_root, 402, 560);
  if (!content) return 0;
  lv_obj_set_pos(content, 24, 24);
  static const int32_t columns[] = {402, LV_GRID_TEMPLATE_LAST};
  static const int32_t rows[] = {32, 56, 318, 56, 78, 20, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(content, columns, rows);
  lv_obj_t *bands[6];
  for (unsigned index = 0; index < 6; ++index) {
    bands[index] = surface(content, 402, rows[index]);
    if (!bands[index]) return 0;
    lv_obj_set_grid_cell(bands[index], LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_STRETCH, index, 1);
  }
  lv_obj_set_flex_flow(bands[0], LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(bands[0], LV_FLEX_ALIGN_SPACE_BETWEEN,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  const char *modes[] = {"Probe", "Cargo", "Companions"};
  for (unsigned mode = 0; mode < 3; ++mode) {
    lv_obj_t *name = label(bands[0], &context->small_font,
                           mode == 1 ? CORE_ART_INK_RGB : CORE_ART_SECONDARY_RGB,
                           0, 0, mode == 2 ? 132 : 104, 26, modes[mode]);
    if (!name) return 0;
    if (mode == 1) {
      lv_obj_set_style_border_side(name, LV_BORDER_SIDE_BOTTOM, 0);
      lv_obj_set_style_border_width(name, 2, 0);
      lv_obj_set_style_border_color(name, lv_color_hex(CORE_ART_BLUE_RGB), 0);
    }
  }
  context->title = label(bands[1], &context->title_font, CORE_ART_INK_RGB,
                         4, 8, 394, 32, "Cargo");
  context->context = label(bands[1], &context->small_font, CORE_ART_SECONDARY_RGB,
                           5, 38, 390, 20, "");
  static const int32_t material_columns[] = {132, 134, 132, LV_GRID_TEMPLATE_LAST};
  static const int32_t subject_rows[] = {228, 1, 85, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_style_pad_all(bands[2], 2, 0);
  lv_obj_set_grid_dsc_array(bands[2], material_columns, subject_rows);
  const char *names[] = {"Data", "Energy", "Essence"};
  const int image_x[] = {29, 30, 27}, image_y[] = {64, 67, 67};
  for (unsigned index = 0; index < 3; ++index) {
    int cell_width = material_columns[index];
    lv_obj_t *cell = surface(bands[2], cell_width, 228);
    if (!cell) return 0;
    lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, index, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_t *image = lv_image_create(cell);
    if (!image) return 0;
    lv_image_set_src(image, &context->images[index].image);
    lv_obj_set_pos(image, image_x[index], image_y[index]);
    lv_image_set_antialias(image, false);
    context->quantity[index] = label(cell, &context->quantity_font, CORE_ART_INK_RGB,
                                    0, 160, cell_width, 36, "0");
    lv_obj_t *name = label(cell, &context->small_font, CORE_ART_SECONDARY_RGB,
                           0, 196, cell_width, 24, names[index]);
    if (!context->quantity[index] || !name) return 0;
    lv_obj_set_style_text_align(context->quantity[index], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
  }
  lv_obj_t *divider = surface(bands[2], 398, 1);
  lv_obj_t *sample = surface(bands[2], 398, 85);
  if (!divider || !sample) return 0;
  lv_obj_set_grid_cell(divider, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_STRETCH, 1, 1);
  lv_obj_set_style_bg_color(divider, lv_color_hex(CORE_ART_BLUE_RGB), 0);
  lv_obj_set_grid_cell(sample, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_STRETCH, 2, 1);
  context->capsule_image = lv_image_create(sample);
  if (!context->capsule_image) return 0;
  lv_image_set_src(context->capsule_image, &context->images[3].image);
  lv_obj_set_pos(context->capsule_image, 17, 15);
  lv_image_set_antialias(context->capsule_image, false);
  context->capsule = label(sample, &context->body_font, CORE_ART_INK_RGB,
                           86, 10, 298, 24, "");
  context->detail = label(sample, &context->small_font, CORE_ART_SECONDARY_RGB,
                          86, 34, 298, 51, "");
  context->capacity = label(bands[3], &context->body_font, CORE_ART_SECONDARY_RGB,
                            4, 4, 394, 25, "");
  context->feedback = label(bands[3], &context->small_font, CORE_ART_SECONDARY_RGB,
                            4, 29, 394, 24, "");
  lv_obj_set_flex_flow(bands[4], LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(bands[4], 4, 0);
  for (unsigned action = 0; action < 2; ++action) {
    context->buttons[action] = lv_button_create(bands[4]);
    if (!context->buttons[action]) return 0;
    native_ui_action(context->buttons[action]);
    lv_obj_set_size(context->buttons[action], 402, action ? 30 : 38);
    lv_group_add_obj(context->actions, context->buttons[action]);
    context->button_text[action] = label(context->buttons[action], &context->action_font,
                                         CORE_ART_INK_RGB, 16, action ? 2 : 6, 370, 26, "");
    if (!context->button_text[action]) return 0;
  }
  if (!label(bands[5], &context->small_font, CORE_ART_SECONDARY_RGB,
              4, 2, 394, 20, "Up/Down: choose / Back: modes")) return 0;
  /* Overlay retained frame widgets after content so grid cells cannot erase
   * their stepped edges. Their points are owned by this context. */
  if (!native_ui_frame_init(&context->outer_frame, context->cargo_root, 426, 576, CORE_ART_BLUE_RGB) ||
      !native_ui_frame_init(&context->subject_frame, context->cargo_root, 402, 318, CORE_ART_BLUE_RGB) ||
      !native_ui_frame_init(&context->focus_frame, context->cargo_root, 402, 38, CORE_ART_FOCUS_RGB) ||
      !native_ui_frame_init(&context->halo_frame, context->cargo_root, 408, 44, CORE_ART_FOCUS_RGB)) return 0;
  lv_obj_set_pos(context->outer_frame.object, 12, 12);
  lv_obj_set_pos(context->subject_frame.object, 24, 112);
  lv_obj_set_hidden(context->focus_frame.object, true);
  /* A separate transparent outer edge leaves the solid focused border visible. */
  lv_obj_set_hidden(context->halo_frame.object, true);
  return context->title && context->context && context->capsule && context->detail &&
         context->capacity && context->feedback;
}
NativeUiContext *native_ui_create(void) {
  NativeUiContext *context = calloc(1, sizeof(*context));
  if (!context) return NULL;
  if (!context_count) lv_init();
  ++context_count;
  context->rgb = calloc(CARGO_WIDTH * CARGO_HEIGHT, 3);
  context->draw = malloc(CARGO_WIDTH * DRAW_ROWS * 3);
  native_ui_font_init(&context->title_font, &lab_heading_fonts[0]);
  native_ui_font_init(&context->body_font, &lab_fonts[0]);
  native_ui_font_init(&context->small_font, &lab_fonts[15]);
  native_ui_font_init(&context->quantity_font, &lab_fonts[7]);
  native_ui_font_init(&context->action_font, &lab_heading_fonts[4]);
  for (unsigned index = 0; index < 4; ++index) {
    CoreArtId id = index == 3 ? CORE_ART_SAMPLE_NEUTRAL : (CoreArtId)(CORE_ART_DATA_PRIMARY + index);
    if (!native_ui_image_init(&context->images[index], id)) goto failure;
  }
  if (!context->rgb || !context->draw) goto failure;
  context->display = lv_display_create(CARGO_WIDTH, CARGO_HEIGHT);
  context->actions = lv_group_create();
  if (!context->display || !context->actions) goto failure;
  lv_display_set_color_format(context->display, LV_COLOR_FORMAT_RGB888);
  lv_display_set_user_data(context->display, context);
  lv_display_set_buffers(context->display, context->draw, NULL,
                          CARGO_WIDTH * DRAW_ROWS * 3, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(context->display, flush_rgb);
  if (!compose(context)) goto failure;
  context->probe = native_probe_ui_create(context->screen, context->actions,
      &context->body_font, &context->title_font, &context->small_font, &context->action_font, &context->images[3]);
  if (!context->probe) goto failure;
  return context;
failure:
  native_ui_destroy(context);
  return NULL;
}
static void halo_opacity(void *frame, int32_t value) {
  native_ui_frame_opacity(frame, (lv_opa_t)value);
}
void native_ui_cancel(NativeUiContext *context) {
  if (!context) return;
  if (context->halo_frame.object) {
    lv_anim_delete(&context->halo_frame, halo_opacity);
    lv_obj_set_hidden(context->halo_frame.object, true);
  }
  context->pending = 0;
  context->has_previous = 0;
}
void native_ui_destroy(NativeUiContext *context) {
  if (!context) return;
  native_ui_cancel(context);
  native_probe_ui_destroy(context->probe);
  if (context->actions) lv_group_delete(context->actions);
  if (context->display) lv_display_delete(context->display);
  for (unsigned index = 0; index < 4; ++index) native_ui_image_destroy(&context->images[index]);
  free(context->rgb);
  free(context->draw);
  free(context);
  if (--context_count == 0) lv_deinit();
}
int native_ui_animation_pending(const NativeUiContext *context) {
  return context && context->pending;
}
void native_ui_advance(NativeUiContext *context, unsigned milliseconds) {
  /* LVGL's timer clock is process-wide. Export one animated context at a time;
   * advancing one must never silently animate another held context. */
  if (!context || context_count != 1 || !context->pending ||
      context->previous.held || context->previous.suspended) return;
  unsigned remaining = FADE_MS - context->elapsed;
  if (milliseconds > remaining) milliseconds = remaining;
  context->elapsed += milliseconds;
  lv_tick_inc(milliseconds);
  lv_anim_refr_now();
  if (context->elapsed >= FADE_MS) context->pending = 0;
}
const uint8_t *native_ui_cargo(NativeUiContext *context,
                               const CompanionCargoView *view, int still) {
  if (!context || !view || context->failed) return NULL;
  native_probe_ui_hide(context->probe);
  lv_obj_set_hidden(context->cargo_root, false);
  int same = context->has_previous &&
      !strcmp(context->previous.identity, view->identity) &&
      context->previous.phase == view->phase && context->previous.accepted == view->accepted &&
      context->previous.failed == view->failed && context->previous.action_count == view->action_count &&
      context->previous.capsules == view->capsules &&
      !memcmp(context->previous.supplies, view->supplies, sizeof(view->supplies)) &&
      !strcmp(context->previous.feedback, view->feedback) &&
      !memcmp(context->previous.actions, view->actions, sizeof(view->actions));
  int focus_changed = same && context->previous.focus != view->focus;
  if (!same || still || view->suspended) native_ui_cancel(context);
  /* Static labels borrow only this context's fixed storage, never a caller's
   * Kit or stack projection. Ordinary updates do not allocate text buffers. */
  context->previous = *view;
  view = &context->previous;
  lv_label_set_text_static(context->title, view->title);
  lv_label_set_text_static(context->context, view->context);
  for (unsigned resource = 0; resource < 3; ++resource) {
    snprintf(context->quantity_text[resource], sizeof(context->quantity_text[resource]),
             "%u", view->supplies[resource]);
    lv_label_set_text_static(context->quantity[resource], context->quantity_text[resource]);
  }
  lv_label_set_text_static(context->capsule, view->accepted ? "Delivery record" : view->capsule);
  lv_obj_set_hidden(context->capsule_image, view->capsules == 0);
  if (view->accepted) {
    snprintf(context->receipt_text, sizeof(context->receipt_text), "%u Data / %u Energy / %u Essence\n%s",
             view->delivered[0], view->delivered[1], view->delivered[2],
             view->delivered_capsules ? "1 sample delivered to Lab" : "Supplies stored at Lab");
    lv_label_set_text_static(context->detail, view->failed ? view->detail : context->receipt_text);
  } else lv_label_set_text_static(context->detail, view->detail);
  lv_label_set_text_static(context->capacity, view->capacity);
  lv_label_set_text_static(context->feedback, view->feedback);
  for (unsigned action = 0; action < 2; ++action) {
    lv_obj_remove_state(context->buttons[action], LV_STATE_FOCUSED | LV_STATE_PRESSED);
    if (action >= view->action_count) lv_obj_set_hidden(context->buttons[action], true);
    else {
      lv_obj_set_hidden(context->buttons[action], false);
      lv_label_set_text_static(context->button_text[action], view->actions[action]);
    }
  }
  if (view->focus < view->action_count) {
    lv_obj_t *selected = context->buttons[view->focus];
    lv_group_focus_obj(selected);
    lv_obj_add_state(selected, LV_STATE_FOCUSED);
    if (view->pressed) lv_obj_add_state(selected, LV_STATE_PRESSED);
    lv_obj_set_hidden(context->focus_frame.object, false);
    lv_obj_set_pos(context->focus_frame.object, 24, view->focus ? 528 : 486);
    native_ui_frame_size(&context->focus_frame, 402, view->focus ? 30 : 38);
    lv_obj_set_hidden(context->halo_frame.object, false);
    lv_obj_set_pos(context->halo_frame.object, 21, view->focus ? 525 : 483);
    native_ui_frame_size(&context->halo_frame, 408, view->focus ? 36 : 44);
    if (focus_changed && !still && !view->held && !view->suspended) {
      lv_anim_delete(&context->halo_frame, halo_opacity);
      lv_anim_t animation;
      lv_anim_init(&animation);
      lv_anim_set_var(&animation, &context->halo_frame);
      lv_anim_set_exec_cb(&animation, halo_opacity);
      lv_anim_set_values(&animation, 0, 180);
      lv_anim_set_duration(&animation, FADE_MS);
      context->pending = lv_anim_start(&animation) != NULL;
      context->elapsed = 0;
    } else if (!context->pending) halo_opacity(&context->halo_frame, 180);
  } else {
    lv_obj_set_hidden(context->focus_frame.object, true);
    lv_obj_set_hidden(context->halo_frame.object, true);
    context->pending = 0;
  }
  context->has_previous = 1;
  lv_refr_now(context->display);
  return context->failed ? NULL : context->rgb;
}
const uint8_t *native_ui_probe(NativeUiContext *context, const CompanionProbeView *view) {
  if (!context || !view || context->failed) return NULL;
  native_ui_cancel(context);
  lv_obj_set_hidden(context->cargo_root, true);
  if (!native_probe_ui_update(context->probe, view)) return NULL;
  lv_refr_now(context->display);
  return context->failed ? NULL : context->rgb;
}
