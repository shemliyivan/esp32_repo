idf_component_register(
    SRCS "bluetooth_component.c"
    INCLUDE_DIRS "include"
    REQUIRES log nvs_flash bt
    mqtt_component
    "espressif__led_strip"
)