#include "encoder.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

// Set to 1 to enable hardware timers, or 0 to run encoders in stub mode
#define ENCODERS_ENABLED 1

#if ENCODERS_ENABLED
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
#endif

// Global structures holding state for Left and Right encoders
Encoder_HandleTypeDef left_encoder = {0};
Encoder_HandleTypeDef right_encoder = {0};

/**
 * @brief Initialize a single encoder handle
 * @param enc Pointer to the encoder handle structure
 * @param htim Pointer to the TIM peripheral handle
 * @param direction_multiplier Inversion factor (+1 or -1)
 */
void Encoder_InitDevice(Encoder_HandleTypeDef *enc, TIM_HandleTypeDef *htim, int16_t direction_multiplier) {
  if (enc == NULL) {
    return;
  }
#if ENCODERS_ENABLED
  enc->htim = htim;
  enc->last_timer_value = (uint16_t)__HAL_TIM_GET_COUNTER(htim);
  HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
#else
  enc->htim = NULL;
  enc->last_timer_value = 0;
#endif
  enc->direction_multiplier = (direction_multiplier < 0) ? -1 : 1;
  enc->total_position = 0;
  enc->last_delta = 0;
  enc->last_update_time_ms = HAL_GetTick();
  enc->speed_counts_per_sec = 0.0f;
  enc->speed_rev_per_sec = 0.0f;
  enc->speed_meters_per_sec = 0.0f;
  enc->is_initialized = true;
}

/**
 * @brief Reset the accumulated position and current speed values for an encoder
 * @param enc Pointer to the encoder handle structure
 */
void Encoder_ResetDevice(Encoder_HandleTypeDef *enc) {
  if (enc == NULL || !enc->is_initialized) {
    return;
  }
  enc->total_position = 0;
  enc->last_delta = 0;
#if ENCODERS_ENABLED
  enc->last_timer_value = (uint16_t)__HAL_TIM_GET_COUNTER(enc->htim);
#else
  enc->last_timer_value = 0;
#endif
  enc->last_update_time_ms = HAL_GetTick();
  enc->speed_counts_per_sec = 0.0f;
  enc->speed_rev_per_sec = 0.0f;
  enc->speed_meters_per_sec = 0.0f;
}

/**
 * @brief Update the encoder accumulated position and calculate its current speed
 * @param enc Pointer to the encoder handle structure
 * @param current_time_ms Current system timestamp in milliseconds
 * @param cpr Counts per wheel revolution
 * @param wheel_diameter_m Wheel diameter in meters
 */
void Encoder_UpdateDevice(Encoder_HandleTypeDef *enc, uint32_t current_time_ms, float cpr, float wheel_diameter_m) {
  if (enc == NULL || !enc->is_initialized) {
    return;
  }

#if ENCODERS_ENABLED
  // 1. Read current 16-bit hardware timer counter
  uint16_t current_timer = (uint16_t)__HAL_TIM_GET_COUNTER(enc->htim);

  // 2. Correctly calculate the count difference even when the hardware timer wraps around.
  int16_t raw_diff = (int16_t)(current_timer - enc->last_timer_value);

  // 3. Apply configurable direction multiplier (+1 or -1)
  int16_t delta = raw_diff * enc->direction_multiplier;

  // 4. Save the delta change since the previous update
  enc->last_delta = delta;

  // 5. Accumulate the differences into a signed 32-bit total position
  enc->total_position += delta;

  // 6. Update last timer value
  enc->last_timer_value = current_timer;

  // 7. Calculate speed using real elapsed time from HAL_GetTick()
  uint32_t elapsed_ms = current_time_ms - enc->last_update_time_ms;
  enc->last_update_time_ms = current_time_ms;

  if (elapsed_ms > 0) {
    float dt = (float)elapsed_ms / 1000.0f; // Convert ms to seconds

    // 8. Calculate speed in counts per second
    enc->speed_counts_per_sec = (float)delta / dt;

    // 9. Calculate speed in revolutions per second
    if (cpr > 0.0f) {
      enc->speed_rev_per_sec = enc->speed_counts_per_sec / cpr;
    } else {
      enc->speed_rev_per_sec = 0.0f;
    }

    // 10. Calculate speed in meters per second (Linear speed = RPS * Circumference)
    float circumference = 3.14159265f * wheel_diameter_m;
    enc->speed_meters_per_sec = enc->speed_rev_per_sec * circumference;
  }
#else
  // Hardware disabled, do nothing
  enc->last_delta = 0;
#endif
}

// =========================================================================
// Simplified Wrapper API Implementations (Left/Right instances)
// =========================================================================

void Encoder_Init(void) {
#if ENCODERS_ENABLED
  // PA11 & PA12 (TIM4) are wired to the LEFT motor
  Encoder_InitDevice(&left_encoder, &htim4, ENCODER_LEFT_POLARITY);
  // PA8 & PA9 (TIM1) are wired to the RIGHT motor
  Encoder_InitDevice(&right_encoder, &htim1, ENCODER_RIGHT_POLARITY);
#else
  Encoder_InitDevice(&left_encoder, NULL, ENCODER_LEFT_POLARITY);
  Encoder_InitDevice(&right_encoder, NULL, ENCODER_RIGHT_POLARITY);
#endif
}

void Encoder_ResetLeft(void) {
  Encoder_ResetDevice(&left_encoder);
}

void Encoder_ResetRight(void) {
  Encoder_ResetDevice(&right_encoder);
}

void Encoder_Update(void) {
  uint32_t now = HAL_GetTick();
  Encoder_UpdateDevice(&left_encoder, now, ENCODER_CPR, WHEEL_DIAMETER_M);
  Encoder_UpdateDevice(&right_encoder, now, ENCODER_CPR, WHEEL_DIAMETER_M);
}

int32_t Encoder_GetLeftTotal(void) {
  return left_encoder.total_position;
}

int32_t Encoder_GetRightTotal(void) {
  return right_encoder.total_position;
}

int16_t Encoder_GetLeftDelta(void) {
  return left_encoder.last_delta;
}

int16_t Encoder_GetRightDelta(void) {
  return right_encoder.last_delta;
}

int16_t Encoder_GetLeftCount(void) {
  return (int16_t)left_encoder.total_position;
}

int16_t Encoder_GetRightCount(void) {
  return (int16_t)right_encoder.total_position;
}

float Encoder_GetRobotSpeed(void) {
  return (left_encoder.speed_meters_per_sec + right_encoder.speed_meters_per_sec) / 2.0f;
}
