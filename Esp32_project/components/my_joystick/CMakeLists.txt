idf_component_register(
    SRCS "joystick.c"
    INCLUDE_DIRS "include"
    REQUIRES "esp_adc" "driver" "freertos" "log" "espressif__button"
)