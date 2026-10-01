#ifndef CRITTER_UI_THEME_H
#define CRITTER_UI_THEME_H
#include "lvgl.h"
void native_ui_surface(lv_obj_t *object, uint32_t fill, uint32_t border, unsigned border_width);
void native_ui_text(lv_obj_t *object, const lv_font_t *font, uint32_t color);
void native_ui_action(lv_obj_t *object);
#endif
