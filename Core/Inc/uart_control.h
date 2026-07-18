#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include <stdio.h>

typedef enum {
  UART_MODE_MENU,
  UART_MODE_MOTOR,
  UART_MODE_VOLTAGE,
  UART_MODE_BOTH
} UART_ControlMode;

void UART_CONTROL_init(void);
void UART_CONTROL_update(void);
void UART_CONTROL_check_timeout(void);
UART_ControlMode UART_CONTROL_GetMode(void);

#endif
