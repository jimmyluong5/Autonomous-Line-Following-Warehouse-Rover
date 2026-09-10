#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdbool.h>
#include <stdint.h>

void init_speaker(void);
void speaker_update(uint8_t button_packet);
void speaker_on(void);
void speaker_off(void);
void speaker_pattern(int beep_count, uint32_t beep_on, uint32_t beep_off);
#endif