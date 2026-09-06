#ifndef TRANSMIT_DATA_H
#define TRANSMIT_DATA_H

#include <stdint.h>
#include "esp_now.h"

#define MANUAL_MODE 0
#define AUTO_MODE 1
#define IMU_MODE 2


#define TOTAL_MODES 3

extern uint8_t current_speed;

typedef struct {
    uint8_t button_data;
    uint8_t speed;
    //uint16_t sequence;
    uint16_t joystick_x;
    uint16_t joystick_y;
    uint8_t imu_x;
    uint8_t imu_y;
    uint8_t mode; //eventually it'll contain more modes.
} data_packet_t;

//everytime you want to add a page, just add it here.
typedef enum {
    PAGE_MENU = 0,
    PAGE_MANUAL,
    PAGE_MANUAL_DATA, //2nd page of manual
    PAGE_AUTO,
    PAGE_AUTO_DATA,
    PAGE_IMU,
    PAGE_IMU_DATA,
    PAGE_GITHUB,
    PAGE_LINKEDIN,
    PAGE_LEFTPAGE,
    PAGE_MAX_COUNT // TOTAL NUMBER OF PAGES (4)
} page_t;

#define PAGE_EMPTY     PAGE_LEFTPAGE
#define PAGE_LEFT      PAGE_LEFTPAGE


void init_button_pin(void);
uint8_t read_buttons(void);
esp_err_t transmit_data(const uint8_t *receiver_mac, const data_packet_t *packet);
uint8_t update_speed(data_packet_t *packet);
uint8_t update_mode(data_packet_t *packet);

//static variables
extern page_t current_page; //this is the default mode
extern int hovered_mode; //which button is the cursor on
extern uint8_t active_mode; 
void process_arrow_keys(data_packet_t* packet);

#endif /* TRANSMIT_DATA_H */