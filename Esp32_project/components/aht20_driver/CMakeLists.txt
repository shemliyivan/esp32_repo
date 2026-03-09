idf_component_register(SRCS "aht20_driver.c"
                    INCLUDE_DIRS "include"
                    REQUIRES i2c_bus log)
