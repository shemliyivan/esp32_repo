#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/uart.h"

#include "host/ble_hs.h"
#include "host/ble_gatt.h"

#include "led.h"
#include "joystick.h"
#include "my_wifi.h"
#include "mqtt_component.h"
#include "led_types.h"
#include "bluetooth_component.h"
#include "uart_communicator.h"
#include "i2c_bus.h"
#include "spi_bus.h"
#include "aht20_driver.h"
#include "bmp280_driver.h"
#include "lsm6ds3_driver.h"

static const char *TAG = "MAIN";

uint16_t sensor_char_handle;
char ble_telemetry_data[256] = {0};

#define UART_TELEMETRY "UART_TELEMETRY"
#define MQTT_TELEMETRY "MQTT_TELEMETRY"
#define BLE_TELEMETRY  "BLE_TELEMETRY"

led_strip_handle_t strip;

// Для того, щоб таска led_logic_task не переривала data receiver MQTT
led_mode_t g_led_mode = LED_MODE_WIFI_STATUS;

// Для зручної перевірки стану WIFI - та виставлення відповідного значення на Led
void led_logic_task(void *arg) {
    led_strip_handle_t strip = (led_strip_handle_t)arg;
    bool blink = false;
    
    while (1) {
        if(g_led_mode == LED_MODE_WIFI_STATUS){
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
        }
        blink = !blink;
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

void telemetry_task(void* pvParameters) {
    float t = 0, h = 0, ax = 0, ay = 0, az = 0, p = 0;

    const char* where = (char*)pvParameters;

    while (1) {
        aht20_read(&t, &h);
        lsm6ds3_read_accel(&ax, &ay, &az);
        p = bmp280_read_pressure();

        char payload[256];
        snprintf(payload, sizeof(payload), 
             "{\"T\":%.2f,\"H\":%.2f,\"P\":%.1f,\"A\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f}}", 
             t, h, p, ax, ay, az);

        if(strcmp(where, UART_TELEMETRY) == 0){
            uart_send_telemetry(payload);
        }
        else if(strcmp(where, MQTT_TELEMETRY) == 0){
            if (g_mqtt_connected && g_mqtt_client != NULL) {
                int msg_id = esp_mqtt_client_publish(g_mqtt_client, "esp-lection/status", payload, 0, 1, 0);
                ESP_LOGI(TAG, "Telemetry sent to broker, msg_id=%d", msg_id);
            } else {
                ESP_LOGW(TAG, "MQTT not connected, skipping telemetry data");
            }
        }
        else{
            strncpy(ble_telemetry_data, payload, sizeof(ble_telemetry_data));
            ble_gatts_chr_updated(sensor_char_handle);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    TaskHandle_t taskTelemertyHandler = NULL;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    strip = configure_rgb_led();
    configure_joystick();
    wifi_init_global();

    xTaskCreate(led_logic_task, "LED_Logic", 2048, (void*)strip, 5, NULL);

    ESP_LOGI(TAG, "System Ready. Joystick: Left=AP, Right=STA");

    int x, y, mode = 0; // 0=None, 1=AP, 2=STA
    bool btn;

    // Ініціалізація I2C, SPI
    // Ініціалізація датчиків AHT20, BMP280, LSM6DS3
    // Та запуск таски на телеметрію
    i2c_bus_init();
    spi_bus_init();

    aht20_init();
    bmp280_init();
    lsm6ds3_init();

    while (1) {
        read_joystick(&x, &y, &btn);

        if (x < 1000 && mode != 1) {
            if (mode == 3) bluetooth_component_deinit();
            if(taskTelemertyHandler != NULL){
                vTaskDelete(taskTelemertyHandler);
                taskTelemertyHandler = NULL;
            }
            ESP_LOGI(TAG, "Switching to AP Mode");
            wifi_start_ap();
            mode = 1;
            vTaskDelay(pdMS_TO_TICKS(1000)); 
        }

        else if (x > 3000 && mode != 2) {
            if (mode == 3) bluetooth_component_deinit();
            if(taskTelemertyHandler != NULL){
                vTaskDelete(taskTelemertyHandler);
                taskTelemertyHandler = NULL;
            }
            ESP_LOGI(TAG, "Switching to STA Mode");
            wifi_start_sta();
            mode = 2;
            g_led_mode = LED_MODE_WIFI_STATUS;
            mqtt_component_start();
            xTaskCreate(telemetry_task, "Telemetry task", 4096, (void*)MQTT_TELEMETRY, 5, &taskTelemertyHandler);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        else if(y > 3000 && mode != 3){
            if(taskTelemertyHandler != NULL){
                    vTaskDelete(taskTelemertyHandler);
                    taskTelemertyHandler = NULL;
                }
                ESP_LOGI(TAG, "Switching to bluetooth mode");
                bluetooth_component_init();
                mode = 3;
                xTaskCreate(telemetry_task, "Telemetry task", 4096, (void*)BLE_TELEMETRY, 5, &taskTelemertyHandler);
                vTaskDelay(pdMS_TO_TICKS(2000));
            }

            else if(y < 1000 && mode != 4){
                if (mode == 3) bluetooth_component_deinit();
                if(taskTelemertyHandler != NULL){
                    vTaskDelete(taskTelemertyHandler);
                    taskTelemertyHandler = NULL;
                }
                ESP_LOGI(TAG, "Switching to UART");
                uart_handler_init();
                mode = 4;
                g_led_mode = LED_MODE_MQTT_MANUAL;
                xTaskCreate(telemetry_task, "Telemetry task", 4096, (void*)UART_TELEMETRY, 5, &taskTelemertyHandler);
            }

            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }