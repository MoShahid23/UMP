#pragma once

#include "driver/gpio.h"

#define LCD_PIN_MOSI  GPIO_NUM_13
#define LCD_PIN_SCLK  GPIO_NUM_10
#define LCD_PIN_CS    GPIO_NUM_9
#define LCD_PIN_DC    GPIO_NUM_11
#define LCD_PIN_RST   GPIO_NUM_46

#define LCD_WIDTH  320
#define LCD_HEIGHT 480

#define LCD_SPI_HOST  SPI2_HOST
#define LCD_SPI_HZ    (40 * 1000 * 1000)

/* Tact buttons: GPIO ── switch ── GND, internal pull-up */
#define BT_MENU_GPIO    GPIO_NUM_15
#define BT_BACK_GPIO    GPIO_NUM_4

/* EC11 encoder: CLK + DT = quadrature A/B; SW = shaft push (SELECT) */
#define ENC_CLK_GPIO    GPIO_NUM_5
#define ENC_DT_GPIO     GPIO_NUM_6
#define ENC_SW_GPIO     GPIO_NUM_7

/** Active when pin reads low (wired to GND through switch). */
#define INPUT_ACTIVE_LEVEL  0
