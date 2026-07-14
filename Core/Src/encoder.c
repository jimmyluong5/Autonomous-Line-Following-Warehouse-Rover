#include "encoder.h"
#include "main.h"

#include <stdio.h>
#include <string.h>

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef hcom_uart[];

#define ENCODER_PRINT_INTERVAL_MS 250U

void Encoder_Init(void) {
  // Start TIM3 in encoder mode for Right encoder
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  // Start TIM1 in encoder mode for Left encoder
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
}

int16_t Encoder_GetRightCount(void) {
  return (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
}

int16_t Encoder_GetLeftCount(void) {
  return (int16_t)__HAL_TIM_GET_COUNTER(&htim1);
}

void Encoder_ResetRight(void) {
  __HAL_TIM_SET_COUNTER(&htim3, 0U);
}

void Encoder_ResetLeft(void) {
  __HAL_TIM_SET_COUNTER(&htim1, 0U);
}

void Encoder_Update(void) {
  static uint32_t prev_time = 0U;
  static int16_t prev_left_count = 0;
  static int16_t prev_right_count = 0;
  uint32_t curr_time = HAL_GetTick();

  if ((curr_time - prev_time) >= ENCODER_PRINT_INTERVAL_MS) {
    prev_time = curr_time;

    int16_t right_count = Encoder_GetRightCount();
    int16_t left_count = Encoder_GetLeftCount();

    // Only print if the counts have changed to prevent terminal spam when stopped
    if (left_count != prev_left_count || right_count != prev_right_count) {
      prev_left_count = left_count;
      prev_right_count = right_count;

      char message[64];
      snprintf(message, sizeof(message), "Encoders: L=%d, R=%d\r\n", left_count, right_count);

      HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message),
                        HAL_MAX_DELAY);
    }
  }
}
