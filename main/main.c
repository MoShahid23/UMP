/**
 * @file main.c
 * @brief Application entry: display, UI, input.
 */

#include "display.h"
#include "input.h"
#include "ui.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ump";

void app_main(void)
{
    ESP_LOGI(TAG, "UMP starting");

    lv_display_t *disp = NULL;
    ESP_ERROR_CHECK(display_init(&disp));
    ESP_ERROR_CHECK(ui_init(disp));
    ESP_ERROR_CHECK(input_init(ui_on_nav));

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
