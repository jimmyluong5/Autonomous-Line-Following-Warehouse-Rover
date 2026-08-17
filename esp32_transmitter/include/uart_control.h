#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include <stdio.h>

typedef enum {
    UART_MODE_MENU,
    UART_MODE_SERVO
} UART_ControlMode;

void UART_CONTROL_init(void);
void UART_CONTROL_update(void);
UART_ControlMode UART_CONTROL_GetMode(void);

#endif // UART_CONTROL_H
