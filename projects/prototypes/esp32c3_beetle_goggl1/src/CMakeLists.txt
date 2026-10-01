idf_component_register(
    SRCS "main.c" "ssd1309.c" "goggle_words.c" "net_words.c" "wx_words.c" "wx_sched.c" "bundle.c" "ble_console.c" "ota_words.c"
    INCLUDE_DIRS "."
    REQUIRES forth esp_timer driver nvs_flash esp_wifi esp_netif esp_event
             esp_http_client esp-tls mbedtls json
             bt app_update esp_https_ota
)
