/**
 * @file input.h
 * @brief Navigation input: tact buttons + EC11 quadrature encoder → nav_event_t.
 *
 * Wiring: GPIO ── switch ── GND with internal pull-up (INPUT_ACTIVE_LEVEL in board.h).
 * Encoder CLK/DT are decoded to NAV_EVT_LEFT / NAV_EVT_RIGHT; ENC_SW → NAV_EVT_SELECT.
 */

#pragma once

#include "esp_err.h"

typedef enum {
    NAV_EVT_MENU,
    NAV_EVT_BACK,
    NAV_EVT_LEFT,
    NAV_EVT_RIGHT,
    NAV_EVT_SELECT,
} nav_event_t;

typedef void (*input_nav_cb_t)(nav_event_t event);

/**
 * @brief Configure GPIOs and start the input polling task.
 * @param callback  Called on each debounced press or encoder step (may be NULL).
 */
esp_err_t input_init(input_nav_cb_t callback);
