idf_component_register(
    SRCS main.c ble_console.c hub_lcd.c esp_lcd_jd9853.c font5x7.c hub_touch.c hub_reel.c hub_net.c hub_shoot.c hub_web.c
    INCLUDE_DIRS "."
    REQUIRES forth esp_timer driver nvs_flash bt fatfs wear_levelling esp_wifi esp_netif esp_event esp_http_client esp_lcd lwip esp_http_server mbedtls
)
