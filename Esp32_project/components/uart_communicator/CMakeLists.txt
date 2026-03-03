idf_component_register(
    SRCS "uart_communicator.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_uart log driver freertos my_led
    mqtt_component
)