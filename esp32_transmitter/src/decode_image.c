/*
 * SPI Master example: JPEG decoder using TJpgDec
 */

#include "decode_image.h"
#include "tjpgd.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

// Binary included JPEG file in flash
//just change frame1 to whatever the img file is called to change the image.

//change in this file, platformio.ini and CMakeLists.TXT
// Note: hyphens '-' and dots '.' in filenames are automatically converted to underscores '_' in assembly symbols
extern const uint8_t image_jpg_start[] 
asm("_binary_frame1_jpg_start");
extern const uint8_t image_jpg_end[]   
asm("_binary_frame1_jpg_end");

extern const uint8_t image_jpg_start[]
asm("_binary_frame2_jpg_start");
extern const uint8_t image_jpg_end[]   
asm("_binary_frame2_jpg_end");

extern const uint8_t image_jpg_start[] 
asm("_binary_frame3_jpg_start");
extern const uint8_t image_jpg_end[]   
asm("_binary_frame3_jpg_end");

extern const uint8_t image_jpg_start[]
asm("_binary_frame4_jpg_start");
extern const uint8_t image_jpg_end[]   
asm("_binary_frame4_jpg_end");

extern const uint8_t image_jpg_start[] 
asm("_binary_frame5_jpg_start");
extern const uint8_t image_jpg_end[]   
asm("_binary_frame5_jpg_end");

extern const uint8_t image_jpg_start[]
asm("_binary_frame6_jpg_start");
extern const uint8_t image_jpg_end[]   
asm("_binary_frame6_jpg_end");

extern const uint8_t image_jpg_start[] 
asm("_binary_frame7_jpg_start");
extern const uint8_t image_jpg_end[]   
asm("_binary_frame7_jpg_end");

extern const uint8_t image_jpg_start[]
asm("_binary_frame8_jpg_start");
extern const uint8_t image_jpg_end[]   
asm("_binary_frame8_jpg_end");



static const char *TAG = "ImageDec";

// Struct to pass to decoder callback
typedef struct {
    const uint8_t *inData;
    uint32_t inPos;
    uint32_t inLen;
    uint16_t **outData;
} JpegDev;

// Stream input function for TJpgDec
static size_t infunc(JDEC *decoder, uint8_t *buf, size_t len) {
    JpegDev *jd = (JpegDev *)decoder->device;
    if (jd->inPos + len > jd->inLen) {
        len = jd->inLen - jd->inPos;
    }
    if (buf != NULL && len > 0) {
        memcpy(buf, jd->inData + jd->inPos, len);
    }
    jd->inPos += len;
    return len;
}

// Block output function for TJpgDec
static int outfunc(JDEC *decoder, void *bitmap, JRECT *rect) {
    JpegDev *jd = (JpegDev *)decoder->device;
    uint16_t *in = (uint16_t *)bitmap;
    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            if (x < IMAGE_W && y < IMAGE_H) {
                // Byte swap for ILI9341 SPI
                uint16_t color = *in++;
                (*jd->outData)[y * IMAGE_W + x] = (color >> 8) | (color << 8);
            } 
            else {
                in++;
            }
        }
    }
    return 1; // Continue decoding
}

esp_err_t decode_image(uint16_t **pixels) {
    char *work = NULL;
    JDEC decoder;
    JpegDev jd;
    *pixels = NULL;
    esp_err_t ret = ESP_OK;

    // Workspace required by TJpgDec
    work = (char *)calloc(3800, sizeof(char));
    if (work == NULL) {
        ESP_LOGE(TAG, "Cannot allocate workspace");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    // Allocate memory for the 320x240 image
    *pixels = (uint16_t *)calloc(IMAGE_H * IMAGE_W, sizeof(uint16_t));
    if (*pixels == NULL) {
        ESP_LOGE(TAG, "Cannot allocate pixel memory");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    jd.inData = image_jpg_start;
    jd.inPos = 0;
    jd.inLen = image_jpg_end - image_jpg_start;
    jd.outData = pixels;

    // Prepare the JPEG decoder
    JRESULT res = jd_prepare(&decoder, infunc, work, 3800, &jd);
    if (res != JDR_OK) {
        ESP_LOGE(TAG, "jd_prepare failed (%d)", res);
        ret = ESP_FAIL;
        goto err;
    }

    // Decompress the JPEG image
    res = jd_decomp(&decoder, outfunc, 0);
    if (res != JDR_OK) {
        ESP_LOGE(TAG, "jd_decomp failed (%d)", res);
        ret = ESP_FAIL;
        goto err;
    }

    free(work);
    ESP_LOGI(TAG, "JPEG successfully decoded (%d x %d px)!", decoder.width, decoder.height);
    return ESP_OK;

err:
    if (work) free(work);
    if (*pixels) free(*pixels);
    *pixels = NULL;
    return ret;
}
