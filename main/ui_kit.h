/**
 * @file ui_kit.h
 * @brief UMP default-styled LVGL components.
 *
 * Apps and the UI shell compose screens from these factories. Styling lives
 * inside ui_kit.c — do not call lv_obj_set_style_* from sub-app modules.
 * Per-widget overrides may be added later via optional factory flags.
 */

#pragma once

#include "lvgl.h"

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *content;
} ui_page_t;

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *list;
} ui_menu_page_t;

/** Call once from ui_init before creating screens (disables LVGL default theme). */
void ui_kit_init(lv_display_t *disp);

/** Full-screen page-colored root screen. */
lv_obj_t *ui_screen_create(void);

/** Standard page: title, content slot, optional footer hint. */
ui_page_t ui_page_create(const char *title, const char *hint);

/** Wrap label inside a page content slot (top-aligned body text). */
lv_obj_t *ui_body_label(ui_page_t *page, const char *text);

/** Placeholder page: title, centered body, optional hint. */
lv_obj_t *ui_static_page_create(const char *title, const char *body, const char *hint);

/** Menu page: title and bottom flex list region. */
ui_menu_page_t ui_menu_page_create(const char *title);

/** Flex column list inside a page content slot (top-aligned, scrollable). */
lv_obj_t *ui_list_create(lv_obj_t *parent);

/** List row with default unselected styling. */
lv_obj_t *ui_list_row_create(lv_obj_t *list, const char *label);

/** Toggle list row highlight (selected vs normal). */
void ui_list_row_set_selected(lv_obj_t *row, bool selected);
