#pragma once
#include <stdint.h>
#include "esp_err.h"

// Portrait orientation: 240 width x 320 height
#define IMAGE_W 240
#define IMAGE_H 320

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t decode_image(uint16_t **pixels);

#ifdef __cplusplus
}
#endif
