#include "metrics.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "transmit_data.h"
#include "joystick.h"
#include <math.h>
#include <stdlib.h>

#define TARGET_LOOP_PERIOD_US  10000 // 10 ms nominal period (100 Hz)
#define DEADLINE_THRESHOLD_US  25000 // Flag if loop takes > 25 ms

static volatile int64_t s_loop_start_time = 0;
static volatile int64_t s_last_loop_start_time = 0;
static volatile int64_t s_tx_start_time = 0;

static volatile float    s_cpu_load_pct = 5.0f;
static volatile float    s_latency_ms = 1.2f;
static volatile float    s_jitter_ms = 0.2f;
static volatile float    s_control_rate_hz = 100.0f;
static volatile uint32_t s_missed_deadlines = 0;

static float   s_prev_latency_ms = 1.2f;
static int64_t s_active_time_accum = 0;
static int64_t s_total_time_accum = 0;
static uint32_t s_sample_count = 0;

// 1. Direction from Joystick X/Y deflection
const char* metrics_get_direction_str(void) {
    int joy_x = 0, joy_y = 0;
    get_joystick_screen_coords(&joy_x, &joy_y);
    int dx = joy_x - GRID_CENTER_X;
    int dy = GRID_CENTER_Y - joy_y; // Invert Y so up is positive

    int deadband = 8;
    if (abs(dx) < deadband && abs(dy) < deadband) return "STOP";
    if (dy >= deadband && dx >= deadband)         return "FWD-R";
    if (dy >= deadband && dx <= -deadband)        return "FWD-L";
    if (dy <= -deadband && dx >= deadband)        return "REV-R";
    if (dy <= -deadband && dx <= -deadband)        return "REV-L";
    if (dy >= deadband)                           return "FWD";
    if (dy <= -deadband)                          return "REV";
    if (dx >= deadband)                           return "RIGHT";
    if (dx <= -deadband)                          return "LEFT";
    return "STOP";
}

// 2. Speed Setting % (0-255 mapped to 0-100%)
uint8_t metrics_get_speed_percent(void) {
    return (uint8_t)(((uint32_t)current_speed * 100) / 255);
}

// 3. Actual Speed (Placeholder until encoder is wired)
const char* metrics_get_actual_speed_str(void) {
    return "--"; 
}

// 4. Joystick X/Y percentage values (-100 to +100)
int16_t metrics_get_joy_x_val(void) {
    int joy_x = 0, joy_y = 0;
    get_joystick_screen_coords(&joy_x, &joy_y);
    int val = ((joy_x - GRID_CENTER_X) * 100) / GRID_RADIUS_X;
    if (val > 100) val = 100;
    if (val < -100) val = -100;
    return (int16_t)val;
}

int16_t metrics_get_joy_y_val(void) {
    int joy_x = 0, joy_y = 0;
    get_joystick_screen_coords(&joy_x, &joy_y);
    int val = ((GRID_CENTER_Y - joy_y) * 100) / GRID_RADIUS_Y;
    if (val > 100) val = 100;
    if (val < -100) val = -100;
    return (int16_t)val;
}

// 5. System Performance Getters
float metrics_get_cpu_load(void){ 
    return s_cpu_load_pct; 
    }

float metrics_get_latency_ms(void){ 
    return s_latency_ms; 
    }

float metrics_get_jitter_ms(void) { 
    return s_jitter_ms; 
    }

uint32_t metrics_get_missed_deadlines(void) { 
    return s_missed_deadlines; 
    }

float metrics_get_control_rate_hz(void) { 
    return s_control_rate_hz; 
    }

// --- Timing / Statistics Calculation Hooks ---

void metrics_record_loop_start(void) {
    int64_t now = esp_timer_get_time();
    s_loop_start_time = now;

    if (s_last_loop_start_time > 0) {
        int64_t period = now - s_last_loop_start_time;
        if (period > 1000 && period < 100000) {
            float instant_hz = 1000000.0f / (float)period;
            s_control_rate_hz = s_control_rate_hz * 0.95f + instant_hz * 0.05f;
        }
    }
    s_last_loop_start_time = now;
}

void metrics_record_loop_end(void) {
    int64_t now = esp_timer_get_time();
    int64_t active_us = now - s_loop_start_time;

    // Real compute missed deadline: active work took longer than the 10ms loop budget
    if (active_us > TARGET_LOOP_PERIOD_US) {
        s_missed_deadlines++;
    }

    s_active_time_accum += active_us;
    s_total_time_accum += TARGET_LOOP_PERIOD_US;
    s_sample_count++;

    // Compute CPU % every 50 samples (~500 ms)
    if (s_sample_count >= 50) {
        if (s_total_time_accum > 0) {
            float load = ((float)s_active_time_accum * 100.0f) / (float)s_total_time_accum;
            load += 4.5f; // Add baseline background/Wi-Fi task overhead
            if (load > 99.9f) load = 99.9f;
            s_cpu_load_pct = s_cpu_load_pct * 0.7f + load * 0.3f;
        }
        s_active_time_accum = 0;
        s_total_time_accum = 0;
        s_sample_count = 0;
    }
}

void metrics_record_espnow_tx_start(void) {
    s_tx_start_time = esp_timer_get_time();
}

void metrics_record_espnow_tx_done(esp_now_send_status_t status) {
    if (s_tx_start_time <= 0) return;
    int64_t now = esp_timer_get_time();
    int64_t duration_us = now - s_tx_start_time;
    s_tx_start_time = 0;

    if (duration_us > 0 && duration_us < 100000) {
        float instant_ms = (float)duration_us / 1000.0f;
        float delta = fabsf(instant_ms - s_prev_latency_ms);
        s_prev_latency_ms = instant_ms;

        // RFC 3550 standard EMA jitter filter
        s_jitter_ms += (delta - s_jitter_ms) * 0.125f;
        s_latency_ms = s_latency_ms * 0.85f + instant_ms * 0.15f;
    }
}
