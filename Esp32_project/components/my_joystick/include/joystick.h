#pragma once

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

void configure_adc_pin(int gpio, adc_channel_t* out_channel);
void configure_joystick(void);
void read_joystick(int* x, int* y, bool* sw_pressed);
void button_event_cb(void* arg, void* data);

typedef struct {
    uint8_t r, g, b;
    float brightness; 
} led_state_t;

led_state_t current_led = {0, 0, 0, 1.0};