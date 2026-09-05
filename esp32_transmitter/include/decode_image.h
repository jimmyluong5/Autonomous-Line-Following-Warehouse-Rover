#pragma once
#include <stdint.h>
#include "esp_err.h"

// Portrait orientation: 240 width x 320 height
#define IMAGE_W 240
#define IMAGE_H 320
#define TOTAL_FRAMES 8

#ifdef __cplusplus
extern "C" {
#endif
extern uint16_t *pixels;
esp_err_t decode_image(int frame_idx, uint16_t **pixels);
void decode_all_frames(void);
#ifdef __cplusplus
}
#endif
