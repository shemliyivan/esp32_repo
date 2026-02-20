idf_component_register(
    SRCS "main.c"                           
    INCLUDE_DIRS "."
    REQUIRES my_joystick my_led led_strip mqtt_component
    espressif__button nvs_flash esp_wifi esp_event my_wifi esp_netif esp_http_server esp_phy lwip driver
    json
)