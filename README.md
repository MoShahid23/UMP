# UMP

ESP32-S3 firmware for the UMP personal music player (WeAct N16R8 + 3.5" ST7365 LCD + microSD).

## Architecture

| Module | Role |
|--------|------|
| `main.c` | Boot: SPI bus → SD mount (optional) → display → UI → input |
| `storage.c` | Shared SPI2 bus init; FAT mount at `/sdcard` |
| `display.c` | ST7365 panel + `esp_lvgl_port` |
| `ui.c` | Screen router (Home, Menu, Files, Playback, Brightness) |
| `ui_kit.c` | Theme + LVGL component factories (all styling lives here) |
| `files.c` | SD file browser with encoder selection |
| `input.c` | Buttons + EC11 encoder → `nav_event_t` |
| `components/esp_lcd_st7365/` | Custom ST7365 driver |

Boot shows **Home**. **Menu** toggles home ↔ menu. **Back** returns to home from menu, or to menu from sub-screens. Encoder **left/right** moves highlight on menu and file list; encoder **press** = SELECT.

## Pins

### SPI (shared: LCD + SD)

| Signal | GPIO | Device |
|--------|------|--------|
| MOSI   | 13   | bus    |
| MISO   | 8    | bus    |
| SCLK   | 10   | bus    |
| CS     | 9    | LCD    |
| DC     | 11   | LCD    |
| RST    | 46   | LCD    |
| CS     | 18   | SD     |

### Input

| Signal | GPIO |
|--------|------|
| Menu   | 15   |
| Back   | 4    |
| Enc CLK| 5    |
| Enc DT | 6    |
| Enc SW | 7    |

All inputs are active-low with internal pull-ups (switch to GND).

Backlight is not software-controllable yet — the breakout ties LED-K to GND. Dimming needs a GPIO-driven transistor on the cathode side.

## Display colors

The ST7365 path required more than toggling `swap_bytes`. Current working settings in `display.c` and the ST7365 driver:

- `rgb_ele_order = BGR`, `data_endian = BIG`
- `esp_lcd_panel_invert_color(true)` after panel init
- LVGL: `LV_COLOR_FORMAT_RGB565`, `swap_bytes = true`, `mirror_x = true`
- Driver `ramctl_val_2 = 0xC0` for RGB565 RAM control

The panel (UEED035HV-RX40-L001A) is transflective with limited gamut — theme colors in `ui_kit.c` look close but somewhat washed compared to the design palette.

## UI theme

Defined in `ui_kit.c` (do not set styles from other modules):

| Token | Color |
|-------|-------|
| Page background | `#FFE5B8` |
| Text | `#000000` |
| Button | `#910A00` |
| Selected | `#C74A3E` |
| Button text | `#FFFFFF` |

## Roadmap (stage 1)

- [x] ST7365 + LVGL display
- [x] Input (buttons + encoder)
- [x] UI shell (home, menu, placeholder pages)
- [x] `ui_kit` centralized styling
- [x] SD card mount + file browser
- [ ] File SELECT action (open dir / play)
- [ ] Play one audio file (`audio.c`)
- [ ] Home now-playing UI
- [ ] Backlight PWM (hardware + firmware)

## Build

```bash
idf.py set-target esp32s3
idf.py build flash monitor
```

Target board: ESP32-S3 with 8 MB OPI PSRAM (WeAct N16R8). If the build environment gets out of sync, run `idf.py fullclean` before rebuilding.
