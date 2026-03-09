#include "i2c_bus.h"
#include "esp_log.h"

i2c_master_bus_handle_t global_i2c_bus_handle;

void i2c_bus_init(void) {
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = GPIO_NUM_5,
        .sda_io_num = GPIO_NUM_6,
        .glitch_ignore_cnt = 7,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &global_i2c_bus_handle));
}