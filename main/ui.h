/**
 * @file ui.h
 * @brief LVGL screens and navigation (home / menu).
 */

#pragma once

#include "esp_err.h"
#include "input.h"
#include "lvgl.h"

/** Create screens and show home. Call once after display_init(), before input_init(). */
esp_err_t ui_init(lv_display_t *disp);

/** Handle nav events from input.c (uses lvgl_port_lock internally). */
void ui_on_nav(nav_event_t event);
