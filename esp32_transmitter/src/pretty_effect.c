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

void pretty_effect_calc_lines(uint16_t *dest, int line, int frame, int linect)
{
    if (!pixels) return;

    // Direct pixel copy with no wave distortion
    for (int y = line; y < line + linect; y++) {
        for (int x = 0; x < 240; x++) {
            *dest++ = get_bgnd_pixel(x, y);
        }
    }
}

esp_err_t pretty_effect_init(void)
{
    return decode_image(&pixels);
}
