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

typedef enum {
    LCD_TYPE_ILI = 1,  // Represents ILI display controllers (e.g., ILI9341), explicitly starts at 1
    LCD_TYPE_ST,       // Represents ST display controllers (e.g., ST7789), automatically assigned 2
    LCD_TYPE_MAX,      // Boundary/max count marker, automatically assigned 3 for error-checking
} type_lcd_t;

//TFT Controller: ILI9341
//Touch Screen Controller: XPT2046


//place data into dynamic random access memory DRAM. 
//Constant data gets placed into DROm by default, which is not accessible by direct memory address (DMA)
//DMA in a computer stands for Direct Memory Access,
//a feature that allows hardware devices to transfer data directly to and 
// from the system's main memory (RAM) without needing the computer's Central Processing Unit (CPU) 
// to handle every single piece of data. 


DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[] = {
    /* Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0 */
    {0x36, {(1 << 5) | (1 << 6)}, 1},
    /* Interface Pixel Format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Porch Setting */
    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
    /* Gate Control, Vgh=13.65V, Vgl=-10.43V */
    {0xB7, {0x45}, 1},
    /* VCOM Setting, VCOM=1.175V */
    {0xBB, {0x2B}, 1},
    /* LCM Control, XOR: BGR, MX, MH */
    {0xC0, {0x2C}, 1},
    /* VDV and VRH Command Enable, enable=1 */
    {0xC2, {0x01, 0xff}, 2},
    /* VRH Set, Vap=4.4+... */
    {0xC3, {0x11}, 1},
    /* VDV Set, VDV=0 */
    {0xC4, {0x20}, 1},
    /* Frame Rate Control, 60Hz, inversion=0 */
    {0xC6, {0x0f}, 1},
    /* Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V */
    {0xD0, {0xA4, 0xA1}, 2},
    /* Positive Voltage Gamma Control */
    {0xE0, {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19}, 14},
    /* Negative Voltage Gamma Control */
    {0xE1, {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19}, 14},
    /* Sleep Out */
    {0x11, {0}, 0x80},
    /* Display On */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff}
};

DRAM_ATTR static const lcd_init_cmd_t ili_init_cmds[] = {
    /* Power control B, power control = 0, DC_ENA = 1 */
    {0xCF, {0x00, 0x83, 0X30}, 3},
    /* Power on sequence control,
     * cp1 keeps 1 frame, 1st frame enable
     * vcl = 0, ddvdh=3, vgh=1, vgl=2
     * DDVDH_ENH=1
     */
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    /* Driver timing control A,
     * non-overlap=default +1
     * EQ=default - 1, CR=default
     * pre-charge=default - 1
     */
    {0xE8, {0x85, 0x01, 0x79}, 3},
    /* Power control A, Vcore=1.6V, DDVDH=5.6V */
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    /* Pump ratio control, DDVDH=2xVCl */
    {0xF7, {0x20}, 1},
    /* Driver timing control, all=0 unit */
    {0xEA, {0x00, 0x00}, 2},
    /* Power control 1, GVDD=4.75V */
    {0xC0, {0x26}, 1},
    /* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
    {0xC1, {0x11}, 1},
    /* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
    {0xC5, {0x35, 0x3E}, 2},
    /* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
    {0xC7, {0xBE}, 1},
    /* Memory access control, MX=MY=0, MV=1, ML=0, BGR=1, MH=0 */
    {0x36, {0x28}, 1},
    /* Pixel format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Frame rate control, f=fosc, 70Hz fps */
    {0xB1, {0x00, 0x1B}, 2},
    /* Enable 3G, disabled */
    {0xF2, {0x08}, 1},
    /* Gamma set, curve 1 */
    {0x26, {0x01}, 1},
    /* Positive gamma correction */
    {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
    /* Negative gamma correction */
    {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
    /* Column address set, SC=0, EC=0xEF */
    {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
    /* Page address set, SP=0, EP=0x013F */
    {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
    /* Memory write */
    {0x2C, {0}, 0},
    /* Entry mode set, Low vol detect disabled, normal display */
    {0xB7, {0x07}, 1},
    /* Display function control */
    {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
    /* Sleep out */
    {0x11, {0}, 0x80},
    /* Display on */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff},
};

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */

 //this function sends a command to the LCD, where we use the function 
 //spi_device_polling_transmit to send the command

 //spi_device_handle_t spi - tells the code which spi peripheral to use to talk to the screen.
 //cmd - 1 byte command to send to the lcd.
 //keep_cs_active, a flag if true keeps chip select pulled low after sending, meaning
 //"stay tuned" more data will be on the way after this command. 
 void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd, bool keep_cs_active) {

    esp_err_t ret; //variable to store the success/error returned by the SPI functions.
    spi_transaction_t t; //a struct required by the ESP_IDF SPI program.
    memset(&t, 0, sizeof(t)); //wipes the memory of the struct to zero, so 
    //old garbage data doesn't cause bugs.



    t.length = 8; //set the data length to 1 byte because we only send 1 byte at a time. 
    t.tx_buffer = &cmd; //ptr to the address of the command byte.
    t.user = (void*)0; //dk
    if (keep_cs_active) { //tells the program to keep the CS active, don't let go of the screen yet.
        t.flags = SPI_TRANS_CS_KEEP_ACTIVE; //keep chip select active after data transfer.
    }
    ret = spi_device_polling_transmit(spi, &t); //transmit the data in polling mode
    //which means the CPU waits until the data transfer finishes rather than using background interrupts.

    //then saves the result of the data transfer into ret.
    assert(ret == ESP_OK); //see if we have no issues.
    //if successful then we get back ESP_OK.
    //if fail, then assert() will crash so you know what goes wrong.


 }

//spi_transaction_t struct.
 /*
 typedef struct {
    uint32_t flags;                 // Bitwise flags to configure transaction options (e.g. SPI_TRANS_USE_RXDATA)
    uint16_t cmd;                   // Command data to send (length defined in spi_device_interface_config_t)
    uint64_t addr;                  // Address data to send (length defined in spi_device_interface_config_t)
    size_t length;                  // Total data length to transmit/receive, in BITS
    size_t rxlength;                // Total data length to receive, in BITS (defaults to 'length' if 0)
    void *user;                     // User-defined token/pointer, useful for tracking callbacks
    union {
        const void *tx_buffer;      // Pointer to transmit buffer
        uint8_t tx_data[4];         // Array containing data if SPI_TRANS_USE_TXDATA flag is set
    };
    union {
        void *rx_buffer;            // Pointer to receive buffer
        uint8_t rx_data[4];         // Array containing data if SPI_TRANS_USE_RXDATA flag is set
    };
} spi_transaction_t;
 */