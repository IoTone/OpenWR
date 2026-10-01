idf_component_register(
    SRCS "main.c" "ssd1309.c" "goggle_words.c"
    INCLUDE_DIRS "."
    REQUIRES forth esp_timer driver
)
