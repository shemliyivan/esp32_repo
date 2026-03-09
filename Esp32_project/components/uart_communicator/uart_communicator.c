#include "uart_communicator.h"
#include "esp_log.h"
#include "led.h"
#include "led_types.h"
#include <string.h>
#include <stdlib.h>

#include "iot_servo.h"
#include "stepper.h"

static const char *TAG = "UART_HANDLER";

extern led_strip_handle_t strip;
extern led_mode_t g_led_mode;

static void uart_rx_task(void *arg) {
    uint8_t* data = (uint8_t*) malloc(UART_BUF_SIZE);
    while (1) {
        int len = uart_read_bytes(UART_PORT_NUM, data, UART_BUF_SIZE - 1, 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0';
            char *cmd = (char *)data;
            ESP_LOGI(TAG, "Received command: %s", cmd);

            if (strstr(cmd, "LED_ON")) {
                g_led_mode = LED_MODE_MQTT_MANUAL;
                led_strip_set_pixel(strip, 0, 255, 255, 255); 
                led_strip_refresh(strip);
                ESP_LOGI(TAG, "LED turned ON via UART");
            } 
            else if (strstr(cmd, "LED_OFF")) {
                led_strip_clear(strip);
                led_strip_refresh(strip);
                ESP_LOGI(TAG, "LED turned OFF via UART");
            }

            else if (strstr(cmd, "LED:")) {
                int r, g, b;
                if (sscanf(cmd, "LED:%d,%d,%d", &r, &g, &b) == 3) {
                    if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                        g_led_mode = LED_MODE_MQTT_MANUAL;
                        led_strip_set_pixel(strip, 0, r, g, b);
                        led_strip_refresh(strip);
                        ESP_LOGI(TAG, "LED Color set to R:%d G:%d B:%d via UART", r, g, b);
                    } else {
                        ESP_LOGW(TAG, "Invalid RGB values! Must be 0 to 255");
                    }
                } else {
                    ESP_LOGW(TAG, "Wrong format. Use LED:R,G,B (e.g., LED:255,128,0)");
                }
            }

            else if (strstr(cmd, "SYNC:")) {
                float angle = 0;
                if (sscanf(cmd, "SYNC:%f", &angle) == 1) {
                    if (angle >= 0 && angle <= 180) {
                        ESP_LOGI(TAG, "Starting sync movement to %.1f degrees", angle);
                        
                        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, angle);
                        move_stepper_degrees(angle);
                        
                        ESP_LOGI(TAG, "Sync movement finished");
                    } else {
                        ESP_LOGW(TAG, "Angle %.1f out of range!", angle);
                    }
                }
            }   
        }
    }
    free(data);
    vTaskDelete(NULL);
}

void uart_handler_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, -1, -1));

    xTaskCreate(uart_rx_task, "uart_rx_task", 4096, NULL, 10, NULL);
    
    ESP_LOGI(TAG, "UART initialized on pins TX:%d, RX:%d", UART_TX_PIN, UART_RX_PIN);
}

void uart_send_telemetry(const char* data) {
    if (data == NULL) return;
    uart_write_bytes(UART_PORT_NUM, data, strlen(data));
    uart_write_bytes(UART_PORT_NUM, "\n", 1);
}