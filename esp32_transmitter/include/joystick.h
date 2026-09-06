#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "adc.h"
#include <stdint.h>
#include <stdbool.h>

void init_joystick(void);
uint16_t read_joystick_horizontal(void);
uint16_t read_joystick_vertical(void);
void print_joystick_values(void);
void get_joystick_screen_coords(int *out_x, int *out_y);

// Bounding box of the joystick grid on blankfirstpage.jpg
#define GRID_CENTER_X   63
#define GRID_CENTER_Y   78
#define GRID_RADIUS_X   40
#define GRID_RADIUS_Y   38

#endif