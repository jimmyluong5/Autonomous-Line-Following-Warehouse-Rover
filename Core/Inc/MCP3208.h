#ifndef MCP3208_H
#define MCP3208_H

#include <main.h>

#define MCP3208_CHANNEL_COUNT 8U
// there are 8 ADC channels from 0 to 7

#define MCP3208_ERROR_VALUE                                                    \
  0xFFFFU // 4095 (12 bit ADC extender) but 2^12-1 to account for 0

uint16_t MCP3208_ReadChannel(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port,
                             uint16_t cs_pin, uint8_t channel);
// this 16 bit integer reads the ADC
// channel and converts it to voltage
// to calculate voltage its
// voltage = (channel_value / 4095.0) * VREF
// VREF = 3.3V
// the channel value ranges between 0 and 4095
// if the channel is invalid, it will return MCP3208_ERROR_VALUE

// depending on the ADC channel it will return a value
#endif