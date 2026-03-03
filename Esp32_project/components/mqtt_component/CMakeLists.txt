idf_component_register(
    SRCS "mqtt_component.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_event log esp_netif esp_system nvs_flash mqtt aht20_driver bmp280_driver lsm6ds3_driver 
)