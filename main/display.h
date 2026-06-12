/**
 * @file display.h
 * @brief Display stack: ST7365 over SPI + LVGL via esp_lvgl_port.
 *
 * Hardware setup lives in board.h. This module owns panel init and the LVGL
 * display handle; application code builds widgets and must use lvgl_port_lock().
 */

#pragma once

#include "esp_err.h"
#include "lvgl.h"

/**
 * @brief Bring up the panel and register it with LVGL.
 *
 * Order: SPI bus → panel I/O → ST7365 driver → lvgl_port_init → lvgl_port_add_disp.
 * The esp_lcd handles stay inside display.c; only the LVGL display is returned.
 *
 * @param out_disp  Output: LVGL display handle for lv_display_get_screen_active(), etc.
 * @return ESP_OK on success, or an error from the failing step.
 */
esp_err_t display_init(lv_display_t **out_disp);
