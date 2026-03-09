#ifndef STEPPER
#define STEPPER
#include "driver/gpio.h"

void stepper_init(void);
void move_stepper_degrees(float angle);

#endif