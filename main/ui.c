/**
 * @file ui.c
 * @brief UI shell: screen registry and navigation routing.
 */

#include "ui.h"

#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "files.h"
#include "ui_kit.h"

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

    if (id == UI_SCR_FILES) {
        files_on_show();
    }
}

static void menu_highlight_update(void)
{
    const int count = sizeof(s_menu_items) / sizeof(s_menu_items[0]);

    for (int i = 0; i < count; i++) {
        ui_list_row_set_selected(s_menu_rows[i], i == s_menu_sel);
    }
}

static lv_obj_t *create_menu_screen(void)
{
    ui_menu_page_t menu = ui_menu_page_create("Menu");
    const int count = sizeof(s_menu_items) / sizeof(s_menu_items[0]);

    for (int i = 0; i < count; i++) {
        s_menu_rows[i] = ui_list_row_create(menu.list, s_menu_items[i].label);
    }

    s_menu_sel = 0;
    menu_highlight_update();
    return menu.screen;
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
    if (!lvgl_port_lock(0)) {
        ESP_LOGE(TAG, "LVGL lock failed in ui_init");
        return ESP_FAIL;
    }

    ui_kit_init(disp);

    s_screens[UI_SCR_HOME] = ui_static_page_create("Home", "Now playing: (none)", NULL);
    s_screens[UI_SCR_MENU] = create_menu_screen();
    s_screens[UI_SCR_FILES] = files_screen_create();
    s_screens[UI_SCR_PLAYBACK] = ui_static_page_create("Playback", "TBD\n\nAudio pipeline test.", "Back: menu");
    s_screens[UI_SCR_BRIGHTNESS] = ui_static_page_create("Brightness", "TBD\n\nBacklight PWM test.", "Back: menu");

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
        } else if (s_current == UI_SCR_FILES) {
            files_on_nav(event);
        }
        break;

    case NAV_EVT_RIGHT:
        if (s_current == UI_SCR_MENU) {
            menu_move_selection(1);
        } else if (s_current == UI_SCR_FILES) {
            files_on_nav(event);
        }
        break;

    case NAV_EVT_SELECT:
        if (s_current == UI_SCR_MENU) {
            show_screen_id(s_menu_items[s_menu_sel].target);
        } else if (s_current == UI_SCR_FILES) {
            files_on_nav(event);
        }
        break;

    default:
        break;
    }

    lvgl_port_unlock();
}
