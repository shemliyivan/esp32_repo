#ifndef AHT20_H
#define AHT20_H

void aht20_init(void);
void aht20_read(float *temp, float *hum);

#endif