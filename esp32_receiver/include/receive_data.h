#ifndef RECEIVE_DATA_H 
#define RECEIVE_DATA_H 

#include <stdint.h>

typedef struct {
    uint8_t button_data;
    uint8_t speed;
    //uint16_t sequence;
    uint16_t joystick_x;
    uint16_t joystick_y;
} data_packet_t;



void receive_button_press(data_packet_t* packet);
void init_pins(void);
//void update_speed(data_packet_t* packet); not needed because we not updating speed at all 
//or calculating the speed of the car because the data packet sends us information.
#endif