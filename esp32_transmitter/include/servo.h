#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>
#include <stdint.h>

// Master Servo Mode Switch: Set to 1 for ON, 0 for OFF (completely disables servo PWM & tasks)
#define ENABLE_SERVO_MODE   0

// Physical hardware limit constraints for suspension system arm
#define SERVO_MIN_ANGLE     34   // Min angle limit (deg)
#define SERVO_MAX_ANGLE     145  // Max angle limit (deg)
#define SERVO_CENTER_ANGLE  90   // Center resting angle (deg)

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
