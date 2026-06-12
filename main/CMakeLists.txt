# main component: application entry + display bring-up
idf_component_register(SRCS "main.c" "display.c" "input.c"
                       INCLUDE_DIRS "."
                       # esp_lcd + ST7365 driver: panel; esp_lvgl_port: LVGL task + flush
                       REQUIRES esp_lcd esp_lcd_st7365 driver esp_lvgl_port)
