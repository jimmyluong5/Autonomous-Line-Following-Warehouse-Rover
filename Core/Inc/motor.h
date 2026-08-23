#ifndef MOTOR_H
#define MOTOR_H

#include <stdbool.h>
#include <stdint.h>

// Default motor speed / PWM duty cycle (0 to 999)
// 0 = 0% PWM, 500 = 50% PWM, 999 = 100% PWM
#define MOTOR_DEFAULT_SPEED 999

void Motor_Init(void);
void Motor_Left_SetSpeed(int16_t speed);
void Motor_Right_SetSpeed(int16_t speed);
void Motor_Forward(int16_t speed);
void Motor_Reverse(int16_t speed);
void Motor_Stop(void);
void Motor_Brake(void);
void Motor_SetStandby(bool standby);

#endif /* MOTOR_H */
