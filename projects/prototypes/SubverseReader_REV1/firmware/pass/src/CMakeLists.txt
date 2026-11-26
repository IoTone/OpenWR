# One source tree, two passes. platformio.ini passes -DPASS_BOARD=sr15 or sr16, which becomes
# BOARD_SR15 / BOARD_SR16; each board-specific file compiles to nothing on the other board.
idf_component_register(
    SRCS main.c console.c gfx.c font5x7.c keys.c pack.c reader.c words.c ble_console.c
         epd_crowpanel42.c store_sd.c battery_crowpanel.c
         epd_uc8253.c store_flash.c battery_xiao.c
    INCLUDE_DIRS "."
    REQUIRES forth driver esp_timer nvs_flash fatfs vfs esp_driver_spi esp_driver_gpio bt esp_adc
             sdmmc esp_driver_sdspi esp_driver_uart wear_levelling spi_flash esp_driver_usb_serial_jtag
)
if(PASS_BOARD STREQUAL "sr16")
    target_compile_definitions(${COMPONENT_LIB} PUBLIC BOARD_SR16)
else()
    target_compile_definitions(${COMPONENT_LIB} PUBLIC BOARD_SR15)
endif()
