#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void UART_CONTROL_init(void);
void UART_CONTROL_update(void);
void UART_SendMessage(const char *message);

#endif // UART_CONTROL_H
