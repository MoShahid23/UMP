/**
 * @file display.c
 * @brief Low-level LCD (esp_lcd + ST7365) and LVGL port integration.
 *
 * Split into display_init_panel() (hardware only) and display_init() (+ LVGL).
 * LVGL draws into partial buffers in PSRAM; esp_lvgl_port flushes dirty regions
 * to the panel using the same esp_lcd_panel_handle_t as the old test pattern.
 */

#include "driver/spi_master.h"
#include "display.h"

#include "board.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_st7365.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"

static const char *TAG = "display";

/*
 * LVGL draw buffer size (in pixels, not bytes).
 * Partial refresh: LVGL renders in chunks; esp_lvgl_port sends each chunk over SPI.
 * 40 lines × 320 px = 12 800 px × 2 bytes ≈ 25 KB per buffer; ×2 for double_buffer.
 * Full frame (320×480) would be ~300 KB per buffer — unnecessary for stage 1.
 */
#define LCD_LVGL_BUF_LINES  40
#define LCD_LVGL_BUF_PIXELS (LCD_WIDTH * LCD_LVGL_BUF_LINES)

/* Panel handles required by lvgl_port_add_disp(); kept here, not exposed in the header. */
static esp_lcd_panel_io_handle_t s_io;
static esp_lcd_panel_handle_t s_panel;

/**
 * @brief Initialize SPI and the ST7365 panel (no LVGL).
 *
 * Same sequence as before LVGL: bus → panel I/O → chip driver → reset → init.
 */
static esp_err_t display_init_panel(void)
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_PIN_SCLK,
        .mosi_io_num = LCD_PIN_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        /* Largest single transfer we may queue (full-screen RGB565 bitmap). */
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_PIN_DC,
        .cs_gpio_num = LCD_PIN_CS,
        .pclk_hz = LCD_SPI_HZ,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_RETURN_ON_ERROR(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &s_io),
        TAG, "panel IO init failed");

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = 16,
        .flags = {
            .reset_active_high = false,
        },
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7365(s_io, &panel_config, &s_panel), TAG, "ST7365 init failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel), TAG, "panel reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel), TAG, "panel init failed");

    return ESP_OK;
}

esp_err_t display_init(lv_display_t **out_disp)
{
    ESP_RETURN_ON_FALSE(out_disp, ESP_ERR_INVALID_ARG, TAG, "display out pointer required");

    ESP_RETURN_ON_ERROR(display_init_panel(), TAG, "panel init failed");

    /*
     * esp_lvgl_port: FreeRTOS task + timer call lv_timer_handler() for us.
     * ESP_LVGL_PORT_INIT_CONFIG() sets task priority, stack, 5 ms tick period, etc.
     */
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port init failed");

    /*
     * Register our esp_lcd panel with LVGL. The port installs flush callbacks that
     * call esp_lcd_panel_draw_bitmap() (same path as the old line-by-line test pattern).
     */
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = s_io,
        .panel_handle = s_panel,
        .buffer_size = LCD_LVGL_BUF_PIXELS,
        .double_buffer = true,
        .hres = LCD_WIDTH,
        .vres = LCD_HEIGHT,
        .monochrome = false,
        /* Panel mount / ST7365 MADCTL: mirror_x corrects horizontally flipped LVGL output. */
        .rotation = {
            .swap_xy = false,
            .mirror_x = true,
            .mirror_y = false,
        },
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .flags = {
            /*
             * buff_spiram: draw buffers in PSRAM (8 MB on N16R8). Keeps internal SRAM free.
             * buff_dma: false — DMA-capable buffers must be in internal RAM; we use PSRAM.
             * swap_bytes: LVGL RGB565 byte order vs what ST7365 SPI expects. Toggle if colors wrong.
             */
            .buff_dma = false,
            .buff_spiram = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = true,
#endif
        },
    };

    lv_display_t *disp = lvgl_port_add_disp(&disp_cfg);
    ESP_RETURN_ON_FALSE(disp, ESP_FAIL, TAG, "lvgl_port_add_disp failed");

    *out_disp = disp;
    ESP_LOGI(TAG, "LVGL display ready (%dx%d, buf=%d px)", LCD_WIDTH, LCD_HEIGHT, LCD_LVGL_BUF_PIXELS);
    return ESP_OK;
}
