idf_component_register(
    SRCS "led.c"
    INCLUDE_DIRS "include"
    REQUIRES "espressif__led_strip"        # Основна бібліотека
    PRIV_REQUIRES "log" "driver" "freertos" # Виправлено: "log" замість "esp_log"
)