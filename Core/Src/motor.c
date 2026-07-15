#include "motor.h"
#include "main.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim17;

void Motor_Init(void) {
  // Start TIM17 Channel 1 PWM for right motor
  HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
  // Start TIM2 Channel 2 PWM for left motor
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  // Set speeds to 0 initially
  Motor_Left_SetSpeed(0);
  Motor_Right_SetSpeed(0);

  // Enable the motor driver by setting STBY high (active high for TB6612FNG)
  Motor_SetStandby(false); // standby = false means STBY pin = high (run mode)
}

void Motor_SetStandby(bool standby) {
  HAL_GPIO_WritePin(Motor_STBY_GPIO_Port, Motor_STBY_Pin,
                    standby ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Motor_Right_SetSpeed(int16_t speed) {
  // Constrain speed to -999 to 999
  if (speed > 999)
    speed = 999;
  if (speed < -999)
    speed = -999;

  if (speed > 0) {
    // BIN1 = High, BIN2 = Low for Forward
    HAL_GPIO_WritePin(Motor_BIN1_GPIO_Port, Motor_BIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(Motor_BIN2_GPIO_Port, Motor_BIN2_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint32_t)speed);
  } else if (speed < 0) {
    // BIN1 = Low, BIN2 = High for Reverse
    HAL_GPIO_WritePin(Motor_BIN1_GPIO_Port, Motor_BIN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Motor_BIN2_GPIO_Port, Motor_BIN2_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint32_t)(-speed));
  } else {
    // Stop: BIN1 = Low, BIN2 = Low
    HAL_GPIO_WritePin(Motor_BIN1_GPIO_Port, Motor_BIN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Motor_BIN2_GPIO_Port, Motor_BIN2_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
  }
}

void Motor_Left_SetSpeed(int16_t speed) {
  // Constrain speed to -999 to 999
  if (speed > 999)
    speed = 999;
  if (speed < -999)
    speed = -999;

  if (speed > 0) {
    // AIN1 = High, AIN2 = Low for Forward
    HAL_GPIO_WritePin(Motor_AIN1_GPIO_Port, Motor_AIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(Motor_AIN2_GPIO_Port, Motor_AIN2_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, (uint32_t)speed);
  } else if (speed < 0) {
    // AIN1 = Low, AIN2 = High for Reverse
    HAL_GPIO_WritePin(Motor_AIN1_GPIO_Port, Motor_AIN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Motor_AIN2_GPIO_Port, Motor_AIN2_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, (uint32_t)(-speed));
  } else {
    // Stop: AIN1 = Low, AIN2 = Low
    HAL_GPIO_WritePin(Motor_AIN1_GPIO_Port, Motor_AIN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Motor_AIN2_GPIO_Port, Motor_AIN2_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, 0);
  }
}

void Motor_Forward(int16_t speed) {
  // If negative speed is passed, make it positive
  if (speed > 0)
    speed = -speed;
  Motor_Left_SetSpeed(speed);
  Motor_Right_SetSpeed(speed);
}

void Motor_Reverse(int16_t speed) {
  // If positive speed is passed, make it negative
  if (speed < 0)
    speed = -speed;
  Motor_Left_SetSpeed(speed);
  Motor_Right_SetSpeed(speed);
}

void Motor_Stop(void) {
  Motor_Left_SetSpeed(0);
  Motor_Right_SetSpeed(0);
}

void Motor_Brake(void) {
  // Active Brake: set both direction pins high
  HAL_GPIO_WritePin(Motor_AIN1_GPIO_Port, Motor_AIN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(Motor_AIN2_GPIO_Port, Motor_AIN2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(Motor_BIN1_GPIO_Port, Motor_BIN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(Motor_BIN2_GPIO_Port, Motor_BIN2_Pin, GPIO_PIN_SET);

  __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, 0);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
}
