idf_component_register(
    SRCS "main.c" "epd.c" "skin_words.c" "net_words.c" "bundle.c" "ble_console.c" "ota_words.c" "ota_mnpkg.c"
    INCLUDE_DIRS "."
    REQUIRES forth esp_timer driver nvs_flash esp_wifi esp_netif esp_event
             esp_http_client esp-tls mbedtls json
             bt app_update esp_https_ota esp_partition monocypher
)
