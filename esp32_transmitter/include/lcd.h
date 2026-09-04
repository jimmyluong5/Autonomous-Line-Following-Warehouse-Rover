#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"

void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd, bool keep_cs_active);
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len);
void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
uint32_t lcd_get_id(spi_device_handle_t spi);
void init_lcd(spi_device_handle_t spi);
int xpt2046_get_touch(spi_device_handle_t spi, uint16_t *x, uint16_t *y);
void init_lcd_driver(void);
#endif