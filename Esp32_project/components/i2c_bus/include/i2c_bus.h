#ifndef I2C_BUS_H
#define I2C_BUS_H
#include "driver/i2c_master.h"

extern i2c_master_bus_handle_t global_i2c_bus_handle;
void i2c_bus_init(void);
#endif