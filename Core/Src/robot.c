#include "robot.h"

static RobotState current_state = robot_idle;

void Robot_Init(void) {
  current_state = robot_idle;
}

void Robot_SetState(RobotState new_state) {
  current_state = new_state;
}

void Robot_Update(void) {
  // Add state update logic if needed
}

RobotState Robot_GetState(void) {
  return current_state;
}
