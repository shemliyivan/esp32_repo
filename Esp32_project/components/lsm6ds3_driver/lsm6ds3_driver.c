#include "lsm6ds3_driver.h"
#include "spi_bus.h"
#include "driver/gpio.h"

#define PIN_CS_LSM 10
static spi_device_handle_t lsm6_handle;

void lsm6ds3_init(void) {
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10*1000*1000, // 10 MHz
        .mode = 0,
        .spics_io_num = PIN_CS_LSM,
        .queue_size = 7,
    };
    spi_bus_add_device(BUS_SPI_HOST, &devcfg, &lsm6_handle);

    uint8_t setup[] = {0x10, 0x40};
    spi_transaction_t t = { .length = 16, .tx_buffer = setup };
    spi_device_transmit(lsm6_handle, &t);
}

void lsm6ds3_read_accel(float *x, float *y, float *z) {
    uint8_t tx_data[7] = {0}; 
    uint8_t rx_data[7] = {0};
    
    tx_data[0] = 0x28 | 0x80; 

    spi_transaction_t t = {
        .length = 7 * 8,       
        .tx_buffer = tx_data,  
        .rx_buffer = rx_data,  
    };

    esp_err_t ret = spi_device_transmit(lsm6_handle, &t);
    
    if (ret == ESP_OK) {
        int16_t raw_x = (rx_data[2] << 8) | rx_data[1];
        int16_t raw_y = (rx_data[4] << 8) | rx_data[3];
        int16_t raw_z = (rx_data[6] << 8) | rx_data[5];

        *x = (float)raw_x * 0.061 / 1000.0;
        *y = (float)raw_y * 0.061 / 1000.0;
        *z = (float)raw_z * 0.061 / 1000.0;
    }
}