/**
 * @file ui_kit.c
 * @brief UMP default-styled LVGL component implementations.
 */

#include "ui_kit.h"

#include "board.h"

/* Theme tokens — private to this file. */
#define UI_COLOR_PAGE         0xFFE5B8
#define UI_COLOR_TEXT         0x000000
#define UI_COLOR_BTN          0x910A00
#define UI_COLOR_BTN_SEL      0xC74A3E
#define UI_COLOR_BTN_TEXT     0xFFFFFF

#define UI_TITLE_Y        24
#define UI_HINT_Y         -16
#define UI_CONTENT_X      20
#define UI_CONTENT_Y      56
#define UI_CONTENT_W      (LCD_WIDTH - 40)
#define UI_ROW_H          48
#define UI_ROW_PAD        12
#define UI_ROW_RADIUS     4
#define UI_LIST_PAD_ROW   8
#define UI_LIST_W         (LCD_WIDTH - 32)
#define UI_LIST_H         (LCD_HEIGHT - 100)
#define UI_LIST_BOTTOM    -12

static lv_style_t s_sty_page;
static lv_style_t s_sty_text;
static lv_style_t s_sty_btn;
static lv_style_t s_sty_btn_sel;
static lv_style_t s_sty_btn_text;
static lv_style_t s_sty_transparent;
static bool s_styles_ready;

static lv_color_t ui_color(uint32_t hex)
{
    return lv_color_hex(hex);
}

static void ui_styles_init(void)
{
    if (s_styles_ready) {
        return;
    }

    lv_style_init(&s_sty_page);
    lv_style_set_bg_color(&s_sty_page, ui_color(UI_COLOR_PAGE));
    lv_style_set_bg_opa(&s_sty_page, LV_OPA_COVER);
    lv_style_set_border_width(&s_sty_page, 0);
    lv_style_set_pad_all(&s_sty_page, 0);

    lv_style_init(&s_sty_text);
    lv_style_set_text_color(&s_sty_text, ui_color(UI_COLOR_TEXT));

    lv_style_init(&s_sty_btn);
    lv_style_set_bg_color(&s_sty_btn, ui_color(UI_COLOR_BTN));
    lv_style_set_bg_opa(&s_sty_btn, LV_OPA_COVER);
    lv_style_set_border_width(&s_sty_btn, 0);
    lv_style_set_radius(&s_sty_btn, UI_ROW_RADIUS);
    lv_style_set_pad_all(&s_sty_btn, UI_ROW_PAD);

    lv_style_init(&s_sty_btn_sel);
    lv_style_set_bg_color(&s_sty_btn_sel, ui_color(UI_COLOR_BTN_SEL));
    lv_style_set_bg_opa(&s_sty_btn_sel, LV_OPA_COVER);
    lv_style_set_border_width(&s_sty_btn_sel, 0);
    lv_style_set_radius(&s_sty_btn_sel, UI_ROW_RADIUS);
    lv_style_set_pad_all(&s_sty_btn_sel, UI_ROW_PAD);

    lv_style_init(&s_sty_btn_text);
    lv_style_set_text_color(&s_sty_btn_text, ui_color(UI_COLOR_BTN_TEXT));

    lv_style_init(&s_sty_transparent);
    lv_style_set_bg_opa(&s_sty_transparent, LV_OPA_TRANSP);
    lv_style_set_border_width(&s_sty_transparent, 0);

    s_styles_ready = true;
}

void ui_kit_init(lv_display_t *disp)
{
    ui_styles_init();
    if (disp != NULL) {
        /* Stop LVGL default theme from painting white card styles over ours. */
        lv_display_set_theme(disp, NULL);
    }
}

static lv_obj_t *ui_title_create(lv_obj_t *parent, const char *title)
{
    lv_obj_t *heading = lv_label_create(parent);
    lv_label_set_text(heading, title);
    lv_obj_add_style(heading, &s_sty_text, 0);
    lv_obj_align(heading, LV_ALIGN_TOP_MID, 0, UI_TITLE_Y);
    return heading;
}

static void ui_hint_create(lv_obj_t *parent, const char *hint)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, hint);
    lv_obj_add_style(label, &s_sty_text, 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, UI_HINT_Y);
}

lv_obj_t *ui_screen_create(void)
{
    ui_styles_init();

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &s_sty_page, 0);
    return scr;
}

ui_page_t ui_page_create(const char *title, const char *hint)
{
    ui_page_t page = {
        .screen = ui_screen_create(),
        .content = NULL,
    };

    ui_title_create(page.screen, title);

    page.content = lv_obj_create(page.screen);
    lv_obj_add_style(page.content, &s_sty_transparent, 0);
    lv_obj_set_size(page.content, UI_CONTENT_W, LCD_HEIGHT - UI_CONTENT_Y - 40);
    lv_obj_align(page.content, LV_ALIGN_TOP_LEFT, UI_CONTENT_X, UI_CONTENT_Y);
    lv_obj_set_style_pad_all(page.content, 0, 0);
    lv_obj_set_flex_flow(page.content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page.content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    if (hint != NULL) {
        ui_hint_create(page.screen, hint);
    }

    return page;
}

lv_obj_t *ui_body_label(ui_page_t *page, const char *text)
{
    lv_obj_t *label = lv_label_create(page->content);
    lv_label_set_text(label, text);
    lv_obj_add_style(label, &s_sty_text, 0);
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    return label;
}

lv_obj_t *ui_static_page_create(const char *title, const char *body, const char *hint)
{
    ui_page_t page = ui_page_create(title, hint);
    ui_body_label(&page, body);
    return page.screen;
}

static void ui_style_list_container(lv_obj_t *list)
{
    lv_obj_add_style(list, &s_sty_transparent, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, UI_LIST_PAD_ROW, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
}

lv_obj_t *ui_list_create(lv_obj_t *parent)
{
    ui_style_list_container(parent);
    lv_obj_set_width(parent, lv_pct(100));
    return parent;
}

ui_menu_page_t ui_menu_page_create(const char *title)
{
    ui_menu_page_t menu = {
        .screen = ui_screen_create(),
        .list = NULL,
    };

    ui_title_create(menu.screen, title);

    menu.list = lv_obj_create(menu.screen);
    lv_obj_set_size(menu.list, UI_LIST_W, UI_LIST_H);
    lv_obj_align(menu.list, LV_ALIGN_BOTTOM_MID, 0, UI_LIST_BOTTOM);
    ui_style_list_container(menu.list);

    return menu;
}

lv_obj_t *ui_list_row_create(lv_obj_t *list, const char *label_text)
{
    lv_obj_t *row = lv_obj_create(list);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, UI_ROW_H);
    ui_list_row_set_selected(row, false);

    lv_obj_t *label = lv_label_create(row);
    lv_label_set_text(label, label_text);
    lv_obj_add_style(label, &s_sty_btn_text, 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    return row;
}

void ui_list_row_set_selected(lv_obj_t *row, bool selected)
{
    lv_obj_remove_style(row, &s_sty_btn, 0);
    lv_obj_remove_style(row, &s_sty_btn_sel, 0);
    lv_obj_add_style(row, selected ? &s_sty_btn_sel : &s_sty_btn, 0);
}
