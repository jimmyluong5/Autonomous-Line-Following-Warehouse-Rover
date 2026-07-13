#include "robot.h"
#include "main.h"

static RobotState current_state = robot_idle;

void Robot_Init(void) {
  current_state = robot_idle;
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
}

void Robot_SetState(RobotState new_state) {
  current_state = new_state;
  if (current_state == robot_idle || current_state == robot_fault) {
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
  }
}

void Robot_Update(void) {
  // Add state update logic if needed
}

RobotState Robot_GetState(void) {
  return current_state;
}
