#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include <stdio.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t  button_data;
    uint8_t  speed;
    uint16_t joystick_x;
    uint16_t joystick_y;
    uint8_t  imu_x;
    uint8_t  imu_y;
    uint8_t  mode;
} data_packet_t;



typedef struct __attribute__((packed)) {
  float actualspeed;
  float leftWheelSpeed;
  float rightWheelSpeed;

  //wheel encoders
  int32_t leftEncoder;
  int32_t rightEncoder;

  //states and status
  uint8_t speedSetting;      // Commanded throttle %
  uint8_t direction;         // 0=STOP, 1=FWD, 2=REV, etc.
  uint8_t lineSensors;       // 8-bit sensor mask
  uint8_t emergencyStop;     // 1 if stopped, 0 if OK

  //stm32 shit
  uint8_t cpuLoad; //stm32 cpu load
  float controlRate;//loop frequency 
  float latencyMs; //execution latency
  float jitterMs; //loop jitter 
  uint16_t missedDeadlines;

} robot_status_t;

//everytime you want to add a new mode, just add it here.
typedef enum {
  UART_MODE_MENU,
  UART_MODE_MOTOR,
  UART_MODE_COMBINED,
  UART_MODE_VOLTAGE,
  UART_MODE_BOTH,
  UART_MODE_NORMALIZE,
  UART_MODE_AUTO,
  UART_MODE_SERVO,
  UART_MODE_STEPPER,
  UART_MODE_SPEAKER,
  UART_MODE_IMU
} UART_ControlMode;

void UART_CONTROL_init(void);
void UART_CONTROL_update(void);
void UART_CONTROL_check_timeout(void);
UART_ControlMode UART_CONTROL_GetMode(void);
void menu_imu(void);
void menu_main(void);
void menu_motor(void);
void menu_combined(void);
void menu_speaker(void);
void menu_stepper(void);
void menu_normalized(void);
void menu_autonomous(void);
void menu_servo(void);
void menu_voltage(void);
void menu_both(void);
void Telemetry_Update_Wheel_Speeds(float delta_time_sec);
void UART_Send_Telemetry(void);
void DWT_Init(void);




#endif
