#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#include "iot_button.h"
#include "button_gpio.h" 

#include "joystick.h"

#define JOY_X_GPIO 1
#define JOY_Y_GPIO 2
#define JOY_SW_GPIO 42

static const char *TAG = "JOYSTICK";

led_state_t current_led = {0, 0, 0, 1.0};

static adc_oneshot_unit_handle_t s_adc;
static adc_channel_t s_chan_x;
static adc_channel_t s_chan_y;
static button_handle_t s_btn_handle = NULL;

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
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc, channel, &chan_cfg));
    *out_channel = channel;
}

void configure_joystick(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &s_adc));

    configure_adc_pin(JOY_X_GPIO, &s_chan_x);
    configure_adc_pin(JOY_Y_GPIO, &s_chan_y);

    button_config_t btn_cfg = {
        .long_press_time = 1000,
        .short_press_time = 200,
    };

    button_gpio_config_t gpio_btn_cfg = {
        .gpio_num = JOY_SW_GPIO,
        .active_level = 0, 
    };

    esp_err_t err = iot_button_new_gpio_device(&btn_cfg, &gpio_btn_cfg, &s_btn_handle);

    if (err == ESP_OK && s_btn_handle != NULL) {
        iot_button_register_cb(s_btn_handle, BUTTON_SINGLE_CLICK, NULL, button_event_cb, NULL);
        iot_button_register_cb(s_btn_handle, BUTTON_DOUBLE_CLICK, NULL, button_event_cb, NULL);
        iot_button_register_cb(s_btn_handle, BUTTON_LONG_PRESS_START, NULL, button_event_cb, NULL);
        ESP_LOGI(TAG, "Button initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create button handle: %s", esp_err_to_name(err));
    }
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