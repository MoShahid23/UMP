/**
 * @file storage.c
 * @brief Shared SPI2 bus init and SD card mount (/sdcard).
 */

#include "storage.h"

#include "board.h"
#include "diskio_impl.h"
#include "diskio_sdmmc.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_vfs_fat.h"
#include "ff.h"
#include "sdmmc_cmd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "storage";
static bool s_mounted;

esp_err_t spi_bus_init(void)
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = SPI_PIN_SCLK,
        .mosi_io_num = SPI_PIN_MOSI,
        .miso_io_num = SPI_PIN_MISO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        /* Sized for largest SPI2 transfer (full-screen RGB565 flush). */
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
    };
    return spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
}

bool storage_is_ready(void)
{
    return s_mounted;
}

esp_err_t storage_mount(void)
{
    s_mounted = false;
    sdspi_dev_handle_t sd_handle;

    sdspi_device_config_t storage_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    storage_config.gpio_cs = SD_PIN_CS;
    storage_config.host_id = SPI_HOST;
    ESP_RETURN_ON_ERROR(sdspi_host_init(), TAG, "SDSPI host init failed");
    ESP_RETURN_ON_ERROR(sdspi_host_init_device(&storage_config, &sd_handle), TAG, "SDSPI device init failed");


    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = sd_handle;
    host.max_freq_khz = 10000;
    static sdmmc_card_t card;

    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (attempt > 0) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        err = sdmmc_card_init(&host, &card);
        if (err == ESP_OK) {
            break;
        }
        ESP_LOGW(TAG, "card init attempt %d: %s", attempt + 1, esp_err_to_name(err));
    }
    ESP_RETURN_ON_ERROR(err, TAG, "SDMMC card init failed");

    BYTE pdrv;
    ESP_RETURN_ON_ERROR(ff_diskio_get_drive(&pdrv), TAG, "no free FAT drive");
    ff_diskio_register_sdmmc(pdrv, &card);

    char drv[3] = { (char)('0' + pdrv), ':', 0 };
    FATFS *fs;
    ESP_RETURN_ON_ERROR(esp_vfs_fat_register(STORAGE_MOUNT_PATH, drv, 5, &fs), TAG, "FATFS VFS register failed");

    FRESULT res = f_mount(fs, drv, 1);
    if (res != FR_OK) {
        ESP_LOGE(TAG, "f_mount failed (%d)", res);
        return ESP_FAIL;
    }

    s_mounted = true;
    ESP_LOGI(TAG, "SD card mounted at %s", STORAGE_MOUNT_PATH);
    return ESP_OK;
}
