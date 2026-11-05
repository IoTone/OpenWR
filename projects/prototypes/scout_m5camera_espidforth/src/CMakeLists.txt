idf_component_register(
    SRCS main.c ble_console.c scout_cam.c scout_net.c scout_wdog.c
    INCLUDE_DIRS "."
    REQUIRES forth esp_timer driver nvs_flash bt fatfs wear_levelling esp_psram esp_wifi esp_netif esp_event esp_http_server lwip
)
