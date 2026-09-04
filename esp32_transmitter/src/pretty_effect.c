#include <math.h>
#include <stdlib.h>
#include "pretty_effect.h"
#include "sdkconfig.h"

#define IMAGE_W 320
#define IMAGE_H 240

static uint16_t *pixels = NULL;

//Grab a rgb16 pixel from the background pattern
static inline uint16_t get_bgnd_pixel(int x, int y)
{
    x = (x < 0) ? 0 : (x >= IMAGE_W) ? IMAGE_W - 1 : x;
    y = (y < 0) ? 0 : (y >= IMAGE_H) ? IMAGE_H - 1 : y;

    return *(pixels + (y * IMAGE_W) + x);
}

static int prev_frame = -1;
static int8_t xofs[320], yofs[240];
static int8_t xcomp[320], ycomp[240];

void pretty_effect_calc_lines(uint16_t *dest, int line, int frame, int linect)
{
    if (!pixels) return;

    if (frame != prev_frame) {
        for (int x = 0; x < 320; x++) {
            xofs[x] = sin(frame * 0.15 + x * 0.06) * 4;
            xcomp[x] = sin(frame * 0.11 + x * 0.12) * 4;
        }
        for (int y = 0; y < 240; y++) {
            yofs[y] = sin(frame * 0.1 + y * 0.05) * 4;
            ycomp[y] = sin(frame * 0.07 + y * 0.15) * 4;
        }
        prev_frame = frame;
    }
    for (int y = line; y < line + linect; y++) {
        for (int x = 0; x < 320; x++) {
            *dest++ = get_bgnd_pixel(x + yofs[y] + xcomp[x], y + xofs[x] + ycomp[y]);
        }
    }
}

esp_err_t pretty_effect_init(void)
{
    if (pixels == NULL) {
        pixels = (uint16_t *)calloc(IMAGE_W * IMAGE_H, sizeof(uint16_t));
        if (!pixels) return ESP_ERR_NO_MEM;

        // Generate a smooth RGB gradient background pattern
        for (int y = 0; y < IMAGE_H; y++) {
            for (int x = 0; x < IMAGE_W; x++) {
                uint8_t r = (x * 31) / IMAGE_W;
                uint8_t g = (y * 63) / IMAGE_H;
                uint8_t b = ((x + y) * 31) / (IMAGE_W + IMAGE_H);
                uint16_t color = (r << 11) | (g << 5) | b;
                pixels[y * IMAGE_W + x] = (color >> 8) | (color << 8); // Swap endianness for SPI LCD
            }
        }
    }
    return ESP_OK;
}
