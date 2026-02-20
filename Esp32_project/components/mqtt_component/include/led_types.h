#ifndef LED_TYPES_H
#define LED_TYPES_H

typedef enum {
    LED_MODE_WIFI_STATUS,
    LED_MODE_MQTT_MANUAL
} led_mode_t;

extern led_mode_t g_led_mode;

#endif