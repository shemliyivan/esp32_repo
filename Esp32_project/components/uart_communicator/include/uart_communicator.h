#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include "driver/uart.h"

#define UART_PORT_NUM      UART_NUM_1
#define UART_TX_PIN        17
#define UART_RX_PIN        18
#define UART_BUF_SIZE      1024

void uart_handler_init(void);
void uart_send_telemetry(const char* data);

#endif 