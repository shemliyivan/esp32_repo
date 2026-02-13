#include <stdbool.h>
#include "esp_log.h"
#include "esp_err.h"           
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_strip.h"         
#include "led_strip_rmt.h"     

#define GPIO_LED_RGB 48
#define LED_COUNT 1

static const char *TAG = "LED_INIT";

led_strip_handle_t configure_rgb_led()
{
    led_strip_handle_t led_strip = NULL;

    led_strip_config_t strip_config = {
        .strip_gpio_num = GPIO_LED_RGB,  
        .max_leds = LED_COUNT,                 
        .led_model = LED_MODEL_WS2812, 
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, 
        .flags = {
            .invert_out = false
        }
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,           
        .flags = {
            .with_dma = false, 
        }
    };

    
    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Не вдалося ініціалізувати LED: %s", esp_err_to_name(ret));
    }

    return led_strip;
}