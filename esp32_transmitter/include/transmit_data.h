#ifndef TRANSMIT_DATA_H
#define TRANSMIT_DATA_H

#include "esp_now.h"

void init_button_pin(void);
uint8_t read_buttons(void);
esp_err_t transmit_data(uint8_t *receiver_mac,uint8_t data);
void init_led_pin(void);

#endif /* TRANSMIT_DATA_H */