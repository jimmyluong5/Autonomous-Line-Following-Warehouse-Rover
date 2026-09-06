#ifndef SETUP_H
#define SETUP_H

#include "esp_now.h"
extern uint8_t receiver_mac[6];
void init_esp_nvs(void);
void init_wifi(void);
void init_esp_now(void);
#endif

