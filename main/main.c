/**
 * @file main.c
 * @brief Application entry: display/LVGL + button input bring-up.
 */

#include "display.h"
#include "input.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

static const char *TAG = "ump";

static void on_nav(nav_event_t event)
{
    /* Placeholder until ui.c handles screens; confirms callback path works. */
    (void)event;
}

void app_main(void)
{
    ESP_LOGI(TAG, "UMP starting");

    lv_display_t *disp = NULL;
    ESP_ERROR_CHECK(display_init(&disp));
    ESP_ERROR_CHECK(input_init(on_nav));

    if (lvgl_port_lock(0)) {
        lv_obj_t *scr = lv_display_get_screen_active(disp);
        lv_obj_t *label = lv_label_create(scr);
        lv_label_set_text(label, "UMP");
        lv_obj_center(label);
        lvgl_port_unlock();
    } else {
        ESP_LOGE(TAG, "LVGL lock failed");
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
