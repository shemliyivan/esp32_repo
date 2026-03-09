#ifndef LSM6DS3_H
#define LSM6DS3_H

void lsm6ds3_init(void);
void lsm6ds3_read_accel(float *x, float *y, float *z);

#endif