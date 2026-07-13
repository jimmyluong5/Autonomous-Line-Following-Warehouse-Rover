#ifndef ROBOT_H
#define ROBOT_H

typedef enum {
  robot_idle,
  robot_forward,
  robot_reverse,
  robot_left,
  robot_right,
  robot_fault
} RobotState;

void Robot_Init(void);
void Robot_SetState(RobotState new_state);
void Robot_Update(void);
RobotState Robot_GetState(void);
#endif

