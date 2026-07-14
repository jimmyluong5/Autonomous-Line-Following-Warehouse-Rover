#include "robot.h"
#include "main.h"
#include "motor.h"

static RobotState current_state = robot_idle;

void Robot_Init(void) {
  current_state = robot_idle;
  Motor_Init();
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
}

void Robot_SetState(RobotState new_state) {
  current_state = new_state;
  if (current_state == robot_idle || current_state == robot_fault) {
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
  }

  // Update motor speeds according to the new robot state
  switch (current_state) {
    case robot_forward:
      Motor_Forward(500);
      break;
    case robot_reverse:
      Motor_Reverse(500);
      break;
    case robot_left:
      // Spin turn left: left motor backward, right motor forward
      Motor_Left_SetSpeed(-500);
      Motor_Right_SetSpeed(500);
      break;
    case robot_right:
      // Spin turn right: left motor forward, right motor backward
      Motor_Left_SetSpeed(500);
      Motor_Right_SetSpeed(-500);
      break;
    case robot_idle:
      Motor_Stop();
      break;
    case robot_fault:
      Motor_Brake();
      break;
    default:
      Motor_Stop();
      break;
  }
}

void Robot_Update(void) {
  // Add state update logic if needed
}

RobotState Robot_GetState(void) {
  return current_state;
}
