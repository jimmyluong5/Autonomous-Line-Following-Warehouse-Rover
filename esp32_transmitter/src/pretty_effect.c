#include <math.h>
#include <stdio.h>
#include <string.h>
#include "pretty_effect.h"
#include "sdkconfig.h"
#include "decode_image.h"
#include "transmit_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "joystick.h"
#include "metrics.h"
#include "font5x7.h"

#define SWAP16(c) (((c) >> 8) | (((c) & 0xFF) << 8))

uint16_t *pixels = NULL;

#define COLOR_JOYSTICK      SWAP16(0xFD00)
#define COLOR_DOT_BORDER    SWAP16(0xC260)
#define COLOR_PULSE_YELLOW  SWAP16(0xFF66) // #FFEE33
#define COLOR_PULSE_GOLD    SWAP16(0xFD40) // #FFAA00
#define COLOR_TEXT_CYAN     SWAP16(0x07FF) // #00FFFF Electric Cyan

// Center Y of each pill button on the 240x320 screen
static const int pill_center_y[TOTAL_MODES] = {
    [MANUAL_MODE] = 126,
    [AUTO_MODE]   = 170,
    [IMU_MODE]    = 214
};

typedef struct {
    char str[12];
    int x0;
    int y0;
    int len;
} ui_text_item_t;

static ui_text_item_t s_ui_texts[10];

static void prepare_manual_ui_strings(void) {
    // 1. Joystick X (-100 to +100)
    int16_t jx = metrics_get_joy_x_val();
    snprintf(s_ui_texts[0].str, sizeof(s_ui_texts[0].str), "%+d", jx);
    s_ui_texts[0].len = strlen(s_ui_texts[0].str);
    s_ui_texts[0].x0 = 73 - (s_ui_texts[0].len * 6) / 2;
    s_ui_texts[0].y0 = 128;

    // 2. Joystick Y (-100 to +100)
    int16_t jy = metrics_get_joy_y_val();
    snprintf(s_ui_texts[1].str, sizeof(s_ui_texts[1].str), "%+d", jy);
    s_ui_texts[1].len = strlen(s_ui_texts[1].str);
    s_ui_texts[1].x0 = 73 - (s_ui_texts[1].len * 6) / 2;
    s_ui_texts[1].y0 = 145;

    // 3. Speed Setting (0-100%)
    uint8_t spd = metrics_get_speed_percent();
    snprintf(s_ui_texts[2].str, sizeof(s_ui_texts[2].str), "%u%%", spd);
    s_ui_texts[2].len = strlen(s_ui_texts[2].str);
    s_ui_texts[2].x0 = 193 - (s_ui_texts[2].len * 6) / 2;
    s_ui_texts[2].y0 = 58;

    // 4. Actual Speed Placeholder
    const char *act_spd = metrics_get_actual_speed_str();
    snprintf(s_ui_texts[3].str, sizeof(s_ui_texts[3].str), "%s", act_spd);
    s_ui_texts[3].len = strlen(s_ui_texts[3].str);
    s_ui_texts[3].x0 = 193 - (s_ui_texts[3].len * 6) / 2;
    s_ui_texts[3].y0 = 99;

    // 5. Direction String
    const char *dir = metrics_get_direction_str();
    snprintf(s_ui_texts[4].str, sizeof(s_ui_texts[4].str), "%s", dir);
    s_ui_texts[4].len = strlen(s_ui_texts[4].str);
    s_ui_texts[4].x0 = 193 - (s_ui_texts[4].len * 6) / 2;
    s_ui_texts[4].y0 = 139;

    // 6. CPU Load %
    float cpu = metrics_get_cpu_load();
    snprintf(s_ui_texts[5].str, sizeof(s_ui_texts[5].str), "%.1f", cpu);
    s_ui_texts[5].len = strlen(s_ui_texts[5].str);
    s_ui_texts[5].x0 = 172 - (s_ui_texts[5].len * 6) / 2;
    s_ui_texts[5].y0 = 201;

    // 7. Latency (ms)
    float lat = metrics_get_latency_ms();
    snprintf(s_ui_texts[6].str, sizeof(s_ui_texts[6].str), "%.1f", lat);
    s_ui_texts[6].len = strlen(s_ui_texts[6].str);
    s_ui_texts[6].x0 = 172 - (s_ui_texts[6].len * 6) / 2;
    s_ui_texts[6].y0 = 220;

    // 8. Jitter (ms)
    float jit = metrics_get_jitter_ms();
    snprintf(s_ui_texts[7].str, sizeof(s_ui_texts[7].str), "%.1f", jit);
    s_ui_texts[7].len = strlen(s_ui_texts[7].str);
    s_ui_texts[7].x0 = 172 - (s_ui_texts[7].len * 6) / 2;
    s_ui_texts[7].y0 = 239;

    // 9. Missed Deadlines
    uint32_t mdl = metrics_get_missed_deadlines();
    snprintf(s_ui_texts[8].str, sizeof(s_ui_texts[8].str), "%lu", (unsigned long)mdl);
    s_ui_texts[8].len = strlen(s_ui_texts[8].str);
    s_ui_texts[8].x0 = 172 - (s_ui_texts[8].len * 6) / 2;
    s_ui_texts[8].y0 = 259;

    // 10. Control Rate (Hz)
    float hz = metrics_get_control_rate_hz();
    snprintf(s_ui_texts[9].str, sizeof(s_ui_texts[9].str), "%.1f", hz);
    s_ui_texts[9].len = strlen(s_ui_texts[9].str);
    s_ui_texts[9].x0 = 172 - (s_ui_texts[9].len * 6) / 2;
    s_ui_texts[9].y0 = 280;
}

static inline bool check_text_pixel(int x, int y) {
    for (int i = 0; i < 10; i++) {
        int y0 = s_ui_texts[i].y0;
        if (y >= y0 && y < y0 + 7) {
            int x0 = s_ui_texts[i].x0;
            int total_w = s_ui_texts[i].len * 6;
            if (x >= x0 && x < x0 + total_w) {
                int char_idx = (x - x0) / 6;
                int char_x0 = x0 + char_idx * 6;
                char c = s_ui_texts[i].str[char_idx];
                if (font5x7_get_pixel(c, char_x0, y0, x, y)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Grab an rgb16 pixel from decoded JPEG buffer
static inline uint16_t get_bgnd_pixel(int x, int y)
{
    x = (x < 0) ? 0 : (x >= IMAGE_W) ? IMAGE_W - 1 : x;
    y = (y < 0) ? 0 : (y >= IMAGE_H) ? IMAGE_H - 1 : y;
    return (uint16_t) * (pixels + (y * IMAGE_W) + x);
}

// Determine the pixel color based on current page and overlay elements
static inline uint16_t apply_overlay(int x, int y, uint16_t bg_pixel, uint16_t hover_color) {
    // 1. Menu mode cursor
    if (current_page == PAGE_MENU) {
        int cy = pill_center_y[hovered_mode];
        if (y >= cy - 22 && y <= cy + 21 && x >= 51 && x <= 190) {
            int dx = (x < 78) ? (78 - x) : (x > 170) ? (x - 170) : 0;
            int dy = (y < cy) ? (y - cy + 1) : (y - cy);
            int dist_sq = dx * dx + dy * dy;
            if (dist_sq >= 335 && dist_sq <= 415) {
                return hover_color;
            }
        }
        return bg_pixel;
    }

    // 2. Manual mode: dynamic joystick dot & telemetry text
    if (current_page == PAGE_MANUAL) {
        static int dot_x = GRID_CENTER_X;
        static int dot_y = GRID_CENTER_Y;
        // Sample latest coordinates and prepare text strings at start of frame (0, 0)
        if (y == 0 && x == 0) {
            get_joystick_screen_coords(&dot_x, &dot_y);
            prepare_manual_ui_strings();
        }

        // Draw metrics text
        if (check_text_pixel(x, y)) {
            return COLOR_TEXT_CYAN;
        }

        // Check if inside joystick grid area on blankfirstpage.jpg
        if (y >= 35 && y <= 120 && x >= 18 && x <= 108) {
            int dx = x - dot_x;
            int dy = y - dot_y;
            int dist_sq = dx * dx + dy * dy;
            // Circle radius = 4 pixels (4*4 = 16)
            if (dist_sq <= 16) {
                return (dist_sq >= 10) ? COLOR_DOT_BORDER : COLOR_JOYSTICK;
            }
        }
        return bg_pixel;
    }

    return bg_pixel;
}

void pretty_effect_calc_lines(uint16_t *dest, int line, int frame, int linect)
{
    if (!pixels) return;

    bool pulse = ((xTaskGetTickCount() / pdMS_TO_TICKS(250)) % 2) == 0;
    uint16_t hover_color = pulse ? COLOR_PULSE_YELLOW : COLOR_PULSE_GOLD;

    for (int y = line; y < line + linect; y++) {
        for (int x = 0; x < 240; x++) {
            uint16_t bg = get_bgnd_pixel(x, y);
            *dest++ = apply_overlay(x, y, bg, hover_color);
        }
    }
}

esp_err_t pretty_effect_init(void)
{
    return decode_image(0, &pixels);
}
