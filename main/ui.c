/**
 * @file ui.c
 * @brief Home and menu screens; MENU toggles, BACK returns home.
 */

#include "ui.h"

#include "esp_log.h"
#include "esp_lvgl_port.h"

static const char *TAG = "ui";

typedef enum {
    UI_SCREEN_HOME,
    UI_SCREEN_MENU,
} ui_screen_t;

static lv_obj_t *s_scr_home;
static lv_obj_t *s_scr_menu;
static ui_screen_t s_current = UI_SCREEN_HOME;

static lv_obj_t *create_screen(const char *title, const char *body)
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
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

static void show_screen(ui_screen_t screen)
{
    lv_obj_t *scr = (screen == UI_SCREEN_HOME) ? s_scr_home : s_scr_menu;
    lv_screen_load(scr);
    s_current = screen;
    ESP_LOGI(TAG, "screen: %s", screen == UI_SCREEN_HOME ? "home" : "menu");
}

esp_err_t ui_init(lv_display_t *disp)
{
    (void)disp;

    if (!lvgl_port_lock(0)) {
        ESP_LOGE(TAG, "LVGL lock failed in ui_init");
        return ESP_FAIL;
    }

    s_scr_home = create_screen("Home", "Now playing: (none)");
    s_scr_menu = create_screen("Menu", "Settings placeholder");
    show_screen(UI_SCREEN_HOME);

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
        show_screen(s_current == UI_SCREEN_HOME ? UI_SCREEN_MENU : UI_SCREEN_HOME);
        break;
    case NAV_EVT_BACK:
        if (s_current != UI_SCREEN_HOME) {
            show_screen(UI_SCREEN_HOME);
        }
        break;
    default:
        break;
    }

    lvgl_port_unlock();
}
