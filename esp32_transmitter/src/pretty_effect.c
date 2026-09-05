#include <math.h>
#include "pretty_effect.h"
#include "sdkconfig.h"
#include "decode_image.h"
#include "transmit_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


uint16_t *pixels = NULL;


//color in the RGB565 format
//#define COLOR_HOVER_BORDER 0x07FF //neon cursor 
//can try other colors
//FFDE17 - golden yellow
//FFFFF - pure white
//39FF14 - 4DEEEA - vibrant lime or neon green

#define COLOR_NORMAL_BORDER 0x7BEF //subtle gray
#define COLOR_PULSE_YELLOW  0xFF66 // #FFEE33 (Bright Electric Yellow)
#define COLOR_PULSE_GOLD    0xFD40 // #FFAA00 (Warm Amber Gold)

typedef struct  {
    int x, y, w, h;

} button_box_t;

//3 mode buttons centered horizontally on the 240x320 screen

//depending on which mode im on, ill draw the cursor around that mode.

//like in manual mode = 0, the cursor will be drawn around it.
static const button_box_t mode_boxes[TOTAL_MODES] = {
    { .x = 25, .y = 80,  .w = 190, .h = 42 }, // Mode 0: MANUAL
    { .x = 25, .y = 140, .w = 190, .h = 42 }, // Mode 1: AUTO
    { .x = 25, .y = 200, .w = 190, .h = 42 }  // Mode 2: IMU
};

// Grab an rgb16 pixel from decoded JPEG buffer
static inline uint16_t get_bgnd_pixel(int x, int y)
{
    x = (x < 0) ? 0 : (x >= IMAGE_W) ? IMAGE_W - 1 : x;
    y = (y < 0) ? 0 : (y >= IMAGE_H) ? IMAGE_H - 1 : y;
    return (uint16_t) * (pixels + (y * IMAGE_W) + x);
}

//determine the pixel color based on the urrent page and button coordinates

static inline uint16_t apply_overlay(int x, int y, uint16_t bg_pixel, uint16_t hover_color) {

    //if not on the menu page, like if we're on the linkedin page, or github,
    //or instagram page

    if (current_page != PAGE_MENU) {
        return bg_pixel; 
    }

    //if we're on the menu page, check if (x,y) are inside any of the 3 mode boxes
    for (int i = 0; i < TOTAL_MODES; i++) {
        int bx = mode_boxes[i].x;
        int by = mode_boxes[i].y;
        int bw = mode_boxes[i].w;
        int bh = mode_boxes[i].h;

        //is the pixel inside this box?
        if (x >= bx && x < (bx + bw) && y >= by && y < (by + bh)) {
            // Is this the button the cursor is hovering on?
            bool is_hovered = (i == hovered_mode);

            // Hovered gets thick 3px pulsing border; inactive gets thin 1px gray border
            int border_th = is_hovered ? 3 : 1; //if we not hovered it gets 1px
            //else we get 3 px border.

            //is_hovered = true then but the hover color, else its the normal border.
            uint16_t border_col = is_hovered ? hover_color : COLOR_NORMAL_BORDER;
            // Check if (x, y) is on the outer border edge
            if (x < (bx + border_th) || x >= (bx + bw - border_th) || y < (by + border_th) || y >= (by + bh - border_th)) {
                return border_col;
            }

        // Inside the button box: tint the background slightly for a translucent "glass" look
        return ((bg_pixel & 0xF7DE) >> 1);
        }
    }
    //everywhere else on the screen, show the normal background.
    return bg_pixel;
}


void pretty_effect_calc_lines(uint16_t *dest, int line, int frame, int linect)
{
    if (!pixels) return;

    //we need to have the pulsing effect for our cursor
    bool pulse = ((xTaskGetTickCount() / pdMS_TO_TICKS(250)) % 2) == 0; //don't want it too fast,
    //just increase the delay to slow down the blinking
    uint16_t hover_color = pulse ? COLOR_PULSE_YELLOW : COLOR_PULSE_GOLD;


    // Direct pixel copy with no wave distortion
    for (int y = line; y < line + linect; y++) {
        for (int x = 0; x < 240; x++) {
            uint16_t bg = get_bgnd_pixel(x,y);
            *dest++ = apply_overlay(x, y, bg, hover_color);
        }
    }
}

esp_err_t pretty_effect_init(void)
{
    return decode_image(0, &pixels);
}
