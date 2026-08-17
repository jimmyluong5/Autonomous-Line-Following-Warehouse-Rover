#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>
#include <stdint.h>

void servo_init(void);
void Servo_SetAngle(uint8_t angle);
void Servo_SetAngleImmediate(uint8_t angle);
void Servo_Update(void);
void Servo_SetSpeed(uint16_t step_interval_ms, float step_deg);
void Servo_SetAutoDetach(bool enable, uint16_t delay_ms);
void servo_stop(void);

uint8_t Servo_GetTargetAngle(void);
uint8_t Servo_GetCurrentAngle(void);
bool Servo_IsMoving(void);

#endif // SERVO_H
