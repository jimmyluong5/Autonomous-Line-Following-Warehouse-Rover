#include "MCP3208.h"
#include <line_following.h>
#include <main.h>
#include <stdio.h>
#include <stdlib.h>

extern SPI_HandleTypeDef hspi1; // SPI1 for the MCP3208.

// defining the sensor max and min.
uint16_t sensor_min[8] = {0};
uint16_t sensor_max[8] = {4095};

uint16_t Robot_Normalize_ADC(uint16_t raw_val, uint16_t max_val, uint16_t min_val) {
  // clamp the inputs to the calibrated min/max range
  if (raw_val <= min_val) {
    return 0;
  }

  if (raw_val >= max_val) {
    return 1000;
  }

  // Calculate normalized value scaled to 0-1000
  return (uint16_t)(((uint32_t)(raw_val - min_val) * 1000U) /
                    (max_val - min_val));
}

void Robot_Read_Normalized_Sensors(uint16_t *normalized_values) {
  for (uint8_t ch = 0; ch < 8; ch++) {
    uint16_t raw = MCP3208_ReadChannel(&hspi1, ADC_CS_GPIO_Port, ADC_CS_Pin, ch);
    if (raw == MCP3208_ERROR_VALUE) {
      normalized_values[ch] = 0;
    } else {
      normalized_values[ch] = Robot_Normalize_ADC(raw, sensor_max[ch], sensor_min[ch]);
    }
  }
}
