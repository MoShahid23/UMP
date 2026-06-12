/**
 * @file input.c
 * @brief Poll tact buttons + decode EC11 quadrature encoder.
 */

#include "input.h"

#include "board.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdbool.h>
#include <stdint.h>

static const char *TAG = "input";

#define INPUT_POLL_MS        10
#define INPUT_DEBOUNCE_MS    40
/** Valid CLK/DT transitions per one encoder detent (EC11 ≈ 4). */
#define ENC_STEPS_PER_DETENT 4

typedef struct {
    gpio_num_t gpio;
    nav_event_t event;
    const char *name;
    bool last_pressed;
    uint32_t last_fire_ms;
} button_t;

static input_nav_cb_t s_nav_cb;

static button_t s_buttons[] = {
    { BT_MENU_GPIO, NAV_EVT_MENU,   "MENU",   false, 0 },
    { BT_BACK_GPIO, NAV_EVT_BACK,   "BACK",   false, 0 },
    { ENC_SW_GPIO,  NAV_EVT_SELECT, "SELECT", false, 0 },
};

/*
 * Quadrature state transition table (prev<<2 | curr) → -1, 0, or +1 step.
 * See comment block below input_task for how this is used.
 */
static const int8_t s_enc_transition[] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0,
};

static uint8_t s_enc_last_state;
static int8_t s_enc_accum;

static bool input_active(gpio_num_t gpio)
{
    return gpio_get_level(gpio) == INPUT_ACTIVE_LEVEL;
}

static void dispatch_event(nav_event_t event, const char *label)
{
    ESP_LOGI(TAG, "%s", label);
    if (s_nav_cb) {
        s_nav_cb(event);
    }
}

static void dispatch_press(button_t *btn)
{
    dispatch_event(btn->event, btn->name);
}

static uint8_t encoder_read_state(void)
{
    uint8_t a = gpio_get_level(ENC_CLK_GPIO) ? 1 : 0;
    uint8_t b = gpio_get_level(ENC_DT_GPIO) ? 1 : 0;
    return (a << 1) | b;
}

static void encoder_poll(void)
{
    uint8_t state = encoder_read_state();
    if (state == s_enc_last_state) {
        return;
    }

    uint8_t index = (s_enc_last_state << 2) | state;
    int8_t delta = s_enc_transition[index];
    s_enc_last_state = state;

    if (delta == 0) {
        return;
    }

    s_enc_accum += delta;

    while (s_enc_accum >= ENC_STEPS_PER_DETENT) {
        s_enc_accum -= ENC_STEPS_PER_DETENT;
        dispatch_event(NAV_EVT_RIGHT, "rotate: RIGHT");
    }
    while (s_enc_accum <= -ENC_STEPS_PER_DETENT) {
        s_enc_accum += ENC_STEPS_PER_DETENT;
        dispatch_event(NAV_EVT_LEFT, "rotate: LEFT");
    }
}

static void input_task(void *arg)
{
    (void)arg;

    s_enc_last_state = encoder_read_state();
    s_enc_accum = 0;

    while (1) {
        uint32_t now_ms = pdTICKS_TO_MS(xTaskGetTickCount());

        encoder_poll();

        for (size_t i = 0; i < sizeof(s_buttons) / sizeof(s_buttons[0]); i++) {
            button_t *btn = &s_buttons[i];
            bool pressed = input_active(btn->gpio);

            if (pressed && !btn->last_pressed) {
                if ((now_ms - btn->last_fire_ms) >= INPUT_DEBOUNCE_MS) {
                    btn->last_fire_ms = now_ms;
                    dispatch_press(btn);
                }
            }

            btn->last_pressed = pressed;
        }

        vTaskDelay(pdMS_TO_TICKS(INPUT_POLL_MS));
    }
}

esp_err_t input_init(input_nav_cb_t callback)
{
    s_nav_cb = callback;

    gpio_config_t cfg = {
        .pin_bit_mask =
            (1ULL << BT_MENU_GPIO) | (1ULL << BT_BACK_GPIO) | (1ULL << ENC_SW_GPIO) |
            (1ULL << ENC_CLK_GPIO) | (1ULL << ENC_DT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));

    BaseType_t ok = xTaskCreate(input_task, "input", 3072, NULL, 5, NULL);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "failed to start input task");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "input ready (buttons + encoder, active level %d)", INPUT_ACTIVE_LEVEL);
    return ESP_OK;
}
