#ifndef TRANSMIT_DATA_H
#define TRANSMIT_DATA_H

#include <stdint.h>
#include "esp_now.h"

#define MANUAL_MODE 0
#define AUTO_MODE 1
#define IMU_MODE 2
#define PHONE_MODE 3

typedef struct {
    uint8_t button_data;
    uint8_t speed;
    //uint16_t sequence;
    uint16_t joystick_x;
    uint16_t joystick_y;
    uint8_t mode; //eventually it'll contain more modes.
} data_packet_t;

void init_button_pin(void);
uint8_t read_buttons(void);
esp_err_t transmit_data(const uint8_t *receiver_mac, const data_packet_t *packet);
uint8_t update_speed(data_packet_t *packet);
uint8_t update_mode(data_packet_t *packet);
#endif /* TRANSMIT_DATA_H */