#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "adc.h"
#include <stdint.h>
#include <stdbool.h>

void init_joystick(void);
uint16_t read_joystick_horizontal(void);
uint16_t read_joystick_vertical(void);
void print_joystick_values(void);


#endif