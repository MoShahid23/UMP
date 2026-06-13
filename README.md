# UMP

ESP32-S3 firmware for the UMP device.

## Display

320×480 ST7365 panel over SPI:

| Signal | GPIO |
|--------|------|
| MOSI   | 13   |
| SCLK   | 10   |
| CS     | 9    |
| DC     | 11   |
| RST    | 46   |

UI uses LVGL via `esp_lvgl_port`. Boot shows **Home**; **Menu** toggles home/menu; **Back** returns home.

## Roadmap (stage 1)

- [x] ST7365 test pattern
- [x] LVGL on display
- [x] Input (buttons + encoder)
- [ ] Home / menu / songs screens (home + menu shell done)
- [ ] Play one audio file
- [ ] Songs list from storage
- [ ] Home / settings content

## Build

```bash
idf.py set-target esp32s3
idf.py build flash monitor
```

If colors look wrong, toggle `swap_bytes` in `main/display.c` (`display_init` → `lvgl_port_display_cfg_t.flags`).
