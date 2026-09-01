#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include <stdio.h>


//everytime you want to add a new mode, just add it here.
typedef enum {
  UART_MODE_MENU,
  UART_MODE_MOTOR,
  UART_MODE_COMBINED,
  UART_MODE_VOLTAGE,
  UART_MODE_BOTH,
  UART_MODE_NORMALIZE,
  UART_MODE_AUTO,
  UART_MODE_SERVO,
  UART_MODE_STEPPER,
  UART_MODE_SPEAKER,
  UART_MODE_IMU
} UART_ControlMode;

void UART_CONTROL_init(void);
void UART_CONTROL_update(void);
void UART_CONTROL_check_timeout(void);
UART_ControlMode UART_CONTROL_GetMode(void);
void menu_imu(void);
void menu_main(void);
void menu_motor(void);
void menu_combined(void);
void menu_speaker(void);
void menu_stepper(void);
void menu_normalized(void);
void menu_autonomous(void);
void menu_servo(void);
void menu_voltage(void);
void menu_both(void);





#endif
