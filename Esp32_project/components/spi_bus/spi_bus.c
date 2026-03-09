#include "spi_bus.h"
#include "esp_log.h"

#define GPIO_NUM_15_MISO 15
#define GPIO_NUM_17_MOSI 17
#define GPIO_NUM_18_SCL 18

#define SPI_CLOCK_HZ 1000000

static const char *TAG = "SPI_BUS";

void spi_bus_init(void) {
    spi_bus_config_t buscfg = {
        .miso_io_num = GPIO_NUM_15_MISO,
        .mosi_io_num = GPIO_NUM_17_MOSI, 
        .sclk_io_num = GPIO_NUM_18_SCL,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    esp_err_t err = spi_bus_initialize(BUS_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "spi bus failed to initialize", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "SPI Bus initialized successfully (MOSI:%d, MISO:%d, CLK:%d)", 
             GPIO_NUM_17_MOSI, GPIO_NUM_15_MISO, GPIO_NUM_18_SCL);
}