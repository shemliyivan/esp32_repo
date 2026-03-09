idf_component_register(
    SRCS "main.c"                           
    INCLUDE_DIRS "."
    REQUIRES my_joystick my_led led_strip mqtt_component bluetooth_component uart_communicator i2c_bus spi_bus aht20_driver bmp280_driver lsm6ds3_driver my_servo stepper
    espressif__button nvs_flash esp_wifi esp_event my_wifi esp_netif esp_http_server esp_phy lwip driver espressif__servo
    json 
)