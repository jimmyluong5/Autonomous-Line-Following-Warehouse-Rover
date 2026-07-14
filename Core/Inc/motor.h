#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>

void Motor_Init(void);
void Motor_Left_SetSpeed(int16_t speed);
void Motor_Right_SetSpeed(int16_t speed);
void Motor_Forward(int16_t speed);
void Motor_Reverse(int16_t speed);
void Motor_Stop(void);
void Motor_Brake(void);
void Motor_SetStandby(bool standby);

#endif /* MOTOR_H */
