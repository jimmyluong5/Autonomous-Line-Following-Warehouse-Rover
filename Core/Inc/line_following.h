#ifndef LINE_FOLLOWING_H
#define LINE_FOLLOWING_H

#include <stdint.h>

uint16_t Robot_Normalize_ADC(uint16_t raw_val, uint16_t max_val, uint16_t min_val);
void Robot_Read_Normalized_Sensors(uint16_t *normalized_values);

#endif