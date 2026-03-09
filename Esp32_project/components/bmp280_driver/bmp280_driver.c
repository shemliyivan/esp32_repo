#include "bmp280_driver.h"
#include "i2c_bus.h"

static i2c_master_dev_handle_t bmp280_dev;

void bmp280_init(void) {
    i2c_device_config_t dev_cfg = { .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x76, .scl_speed_hz = 100000 };
    i2c_master_bus_add_device(global_i2c_bus_handle, &dev_cfg, &bmp280_dev);
    
    uint8_t config[] = {0xF4, 0x27}; 
    i2c_master_transmit(bmp280_dev, config, 2, -1);
}

float bmp280_read_pressure(void) {
    uint8_t reg = 0xF7;
    uint8_t data[3];
    i2c_master_transmit_receive(bmp280_dev, &reg, 1, data, 3, -1);
    
    int32_t adc_P = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    return (float)adc_P / 256.0; 
}