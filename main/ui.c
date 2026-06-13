/**
 * @file ui.c
 * @brief Home, menu list, and setting/detail screens.
 */

#include "ui.h"

#include "board.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"

#include <stddef.h>

static const char *TAG = "ui";

typedef enum {
    UI_SCR_HOME,
    UI_SCR_MENU,
    UI_SCR_FILES,
    UI_SCR_PLAYBACK,
    UI_SCR_BRIGHTNESS,
    UI_SCR_COUNT,
} ui_screen_id_t;

typedef struct {
    const char *label;
    ui_screen_id_t target;
} menu_item_t;

static lv_obj_t *s_screens[UI_SCR_COUNT];
static ui_screen_id_t s_current = UI_SCR_HOME;

static lv_obj_t *s_menu_rows[3];
static int s_menu_sel;

static const menu_item_t s_menu_items[] = {
    { "Files",      UI_SCR_FILES },
    { "Playback",   UI_SCR_PLAYBACK },
    { "Brightness", UI_SCR_BRIGHTNESS },
};

static const char *screen_name(ui_screen_id_t id)
{
    switch (id) {
    case UI_SCR_HOME:       return "home";
    case UI_SCR_MENU:       return "menu";
    case UI_SCR_FILES:      return "files";
    case UI_SCR_PLAYBACK:   return "playback";
    case UI_SCR_BRIGHTNESS: return "brightness";
    default:                return "?";
    }
}

static void show_screen_id(ui_screen_id_t id)
{
    lv_screen_load(s_screens[id]);
    s_current = id;
    ESP_LOGI(TAG, "screen: %s", screen_name(id));
}

/** Reusable detail screen: title + body (for TBD debug pages). */
static lv_obj_t *create_setting_screen(const char *title, const char *body)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *heading = lv_label_create(scr);
    lv_label_set_text(heading, title);
    lv_obj_set_style_text_color(heading, lv_color_white(), 0);
    lv_obj_align(heading, LV_ALIGN_TOP_MID, 0, 24);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, body);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_width(label, LCD_WIDTH - 40);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Back: menu");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -16);

    return scr;
}

static void menu_highlight_update(void)
{
    const int count = sizeof(s_menu_items) / sizeof(s_menu_items[0]);

    for (int i = 0; i < count; i++) {
        lv_obj_t *row = s_menu_rows[i];
        if (i == s_menu_sel) {
            lv_obj_set_style_bg_color(row, lv_color_hex(0x404040), 0);
            lv_obj_set_style_border_color(row, lv_color_white(), 0);
            lv_obj_set_style_border_width(row, 1, 0);
        } else {
            lv_obj_set_style_bg_color(row, lv_color_hex(0x181818), 0);
            lv_obj_set_style_border_width(row, 0, 0);
        }
    }
}

static lv_obj_t *create_menu_screen(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *heading = lv_label_create(scr);
    lv_label_set_text(heading, "Menu");
    lv_obj_set_style_text_color(heading, lv_color_white(), 0);
    lv_obj_align(heading, LV_ALIGN_TOP_MID, 0, 24);

    lv_obj_t *list = lv_obj_create(scr);
    lv_obj_set_size(list, LCD_WIDTH - 32, LCD_HEIGHT - 100);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, -12);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_row(list, 8, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    const int count = sizeof(s_menu_items) / sizeof(s_menu_items[0]);
    for (int i = 0; i < count; i++) {
        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, 48);
        lv_obj_set_style_pad_all(row, 12, 0);
        lv_obj_set_style_radius(row, 4, 0);

        lv_obj_t *label = lv_label_create(row);
        lv_label_set_text(label, s_menu_items[i].label);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

        s_menu_rows[i] = row;
    }

    s_menu_sel = 0;
    menu_highlight_update();
    return scr;
}

static void menu_move_selection(int delta)
{
    const int count = sizeof(s_menu_items) / sizeof(s_menu_items[0]);
    int next = s_menu_sel + delta;

    if (next < 0 || next >= count) {
        return;
    }

    s_menu_sel = next;
    menu_highlight_update();
}

esp_err_t ui_init(lv_display_t *disp)
{
    (void)disp;

    if (!lvgl_port_lock(0)) {
        ESP_LOGE(TAG, "LVGL lock failed in ui_init");
        return ESP_FAIL;
    }

    s_screens[UI_SCR_HOME] = create_setting_screen("Home", "Now playing: (none)");
    s_screens[UI_SCR_MENU] = create_menu_screen();
    s_screens[UI_SCR_FILES] = create_setting_screen("Files", "TBD\n\nSD mount and file list.");
    s_screens[UI_SCR_PLAYBACK] = create_setting_screen("Playback", "TBD\n\nAudio pipeline test.");
    s_screens[UI_SCR_BRIGHTNESS] = create_setting_screen("Brightness", "TBD\n\nBacklight PWM test.");

    show_screen_id(UI_SCR_HOME);

    lvgl_port_unlock();
    return ESP_OK;
}

void ui_on_nav(nav_event_t event)
{
    if (!lvgl_port_lock(0)) {
        return;
    }

    switch (event) {
    case NAV_EVT_MENU:
        if (s_current == UI_SCR_HOME) {
            show_screen_id(UI_SCR_MENU);
        } else if (s_current == UI_SCR_MENU) {
            show_screen_id(UI_SCR_HOME);
        } else {
            show_screen_id(UI_SCR_MENU);
        }
        break;

    case NAV_EVT_BACK:
        if (s_current == UI_SCR_HOME) {
            break;
        }
        if (s_current == UI_SCR_MENU) {
            show_screen_id(UI_SCR_HOME);
        } else {
            show_screen_id(UI_SCR_MENU);
        }
        break;

    case NAV_EVT_LEFT:
        if (s_current == UI_SCR_MENU) {
            menu_move_selection(-1);
        }
        break;

    case NAV_EVT_RIGHT:
        if (s_current == UI_SCR_MENU) {
            menu_move_selection(1);
        }
        break;

    case NAV_EVT_SELECT:
        if (s_current == UI_SCR_MENU) {
            show_screen_id(s_menu_items[s_menu_sel].target);
        }
        break;

    default:
        break;
    }

    lvgl_port_unlock();
}
