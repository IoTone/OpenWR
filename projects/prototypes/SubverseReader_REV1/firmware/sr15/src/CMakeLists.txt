idf_component_register(
    SRCS main.c epd.c font5x7.c keys.c sd.c pack.c reader.c words.c ble_console.c battery.c
    INCLUDE_DIRS "."
    REQUIRES forth driver esp_timer nvs_flash fatfs sdmmc vfs esp_driver_uart esp_driver_spi esp_driver_gpio esp_driver_sdspi bt esp_adc
)
