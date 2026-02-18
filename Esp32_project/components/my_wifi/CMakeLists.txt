idf_component_register(
    SRCS "my_wifi.c"
    INCLUDE_DIRS "include"

    REQUIRES esp_wifi esp_event nvs_flash esp_http_server
)