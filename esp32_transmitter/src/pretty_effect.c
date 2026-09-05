#include <math.h>
#include "pretty_effect.h"
#include "sdkconfig.h"
#include "decode_image.h"
#include "transmit_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define SWAP16(c) (((c) >> 8) | (((c) & 0xFF) << 8))


uint16_t *pixels = NULL;


//color in the RGB565 format
//#define COLOR_HOVER_BORDER 0x07FF //neon cursor 
//can try other colors
//FFDE17 - golden yellow
//FFFFF - pure white
//39FF14 - 4DEEEA - vibrant lime or neon green

#define COLOR_NORMAL_BORDER 0x7BEF //subtle gray
#define COLOR_PULSE_YELLOW  SWAP16(0xFF66) // #FFEE33 (Bright Electric Yellow)
#define COLOR_PULSE_GOLD    SWAP16(0xFD40) // #FFAA00 (Warm Amber Gold)

// Center Y of each pill button on the 240x320 screen (shifted 2px up)
static const int pill_center_y[TOTAL_MODES] = {
    [MANUAL_MODE] = 126, // Manual Mode center
    [AUTO_MODE]   = 170, // Auto Mode center
    [IMU_MODE]    = 214  // IMU Mode center
};

// Grab an rgb16 pixel from decoded JPEG buffer
static inline uint16_t get_bgnd_pixel(int x, int y)
{
    x = (x < 0) ? 0 : (x >= IMAGE_W) ? IMAGE_W - 1 : x;
    y = (y < 0) ? 0 : (y >= IMAGE_H) ? IMAGE_H - 1 : y;
    return (uint16_t) * (pixels + (y * IMAGE_W) + x);
}

// Determine the pixel color based on the current page and pill coordinates
static inline uint16_t apply_overlay(int x, int y, uint16_t bg_pixel, uint16_t hover_color) {
    // If not on the menu page, return clean background
    if (current_page != PAGE_MENU) {
        return bg_pixel; 
    }

    int cy = pill_center_y[hovered_mode];

    // Quick bounding box check: expanded top to cy - 22 so top border is not clipped
    if (y >= cy - 22 && y <= cy + 21 && x >= 51 && x <= 190) {
        // Find horizontal distance to central line segment
        int dx = 0;
        if (x < 78) {
            dx = 78 - x;       // In left rounded cap
        } else if (x > 170) {
            dx = x - 170;      // In right rounded cap
        }

        // Offset dy by +1 for the top half (y < cy) to reach 1 pixel higher
        int dy = (y < cy) ? (y - cy + 1) : (y - cy);
        int dist_sq = dx * dx + dy * dy;

        // Radius: ~18.5 to ~20.5 gives a clean 2px border
        if (dist_sq >= 335 && dist_sq <= 415) {
            return hover_color;
        }
    }

    // Inside the pill and everywhere else: original crisp artwork!
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
