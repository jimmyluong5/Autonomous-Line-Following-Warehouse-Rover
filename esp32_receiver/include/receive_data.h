#ifndef RECEIVE_DATA_H 
#define RECEIVE_DATA_H 

#include <stdint.h>

void receive_button_press(uint8_t data);
void init_pins(void);
void update_speed(uint8_t data);
#endif