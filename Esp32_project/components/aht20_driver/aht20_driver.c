#include "aht20_driver.h"
#include "i2c_bus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static i2c_master_dev_handle_t aht20_dev;
const char* TAG = "AHT20";

void aht20_init(void) {
    i2c_device_config_t dev_cfg = { .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x38, .scl_speed_hz = 100000 };
    i2c_master_bus_add_device(global_i2c_bus_handle, &dev_cfg, &aht20_dev);
    ESP_LOGI(TAG, "Aht20 was initialized\n");
}

void aht20_read(float *temp, float *hum) {
    uint8_t trigger_cmd[] = {0xAC, 0x33, 0x00};
    i2c_master_transmit(aht20_dev, trigger_cmd, 3, -1);
    vTaskDelay(pdMS_TO_TICKS(80)); 

    uint8_t data[6];
    i2c_master_receive(aht20_dev, data, 6, -1);

    uint32_t raw_hum = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t raw_temp = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *hum = ((float)raw_hum / 1048576.0) * 100.0;
    *temp = ((float)raw_temp / 1048576.0) * 200.0 - 50.0;
}