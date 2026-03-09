#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define PIN_IN1 GPIO_NUM_14
#define PIN_IN2 GPIO_NUM_13
#define PIN_IN3 GPIO_NUM_12
#define PIN_IN4 GPIO_NUM_11

static uint16_t step_index = 0; 

static const char* TAG = "Stepper";

static const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

void stepper_init(){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_IN1) | (1ULL << PIN_IN2) | (1ULL << PIN_IN3) | (1ULL << PIN_IN4),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

void move_stepper_degrees(float degrees) {
    float abs_degrees = (degrees < 0) ? -degrees : degrees;
    uint32_t steps = (uint32_t)(abs_degrees * 4096.0f / 360.0f);

    for (uint32_t i = 0; i < steps; i++) {
        gpio_set_level(PIN_IN1, step_sequence[step_index][0]);
        gpio_set_level(PIN_IN2, step_sequence[step_index][1]);
        gpio_set_level(PIN_IN3, step_sequence[step_index][2]);
        gpio_set_level(PIN_IN4, step_sequence[step_index][3]);

        step_index = (degrees > 0) ? (step_index + 1) % 8 : (step_index == 0 ? 7 : step_index - 1);

        ESP_LOGI(TAG, "Stepper is running\n");
    }

    gpio_set_level(PIN_IN1, 0);
    gpio_set_level(PIN_IN2, 0);
    gpio_set_level(PIN_IN3, 0);
    gpio_set_level(PIN_IN4, 0);
    
    ESP_LOGI(TAG, "Movement finished");
}