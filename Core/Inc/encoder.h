#ifndef ENCODER_H
#define ENCODER_H

#include "stm32g4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * =========================================================================
 * DETERMINING COUNTS PER WHEEL REVOLUTION (CPR)
 * =========================================================================
 * To calculate counts per wheel revolution (cpr), you must multiply:
 * 
 * 1. Encoder Pulses Per Revolution (PPR): The number of raw electrical cycles
 *    produced on a single channel per motor shaft revolution.
 * 2. Gearbox Ratio: The mechanical reduction ratio between the motor shaft
 *    and the final wheel axle (e.g., 30 for a 30:1 gearbox).
 * 3. Quadrature Multiplier: 
 *    - 1x mode: Counter counts rising edges of Channel A only (x1).
 *    - 2x mode: Counter counts rising/falling edges of Channel A (x2).
 *    - 4x mode: Counter counts rising/falling of BOTH Channel A and B (x4).
 * 
 * Formula:
 *    CPR = Motor PPR * Gearbox Ratio * Quadrature Multiplier
 * 
 * Note: If the datasheet states "CPR" after quadrature decoding, it already
 * includes the x4 multiplier.
 * 
 * For example:
 *    - Motor PPR = 12
 *    - Gearbox Ratio = 30
 *    - Quadrature mode = Encoder Mode TI1 and TI2 (4x decoding)
 *    CPR = 12 * 30 * 4 = 1440 counts per wheel revolution.
 */
#define ENCODER_CPR         360.0f   // Default counts per wheel revolution (customizable)
#define WHEEL_DIAMETER_M    0.0286f  // Wheel diameter in meters (28.6 mm)

// Configuration direction multipliers for left/right encoders (+1 or -1)
#define ENCODER_LEFT_POLARITY   -1
#define ENCODER_RIGHT_POLARITY  -1

typedef struct {
  TIM_HandleTypeDef *htim;         // Pointer to STM32 HAL Timer handle
  int16_t direction_multiplier;    // +1 or -1 to correct forward/reverse counting
  int32_t total_position;          // Accumulated 32-bit total count (handled with overflow correction)
  uint16_t last_timer_value;       // Previous 16-bit hardware timer count value
  int16_t last_delta;              // Direction-corrected count change since last update
  uint32_t last_update_time_ms;    // Timestamp (HAL_GetTick()) of the last update
  float speed_counts_per_sec;      // Calculated speed in counts/second
  float speed_rev_per_sec;         // Calculated speed in revolutions/second
  float speed_meters_per_sec;      // Calculated speed in meters/second
  bool is_initialized;             // Initialized flag
} Encoder_HandleTypeDef;

// Global structs for Left/Right encoders
extern Encoder_HandleTypeDef left_encoder;
extern Encoder_HandleTypeDef right_encoder;

// --- Object-Oriented Reusable Driver API ---
void Encoder_InitDevice(Encoder_HandleTypeDef *enc, TIM_HandleTypeDef *htim, int16_t direction_multiplier);
void Encoder_ResetDevice(Encoder_HandleTypeDef *enc);
void Encoder_UpdateDevice(Encoder_HandleTypeDef *enc, uint32_t current_time_ms, float cpr, float wheel_diameter_m);

// --- Simplified Wrapper API for Left/Right Encoders ---
void Encoder_Init(void);
void Encoder_Update(void);
void Encoder_ResetLeft(void);
void Encoder_ResetRight(void);

int32_t Encoder_GetLeftTotal(void);
int32_t Encoder_GetRightTotal(void);
int16_t Encoder_GetLeftDelta(void);
int16_t Encoder_GetRightDelta(void);

int16_t Encoder_GetLeftCount(void);   // Backward compatible 16-bit total cast
int16_t Encoder_GetRightCount(void);  // Backward compatible 16-bit total cast

float Encoder_GetRobotSpeed(void);

#endif /* ENCODER_H */
