#include "ui_theme.h"
#include "core_art.h"
void native_ui_surface(lv_obj_t *object, uint32_t fill, uint32_t border, unsigned border_width) {
  lv_obj_remove_style_all(object);
  lv_obj_set_scrollable(object, false);
  lv_obj_set_clickable(object, false);
  lv_obj_set_style_bg_color(object, lv_color_hex(fill), 0);
  lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(object, lv_color_hex(border), 0);
  lv_obj_set_style_border_width(object, border_width, 0);
  lv_obj_set_style_radius(object, 0, 0);
  lv_obj_set_style_pad_all(object, 0, 0);
}
void native_ui_text(lv_obj_t *object, const lv_font_t *font, uint32_t color) {
  lv_obj_remove_style_all(object);
  lv_obj_set_scrollable(object, false);
  lv_obj_set_clickable(object, false);
  lv_obj_set_style_text_font(object, font, 0);
  lv_obj_set_style_text_color(object, lv_color_hex(color), 0);
  lv_obj_set_style_text_line_space(object, 0, 0);
}
void native_ui_action(lv_obj_t *object) {
  native_ui_surface(object, CORE_ART_SHADOW_RGB, CORE_ART_FIELD_RGB, 2);
  lv_obj_set_style_border_color(object, lv_color_hex(CORE_ART_FOCUS_RGB), LV_STATE_FOCUSED);
  lv_obj_set_style_border_width(object, 2, LV_STATE_FOCUSED);
  lv_obj_set_style_bg_color(object, lv_color_hex(CORE_ART_FIELD_RGB), LV_STATE_PRESSED);
}
