#include "iot_servo.h"
#include "driver/ledc.h"

#define SERVO_GPIO 4

void servo_init(){
    servo_config_t servo_cfg = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = 50,
        .timer_number = LEDC_TIMER_0,
        .channels = {
            .servo_pin = {
                SERVO_GPIO,
            },
            .ch = {
                LEDC_CHANNEL_0,
            },
        },
        .channel_number = 1,
    };

    iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_cfg);
}