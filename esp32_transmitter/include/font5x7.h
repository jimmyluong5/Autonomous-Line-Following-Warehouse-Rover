#ifndef FONT5X7_H
#define FONT5X7_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 5x7 ASCII bitmap font data covering ASCII 32 to 126
extern const uint8_t font5x7_data[][5];

// Returns true if pixel (px, py) in character 'c' placed at top-left (x0, y0) is active
static inline bool font5x7_get_pixel(char c, int x0, int y0, int px, int py) {
    if (c < 32 || c > 126) c = ' ';
    int col = px - x0;
    int row = py - y0;
    if (col < 0 || col >= 5 || row < 0 || row >= 7) return false;
    return (font5x7_data[c - 32][col] & (1 << row)) != 0;
}

#ifdef __cplusplus
}
#endif

#endif // FONT5X7_H
