#ifndef METRICS_H
#define METRICS_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_now.h"

#ifdef __cplusplus
extern "C" {
#endif

// --- Metric Getters for UI & Telemetry ---
uint8_t     metrics_get_speed_percent(void);
const char* metrics_get_direction_str(void);
const char* metrics_get_actual_speed_str(void);
int16_t     metrics_get_joy_x_val(void);
int16_t     metrics_get_joy_y_val(void);

float       metrics_get_cpu_load(void);        // CPU % (e.g. 12.4%)
float       metrics_get_latency_ms(void);      // ESP-NOW latency (e.g. 1.8 ms)
float       metrics_get_jitter_ms(void);       // Latency variation (e.g. 0.2 ms)
uint32_t    metrics_get_missed_deadlines(void);// Dropped packets / overrun count
float       metrics_get_control_rate_hz(void); // Real-time frequency (e.g. 100.2 Hz)

// --- Measurement Hooks for Main Loop & Callbacks ---
void metrics_record_loop_start(void);
void metrics_record_loop_end(void);
void metrics_record_espnow_tx_start(void);
void metrics_record_espnow_tx_done(esp_now_send_status_t status);

#ifdef __cplusplus
}
#endif

#endif // METRICS_H
