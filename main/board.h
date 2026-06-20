#pragma once

#include "driver/gpio.h"

/* Shared SPI2 bus (LCD + SD card) */
#define SPI_HOST        SPI2_HOST
#define SPI_LCD_HZ      (40 * 1000 * 1000)

#define SPI_PIN_MISO    GPIO_NUM_8
#define SPI_PIN_MOSI    GPIO_NUM_13
#define SPI_PIN_SCLK    GPIO_NUM_10

/* LCD-only (not shared with SD) */
#define LCD_PIN_CS      GPIO_NUM_9
#define LCD_PIN_DC      GPIO_NUM_11
#define LCD_PIN_RST     GPIO_NUM_46

/* SD card (SPI mode — shares MOSI/SCLK/MISO, separate CS) */
#define SD_PIN_CS       GPIO_NUM_18

#define LCD_WIDTH  320
#define LCD_HEIGHT 480

/* DAC */
#define DAC_DIN GPIO_NUM_37
#define DAC_BCLK GPIO_NUM_36
#define DAC_LRC GPIO_NUM_35

/* Tact buttons: GPIO ── switch ── GND, internal pull-up */
#define BT_MENU_GPIO    GPIO_NUM_15
#define BT_BACK_GPIO    GPIO_NUM_4

/* EC11 encoder: CLK + DT = quadrature A/B; SW = shaft push (SELECT) */
#define ENC_CLK_GPIO    GPIO_NUM_5
#define ENC_DT_GPIO     GPIO_NUM_6
#define ENC_SW_GPIO     GPIO_NUM_7

/** Active when pin reads low (wired to GND through switch). */
#define INPUT_ACTIVE_LEVEL  0
