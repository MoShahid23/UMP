# UMP

ESP32-S3 firmware for the UMP device.

## SPI (shared: LCD + SD)

| Signal | GPIO | Device |
|--------|------|--------|
| MOSI   | 13   | bus    |
| MISO   | 8    | bus    |
| SCLK   | 10   | bus    |
| CS     | 9    | LCD    |
| DC     | 11   | LCD    |
| RST    | 46   | LCD    |
| CS     | 18   | SD     |

UI uses LVGL via `esp_lvgl_port`. Boot shows **Home**; **Menu** toggles home/menu; **Back** returns home.

## Roadmap (stage 1)

- [x] ST7365 test pattern
- [x] LVGL on display
- [x] Input (buttons + encoder)
- [ ] Home / menu / songs screens (menu list + debug detail pages done)
- [ ] Play one audio file
- [ ] Songs list from storage
- [ ] Home / settings content

## Build

```bash
idf.py set-target esp32s3
idf.py build flash monitor
```

If colors look wrong, toggle `swap_bytes` in `main/display.c` (`display_init` → `lvgl_port_display_cfg_t.flags`).
