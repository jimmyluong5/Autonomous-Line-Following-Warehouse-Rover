#ifndef TRANSMIT_DATA_H
#define TRANSMIT_DATA_H

#include <stdint.h>
#include "esp_now.h"

typedef struct {
    uint8_t button_data;
    uint8_t speed;
    //uint16_t sequence;
    uint16_t joystick_x;
    uint16_t joystick_y;
} data_packet_t;

void init_button_pin(void);
uint8_t read_buttons(void);
esp_err_t transmit_data(const uint8_t *receiver_mac, const data_packet_t *packet);
void init_led_pin(void);
#endif /* TRANSMIT_DATA_H */