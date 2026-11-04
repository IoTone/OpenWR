idf_component_register(
    SRCS main.c net_words.c ble_console.c sony_http.c display.c
    INCLUDE_DIRS "."
    REQUIRES forth esp_timer driver nvs_flash esp_wifi esp_netif esp_event
             esp_http_client esp-tls mbedtls lwip bt
)
