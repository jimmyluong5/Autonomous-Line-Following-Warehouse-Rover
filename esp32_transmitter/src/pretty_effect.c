#include <math.h>
#include "pretty_effect.h"
#include "sdkconfig.h"
#include "decode_image.h"

uint16_t *pixels = NULL;

// Grab a rgb16 pixel from the decoded image (240x320)
static inline uint16_t get_bgnd_pixel(int x, int y)
{
    // Clamp coordinates to valid image bounds
    x = (x < 0) ? 0 : (x >= IMAGE_W) ? IMAGE_W - 1 : x;
    y = (y < 0) ? 0 : (y >= IMAGE_H) ? IMAGE_H - 1 : y;

    return (uint16_t) * (pixels + (y * IMAGE_W) + x);
}

static int prev_frame = -1;
static int8_t xofs[240], yofs[320];
static int8_t xcomp[240], ycomp[320];

void pretty_effect_calc_lines(uint16_t *dest, int line, int frame, int linect)
{
    if (!pixels) return;

    if (frame != prev_frame) {
        for (int x = 0; x < 240; x++) {
            xofs[x] = sin(frame * 0.15 + x * 0.06) * 4;
            xcomp[x] = sin(frame * 0.11 + x * 0.12) * 4;
        }
        for (int y = 0; y < 320; y++) {
            yofs[y] = sin(frame * 0.1 + y * 0.05) * 4;
            ycomp[y] = sin(frame * 0.07 + y * 0.15) * 4;
        }
        prev_frame = frame;
    }
    for (int y = line; y < line + linect; y++) {
        for (int x = 0; x < 240; x++) {
            *dest++ = get_bgnd_pixel(x + yofs[y] + xcomp[x], y + xofs[x] + ycomp[y]);
        }
    }
}

esp_err_t pretty_effect_init(void)
{
    return decode_image(&pixels);
}
