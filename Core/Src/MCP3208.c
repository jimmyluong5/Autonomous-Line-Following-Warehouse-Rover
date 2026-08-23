#include <MCP3208.h>
#include <main.h>

#define MCP3208_ERROR_VALUE 0xFFFFU
#define MCP3208_CHANNEL_COUNT 8U

// Reads one MCP3208 analog input in single-ended mode.
//
// Input
// channel: 0 through 7
//
// Return value
// 0 through 4095 means a valid 12-bit ADC reading.
// MCP3208_ERROR_VALUE means invalid channel or SPI communication failed.
uint16_t MCP3208_ReadChannel(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port,
                             uint16_t cs_pin, uint8_t channel) {
  uint8_t tx_data[3] = {0U};
  uint8_t rx_data[3] = {0U};

  if ((hspi == NULL) || (cs_port == NULL)) {
    return MCP3208_ERROR_VALUE;
  }

  // Reject invalid MCP3208 channels.
  if (channel >= MCP3208_CHANNEL_COUNT) {
    return MCP3208_ERROR_VALUE;
  }

  // Build the SPI command.
  // MCP3208 protocol requires: Start Bit (1), SGL/DIFF (1), D2, D1, D0
  //
  // Aligning the Start Bit to Bit 7 of the first byte allows us to fit
  // all 12 bits of the ADC response within 24 clock cycles (3 bytes).
  uint8_t d2 = (channel & 0x04U) >> 2U;
  uint8_t d1 = (channel & 0x02U) >> 1U;
  uint8_t d0 = (channel & 0x01U);

  tx_data[0] = (1U << 7U) | (1U << 6U) | (d2 << 5U) | (d1 << 4U) | (d0 << 3U);
  tx_data[1] = 0x00U;
  tx_data[2] = 0x00U;

  // Pull chip select low to activate the MCP3208.
  HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);

  // Transmit 3 bytes and receive 3 bytes synchronously.
  HAL_StatusTypeDef spi_status =
      HAL_SPI_TransmitReceive(hspi, tx_data, rx_data, 3U, 100U);

  // Pull chip select high to end the transaction.
  HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);

  // Check if communication succeeded
  if (spi_status != HAL_OK) {
    return MCP3208_ERROR_VALUE;
  }

  // Reconstruct the 12-bit ADC result:
  // With the Start bit at Bit 7 of Byte 0, the response bits align as:
  // rx_data[1] contains: B11 B10 B9 B8 B7 B6 B5 B4
  // rx_data[2] contains: B3  B2  B1 B0  x  x  x  x  (where x is trailing garbage/zeros)
  uint16_t adc_value = ((uint16_t)rx_data[1] << 4U) | (rx_data[2] >> 4U);

  return adc_value;
}