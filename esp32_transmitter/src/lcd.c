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


//the lcd needs a bunch of command/argument values to be initialized. they stored in this struct/.

typedef struct {
    uint8_t cmd;        // The command byte sent to the LCD controller (e.g., 0x11 for Sleep Out)
    uint8_t data[16];   // Array of argument/parameter bytes that accompany the command (up to 16 bytes)
    uint8_t databytes;  // Multi-purpose control byte:
                        // - Lower bits (0-6): Number of valid bytes being used in the 'data' array
                        // - Bit 7 (MSB): If set, tells the driver to pause/delay after sending this command
                        // - 0xFF: Sentinel value used to signal the absolute end of the command list
} lcd_init_cmd_t;
// cmd is the byte that tells the lcd what to do
// for example: 0x29 means turn the screen on, or 0x11 means exit sleep mode.

// data[16] is a 16-byte array where each index is 1 byte large.
// It acts like a workspace to hold extra settings/parameters for the command.
// Reads the command, then checks how many bytes are needed.
// Grabs the settings from the active bytes in data[16] and sends them right after cmd.
// Checks if Bit 7 of the 'databytes' variable is flipped (HIGH); if it is, 
// the code pauses for a brief delay.
// A 'databytes' value of 0xFF is the signal to end the entire command list.

//databytes 
//the first 6 bits tells you the length or number of data bytes/arguments that need to be sent
//which is located in data[16], for example: how many of those 16 slots do i need to actually send?
//if databytes = 0x00, then I don't need to send any extra settings after command.
//if databytes = 0xFF, or 1111_1111, then we are at the end of the commmand list so stop updating the screen.
//if databytes = 0x03, it tells the program to read the first 3 items of the data array
//data[0], data[1], data[2] and send them right after the command. 

//if the 7th bit in databytes is set, then the code pauses for a brief moment then sends the command.
//this is so that the display/screen has a break to catch up with the instructions
//if you MCU sends the instructions too fast, the screen may not catch all the instructions.

