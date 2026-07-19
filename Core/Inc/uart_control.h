#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include <stdio.h>


//everytime you want to add a new mode, just add it here.
typedef enum {
  UART_MODE_MENU,
  UART_MODE_MOTOR,
  UART_MODE_VOLTAGE,
  UART_MODE_BOTH,
  UART_MODE_NORMALIZE,
  UART_MODE_AUTO
} UART_ControlMode;

void UART_CONTROL_init(void);
void UART_CONTROL_update(void);
void UART_CONTROL_check_timeout(void);
UART_ControlMode UART_CONTROL_GetMode(void);

#endif
