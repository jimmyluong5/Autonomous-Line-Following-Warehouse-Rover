#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "pretty_effect.h"


//this code is from the esp32 expressif github for this type of display 

/*
 This code displays some fancy graphics on the 320x240 LCD on an ESP-WROVER_KIT board.
 This example demonstrates the use of both spi_device_transmit as well as
 spi_device_queue_trans/spi_device_get_trans_result and pre-transmit callbacks.

 Some info about the ILI9341/ST7789V: It has an C/D line, which is connected to a GPIO here. It expects this
 line to be low for a command and high for data. We use a pre-transmit callback here to control that
 line: every transaction has as the user-definable argument the needed state of the D/C line and just
 before the transaction is sent, the callback will set this line to the correct state.
*/

//this is different for me

//SCK - GPIO7
//SDI/MOSI - GPIO8
//SDO/MISO - GPIO9
//CS - GPIO 15
//DC - GPIO 16
//RESET - GPIO 17

#define LCD_HOST  SPI2_HOST

#define PIN_NUM_MISO 9
#define PIN_NUM_MOSI 8
#define PIN_NUM_CLK  7 //sck
#define PIN_NUM_CS   15

#define PIN_NUM_DC   16
#define PIN_NUM_RST  17
//#define PIN_NUM_BCKL 5, already set high using a wire.

//#define LCD_BK_LIGHT_ON_LEVEL   0

#define PIN_NUM_T_IRQ 42
#define PIN_NUM_T_DO 9
#define PIN_NUM_T_DIN 8
#define PIN_NUM_T_CS 18
#define PIN_NUM_T_CLK 7

//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 16

// Number of horizontal display lines sent in each SPI transfer.
// Larger values send more pixels per transfer, which reduces SPI overhead
// and can improve drawing speed, but requires a larger memory buffer.
// Keep this as a factor of 240 so the screen height divides evenly.
//240/16 = 15 lines each SPI transfer.


