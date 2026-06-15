/**
 * @file storage.h
 * @brief Shared SPI2 bus (LCD + SD) and optional SD card mount.
 */

#pragma once

#include "esp_err.h"

/** Initialize SPI2 shared by LCD and SD card. Call once before display or storage. */
esp_err_t spi_bus_init(void);

/** Mount FAT volume at /sdcard. Non-fatal if card missing or unformatted. */
esp_err_t storage_mount(void);
