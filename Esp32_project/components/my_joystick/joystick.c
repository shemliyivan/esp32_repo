#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "iot_button.h"

#define JOY_X_GPIO 1
#define JOY_Y_GPIO 2
#define JOY_SW_GPIO 3

static const char *TAG = "JOYSTICK";

static adc_oneshot_unit_handle_t s_adc;
static adc_channel_t s_chan_x;
static adc_channel_t s_chan_y;

void configure_adc_pin(int gpio, adc_channel_t *out_channel)
{
    adc_unit_t unit;
    adc_channel_t channel;

    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(gpio, &unit, &channel));
    
    if(unit != ADC_UNIT_1){
        ESP_LOGE(TAG, "GPIO %d mapped to ADC%d. Використовуйте ADC1!", gpio, unit + 1);
        return; 
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc, channel, &chan_cfg));
    *out_channel = channel;
}

void configure_joystick(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &s_adc));

    configure_adc_pin(JOY_X_GPIO, &s_chan_x);
    configure_adc_pin(JOY_Y_GPIO, &s_chan_y);

    gpio_config_t sw_cfg = {
        .pin_bit_mask = (1ULL << JOY_SW_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&sw_cfg));
}

void read_joystick(int *x, int *y, bool *sw_pressed)
{   
    ESP_ERROR_CHECK(adc_oneshot_read(s_adc, s_chan_x, x));
    ESP_ERROR_CHECK(adc_oneshot_read(s_adc, s_chan_y, y));
    *sw_pressed = (gpio_get_level(JOY_SW_GPIO) == 0); 
}

void button_event_cb(void* arg, void* data){
    button_event_t event = iot_button_get_event(arg);
    switch (event) {
        case BUTTON_SINGLE_CLICK: ESP_LOGI(TAG, "Кнопка: Одинарний клік"); break;
        case BUTTON_DOUBLE_CLICK: ESP_LOGI(TAG, "Кнопка: Подвійний клік"); break;
        case BUTTON_LONG_PRESS_START: ESP_LOGI(TAG, "Кнопка: Довге натискання"); break;
        default: break;
    }
}