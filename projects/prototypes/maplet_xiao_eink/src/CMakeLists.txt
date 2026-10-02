set(MAPLET_SRCS main.c epd.c skin_words.c net_words.c bundle.c ble_console.c ota_words.c ota_mnpkg.c cam_words.c)
set(MAPLET_REQ forth esp_timer driver nvs_flash esp_wifi esp_netif esp_event
    esp_http_client esp-tls mbedtls json
    bt app_update esp_https_ota esp_partition monocypher
    fatfs sdmmc vfs esp_driver_ledc)
if(IDF_TARGET STREQUAL "esp32s3")
    list(APPEND MAPLET_REQ espressif__esp32-camera)
endif()
idf_component_register(
    SRCS ${MAPLET_SRCS}
    INCLUDE_DIRS "."
    REQUIRES ${MAPLET_REQ}
)
