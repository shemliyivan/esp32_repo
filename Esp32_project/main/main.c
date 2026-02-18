#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "led.h"
#include "joystick.h"
#include "my_wifi.h"

static const char *TAG = "MAIN";

// Для зручної перевірки стану WIFI - та виставлення відповідного значення на Led
void led_logic_task(void *arg) {
    led_strip_handle_t strip = (led_strip_handle_t)arg;
    bool blink = false;
    
    while (1) {
        uint8_t r=0, g=0, b=0;
        
        switch (g_wifi_state) {
            case WIFI_STATE_OFF:             r=20; g=20; b=20; break;
            case WIFI_STATE_STA_CONNECTING:  r=50; g=50; b=0;  break;
            case WIFI_STATE_STA_ERROR:       r=50; g=0;  b=0;  break; 
            case WIFI_STATE_STA_GOT_IP:      if(blink) g=50;   break;
            case WIFI_STATE_INTERNET_OK:     g=50;             break;
            case WIFI_STATE_AP_STARTED:      if(blink) b=50;   break;
            case WIFI_STATE_AP_CONNECTED:    b=50;             break;
        }

        led_strip_set_pixel(strip, 0, r, g, b);
        led_strip_refresh(strip);
        
        blink = !blink;
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    led_strip_handle_t strip = configure_rgb_led();
    configure_joystick();
    wifi_init_global();

    xTaskCreate(led_logic_task, "LED_Logic", 2048, (void*)strip, 5, NULL);

    ESP_LOGI(TAG, "System Ready. Joystick: Left=AP, Right=STA");

    int x, y, mode = 0; // 0=None, 1=AP, 2=STA
    bool btn;

    while (1) {
        read_joystick(&x, &y, &btn);

        if (x < 1000 && mode != 1) {
            ESP_LOGI(TAG, "Switching to AP Mode");
            wifi_start_ap();
            mode = 1;
            vTaskDelay(pdMS_TO_TICKS(1000)); 
        }

        else if (x > 3000 && mode != 2) {
            ESP_LOGI(TAG, "Switching to STA Mode");
            wifi_start_sta();
            mode = 2;
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}