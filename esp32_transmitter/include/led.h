#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>

void init_led(void);
void blink_led(void);
void set_led(bool on);

#endif
