#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>
#include <stdbool.h>

//define stepper motor parameters

#define stepper_gear_ratio 100.0f
#define base_step_angle 18.0f
#define output_step_angle (base_step_angle / stepper_gear_ratio) //0.18 degrees.
#define step_per_revolution (360 / output_step_angle) //2000 steps per rev.



//initalize stepper motor, going to enable the stepper motor here.
void stepper_init(void);

//function to set the direction.
void stepper_set_dir(void);

//function to pulse the STEP pin once, its PWM
void stepper_step(void);

//function to move a specific number of steps (pos = CW, neg = CCW) at a given speed
//steps is an int32_t because it can be negative for CCW. 
//speed is a float because it can be a fraction. 
void stepper_move_steps(int32_t steps, uint32_t speed);

//function to rotate the output shaft
void stepper_move_degrees(float degrees, uint32_t speed);

//func to stop the motor
void stepper_stop(void);

#endif
