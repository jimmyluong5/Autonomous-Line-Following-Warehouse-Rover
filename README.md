# Multi-mode Rover
<img width="480" height="853" alt="line_following_github" src="https://github.com/user-attachments/assets/8277d7d8-333a-4ad6-8d29-b165df40b6e0" />




An autonomous line-following rover built around the **STM32G431KB**, using an 8-channel reflectance sensor array for navigation, UART for diagnostics and control, and ESP-NOW for wireless communication.

The project began as a basic motor-control prototype and has gradually evolved into a full rover platform with autonomous navigation, sensor monitoring, multiple operating modes, and wireless control.
<p align="center">
  <img src="https://github.com/user-attachments/assets/a8743f6e-6503-4095-bb05-f7fdd0e41f4b" width="24%" />
  <img src="https://github.com/user-attachments/assets/1136528b-bf2d-42e9-91b3-132347ca7aca" width="24%" />
  <img src="https://github.com/user-attachments/assets/8ce91f7d-2d42-44e9-8486-3224fbb3c6df" width="24%" />
  <img src="https://github.com/user-attachments/assets/ef135034-5aff-4634-a7c1-6e8d8ae37eac" width="24%" />
</p>


## Features

* Autonomous line following using an **STM32G431KB**
* 8-channel reflectance sensing using the **QTRX-MD-08A**
* External ADC acquisition using the **MCP3208**
* UART-based control and diagnostics through **PuTTY**
* DC motor control using **FIT0484 motors** and a **TB6612FNG dual motor driver**
* Regulated power distribution using **LM2596 buck converters**
* Wireless communication between two **ESP32-S3** modules using **ESP-NOW**
* Custom 8-bit command packet system for wireless control
* Multiple diagnostic and control modes for testing individual subsystems

## Development Process

### 1. Motor Control

The first stage of the project focused on getting the prototype working.

Two **FIT0484 DC motors** were controlled through a **TB6612FNG dual motor driver**, with the STM32G431KB acting as the main controller.

The power system uses **LM2596 buck converters** to provide regulated voltages to the motors and embedded electronics.

Once reliable bidirectional motor control was established, the prototype was mounted onto a rough cardboard chassis for early testing.

<img width="480" height="853" alt="motors_github" src="https://github.com/user-attachments/assets/ca83976e-c8b5-4a69-80d2-1aaa2ccd849a" />




### 2. Cardboard Prototype

A temporary cardboard rover was constructed to test the complete prototype before moving to a permanent chassis.

This prototype made it possible to verify

* Motor direction and speed control
* Wheel placement and mechanical alignment
* Breadboard wiring
* Initial sensor placement
* Basic autonomous movement

The prototype successfully demonstrated that the embedded control system and prototype could operate together before the mechanical design was finalized.

<img width="480" height="853" alt="first_drive_github" src="https://github.com/user-attachments/assets/3769e857-5dfd-4aa8-bf13-f9b57ea0bf7f" />



### 3. Reflectance Sensor Integration and UART Sensor Calibration and Diagnostics


The next stage introduced the **QTRX-MD-08A 8-channel reflectance array** for line detection.

The sensor outputs were connected to an external **MCP3208 ADC**, allowing the STM32G431KB to acquire all eight reflectance channels in real time.

Each channel produces an analog voltage corresponding to the reflectivity of the surface underneath the sensor.

During testing

* Higher measured voltage corresponded to darker surfaces
* Lower measured voltage corresponded to lighter surfaces

The STM32 reads the MCP3208 measurements and uses the eight sensor values to determine the rover's position relative to the line.

A UART-based diagnostic interface was also created using **PuTTY** to assist with sensor calibration and subsystem testing.

The interface displays both the measured voltage and raw ADC value for each reflectance channel.

Example output

```text
--- Sensor Test Mode Active ---

CH0: 1.91V | ADC: 2376
CH1: 1.92V | ADC: 2391
CH2: 1.96V | ADC: 2442
CH3: 1.98V | ADC: 2466
CH4: 1.96V | ADC: 2434
CH5: 1.97V | ADC: 2453
CH6: 2.05V | ADC: 2552
CH7: 1.96V | ADC: 2440
```

Testing the array across different surfaces made it possible to determine usable thresholds and calibrate the line-following algorithm.

The UART interface also provides multiple operating and diagnostic modes including

* Servo control
* DC motor testing
* Reflectance voltage monitoring
* Autonomous line-following mode

This made it possible to test individual subsystems without repeatedly modifying the firmware.



<img width="520" height="355" alt="image" src="https://github.com/user-attachments/assets/7266f23a-6372-4940-9bef-93d4786f03ce" />
<img width="492" height="395" alt="image" src="https://github.com/user-attachments/assets/38084f03-c8ef-4abd-8d70-750e41a66246" />




### 5. Autonomous Line Following

After calibrating the reflectance sensors, autonomous line following was implemented on the STM32G431KB.

The rover continuously samples all eight channels through the MCP3208 and uses the sensor distribution to determine how far the rover has moved away from the desired path.

Motor commands are then adjusted in real time to steer the rover back toward the line.

This resulted in a complete sensing and control loop

```text
QTRX-MD-08A
      ↓
   MCP3208
      ↓
STM32G431KB
      ↓
Line-Following Logic
      ↓
  TB6612FNG
      ↓
 FIT0484 Motors
```
### 6. CAD Design

After validating the rover's electronics and autonomous navigation on the initial prototype, the mechanical components for the final rover were designed in CAD.

The CAD design focused on creating a rigid and modular platform for the drivetrain, electronics, steering system, and other mechanical components. Designing the rover as an assembly also made it possible to verify component placement, mechanical clearances, and overall dimensions before fabrication.

#### Front Steering Mechanism

The front steering assembly was designed as a separate mechanical system before being integrated into the complete rover. The mechanism allows both front wheels to steer together while maintaining a compact geometry at the front of the chassis.

The assembly was modeled in CAD to verify the movement of the steering components, wheel alignment, and clearance between moving parts.

<img width="900" height="203" alt="front_assembly_zoomed" src="https://github.com/user-attachments/assets/be88c435-6175-4b57-9284-9151091c387f" />

  <i>CAD model demonstrating the rover's front steering assembly.</i>
</p>

#### Main Chassis and Component Layout

The main chassis provides the structural foundation for the rover and supports the motors, wheels, electronics, power system, and cargo platform.

Component placement was considered during the design process to keep the rover compact while maintaining enough space for wiring, assembly, and access to the electrical hardware.

#### Complete Rover Assembly

After designing the individual mechanical components, they were combined into a complete CAD assembly. This allowed the overall rover geometry to be evaluated before the parts were manufactured.

The completed assembly was used to check wheel placement, steering clearance, chassis dimensions, and the positioning of the rover's electrical and mechanical components.

<img width="532" height="301" alt="whole car" src="https://github.com/user-attachments/assets/edb7affe-be3b-4195-935e-133e81508760" />

  <i>Complete CAD assembly of the autonomous warehouse rover.</i>
</p>

The CAD models used for the rover can be found in the [`CAD Models`](./CAD%20Models) directory.

### 7. 3D-Printed Chassis

After validating the electronics and navigation system on the cardboard prototype, the rover was transferred to a custom **3D-printed chassis**.

The chassis provides mounting for

* DC motors
* Wheels
* Breadboard and embedded electronics
* Power system
* Reflectance sensor array
* Additional mechanical components

This provided a much more rigid and repeatable platform for continued development.
<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/c35e2f71-1075-40d3-a830-de78924b504b" />
<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/773da8c9-128c-478e-ad3e-9cb9869b7452" />


### 8. Servo-Based Suspension Testing

After transferring the rover to the 3D-printed chassis, I tested a **servo-actuated front suspension system** using an SG90 servo.

The servo was used to adjust the front of the chassis and verify that the suspension mechanism could move reliably under control from the STM32.

A short demonstration clip is included below.


<img width="480" height="853" alt="servo_github" src="https://github.com/user-attachments/assets/aa3a12b8-8584-4048-a9af-15f80190c2fd" />


The test helped validate the mechanical design before continuing with wireless control and additional rover integration.


### 9. ESP-NOW Wireless Communication

The current stage of the project focuses on wireless communication between two **ESP32-S3 modules**.

A custom command protocol is being implemented on top of **ESP-NOW**.

Button inputs on the transmitter are active-low. When a button is pressed, the corresponding command bit is set inside an **8-bit command packet**.

```text
Button Input
    ↓
GPIO Read
    ↓
8-Bit Command Packet
    ↓
ESP-NOW Transmission
    ↓
Receiver ESP32-S3
    ↓
Packet Decoding
    ↓
Command Execution
```


<img width="480" height="853" alt="esp32_github" src="https://github.com/user-attachments/assets/15d5e751-9ac8-4944-949a-a5fd62a3ef1e" />



The receiving ESP32-S3 unpacks the command byte and executes actions depending on which bits are active.

Initial testing has successfully demonstrated wireless command transmission and execution using LEDs.


### 10. Wireless Button Controller Prototype

After successfully establishing ESP-NOW communication between the two ESP32-S3 modules, I built a physical **button controller prototype** to provide direct wireless input to the rover.

The controller uses an **ESP32-S3** connected to multiple push buttons. Each button represents a different rover command and is read as an active-low GPIO input.

When a button is pressed, the ESP32-S3 updates the corresponding bit in the **8-bit command packet** and transmits the packet wirelessly using ESP-NOW.

The initial controller was assembled on breadboards to allow the button layout, wiring, and firmware to be tested and modified easily.

The prototype successfully demonstrated

* Reliable detection of multiple button inputs
* Active-low GPIO input handling
* Generation of the 8-bit command packet
* ESP-NOW transmission from the handheld controller
* Wireless reception and command decoding on the second ESP32-S3
* LED-based verification of transmitted commands
* Simultaneous handling of multiple command inputs

```text
Push Buttons
     ↓
ESP32-S3 GPIO
     ↓
8-Bit Command Packet
     ↓
   ESP-NOW
     ↓
Receiver ESP32-S3
     ↓
Command Decoding
     ↓
Rover Control
```

<img width="1920" height="2560" alt="photo_2026-08-21_12-02-11" src="https://github.com/user-attachments/assets/2219012e-3b85-4ee7-a2c4-481e44adae1c" />


This prototype confirms that the wireless controller architecture works as intended and provides a functional input device for the next stage of rover integration.

The next step is to connect the receiver ESP32-S3 to the rover control system so that button commands can directly control functions such as driving, operating modes, and additional rover mechanisms.

#### Current Status Update

Add the following under **Completed**

* Wireless button controller prototype
* 8-bit button command packet transmission
* Multi-button input testing

The wireless controller can now be used as the basis for full manual rover control alongside the autonomous line-following system.


### 11. Bidirectional Serial Monitoring and Wireless Command Debugging

After completing the wireless button controller prototype, I expanded the debugging interface so that both the transmitter and receiver ESP32-S3 modules can display controller activity through their respective serial connections.

On the transmitter side, button presses are detected from the active-low GPIO inputs, encoded into the 8-bit command packet, and displayed through the serial console. This makes it possible to verify that the correct command is being generated before it is transmitted wirelessly.

The packet is then sent over ESP-NOW to the receiver ESP32-S3. The receiver decodes the individual command bits and independently displays the interpreted rover command through its own serial console.

Using PuTTY, I can therefore monitor either side of the wireless connection and verify commands such as

MANUAL MODE
AUTONOMOUS MODE
INCREASING SPEED
DECREASING SPEED
STOP

<img width="987" height="726" alt="photo_2026-08-23_20-24-49" src="https://github.com/user-attachments/assets/f57871b0-3ed2-4805-978f-858f2236b0fb" />


The controller also supports an 8-bit speed value ranging from 0–255, with the UP and DOWN buttons adjusting the requested rover speed in approximately 5% increments.

Push Buttons
     ↓
     
Transmitter ESP32-S3
     ↓
Button Detection
     ↓
     
Serial Debug Output
     ↓
     
8-Bit Command Packet
     ↓
     
   ESP-NOW
     ↓
     
Receiver ESP32-S3
     ↓
     
Command Decoding
     ↓
     
Serial Debug Output
     ↓
     
MANUAL / AUTO / SPEED / STOP

Having serial output available on both ESP32-S3 modules provides visibility into both sides of the communication system. The transmitter output verifies that controller inputs are being correctly detected and encoded, while the receiver output verifies that the same commands are successfully transmitted, received, and decoded.

This significantly simplifies debugging because communication problems can be isolated to either the controller input stage, ESP-NOW transmission, or receiver-side command decoding.



<img width="538" height="956" alt="Adobe Express - IMG_5109" src="https://github.com/user-attachments/assets/989fd5d0-0708-4b79-a95e-a0d8b2106800" />



The wireless controller now supports both LED-based command verification and real-time serial debugging, providing a more reliable way to confirm that ESP-NOW packets are being received and interpreted correctly.

### 12. Full Rover Integration & Multi-Subsystem Control

With the 3D-printed chassis, power distribution, and core firmware validated, the rover has progressed to a **fully integrated, multi-mode platform**. All critical subsystems are now orchestrated directly by the STM32G431KB state machine:

* **Integrated Steering & Suspension**: The front servo suspension and steering mechanism is directly integrated into the drive loop, enforcing calibrated limits (45° to 135°) with dedicated combined driving modes (front active steering + rear differential motor thrust).
* **Unified UART Telemetry & Diagnostic Interface**: An interactive, non-blocking serial dashboard allows on-the-fly mode switching between:
  * `[m]` Motor Control Mode (Differential Drive)
  * `[c]` Combined Control System (Active Steering + Rear Motors)
  * `[a]` Autonomous PID Line Following
  * `[s]` Steering Servo Calibration (45°–135°)
  * `[t]` Stepper Motor Positioning
  * `[p]` Piezo Speaker / Buzzer Frequency Testing
  * `[v]` / `[n]` Real-Time ADC Reflectance & Surface Classification


### 13. Custom ESP32-S3 Wireless Handheld Controller & Real-Time Telemetry Dashboard
<img width="1920" height="2560" alt="photo_2026-09-06_17-12-59" src="https://github.com/user-attachments/assets/a79483f9-5157-4bf6-b995-1d22fd751318" />

To provide manual override, multi-mode switching, and live diagnostics for the rover, a dedicated handheld wireless controller was developed using a dual-core **ESP32-S3** (240 MHz) and a 2.4-inch **ILI9341 SPI TFT LCD (240×320)**.

The controller provides an interactive graphical user interface (GUI), live telemetry monitoring, and low-latency packet transmission over **ESP-NOW**.

#### 1. Hardware Architecture & Inputs
* **Microcontroller**: Freenove ESP32-S3 WROOM (16MB Flash, 8MB PSRAM, Dual Xtensa LX7 cores running at 240 MHz).
* **Display & Touch**: 2.4" 240×320 SPI LCD driven by an ILI9341 controller, sharing the SPI bus with an XPT2046 resistive touch controller.
* **Analog 2D Joystick**: Multi-sampled through SAR ADC1 (GPIO 4 and GPIO 5) with 16× oversampling, software deadband filtering, and axis normalization.
* **5-Way Tactile Button Matrix**: Multi-button input array with two-stage temporal debouncing for menu navigation, speed adjustments, and emergency stop.
* **Audio Feedback**: PWM buzzer generating audible confirmation tones on button interactions.

```text
[ Analog Joystick (ADC1) ] ──┐
[ 5-Button Matrix (GPIO) ] ──┼──> [ ESP32-S3#1 Controller & Transmitter ] ──(ESP-NOW 2.4GHz)──> [ESP32#2 Rover Receiver ]
[ Piezo Speaker (LEDC)   ] ──┤      │ (FreeRTOS Core 0/1)
[ ILI9341 240x320 LCD    ] ──┘      └──> [ Real-Time Telemetry & Graphics ]
```

#### 2. Embedded Graphics Engine & Menu Animation
* **High-Speed JPEG Decoder (`tjpgd`)**: Decompresses embedded full-color JPEG assets directly from Flash memory into DMA-accessible RAM.
* **Ping-Pong SPI DMA Buffering**: Uses dual parallel line buffers to pipeline SPI transfers asynchronously while computing the next scanline on the CPU.
* **Main Menu UI**: Plays a 15-frame animated rover sequence with an active pulsing yellow hover cursor to select operating modes (**Manual Mode**, **Autonomous Mode**, **IMU Mode**).
* **Showcase Pages**: Dedicated QR code and documentation screens for GitHub and LinkedIn.

#### 3. Real-Time Manual Dashboard & Custom Bitmap Font Engine
When entering **Manual Mode**, the controller renders an industrial diagnostic dashboard with live telemetry overlays:
* **Interactive 2D Joystick Grid**: Renders a dynamic orange position dot with a contrasting border inside a 2D coordinate grid, reflecting real-time physical stick deflection.
* **Embedded 5x7 ASCII Font Renderer**: A lightweight, zero-heap bitmap font engine (`font5x7`) that injects text characters directly into the scanline stream during display refresh.
* **Vehicle Status Display**:
  * **Speed Setting**: Real-time commanded throttle percentage (`0%` to `100%`).
  * **Direction**: Real-time heading / deflection state (`STOP`, `FWD`, `REV`, `LEFT`, `RIGHT`, `FWD-R`, `FWD-L`, `REV-R`, `REV-L`).
  * **Actual Speed**: Reserved telemetry slot for encoder feedback from the rover.

#### 4. Real-Time System Performance & RTOS Telemetry
To benchmark control loop determinism (comparing FreeRTOS preemptive multitasking vs bare-metal superloops), a dedicated telemetry module (`metrics.c`) tracks real-time performance indicators:

| Telemetry Metric | Measurement Method | Typical Reading | Purpose |
| :--- | :--- | :--- | :--- |
| **CPU Load** | Active computation vs sleep duty cycle | `~12–20%` | Verifies compute headroom |
| **Latency** | Hardware timer round-trip / send callback | `1.2–3.5 ms` | Measures wireless packet transmission latency |
| **Jitter** | RFC 3550 Exponential Moving Average filter | `0.2–0.8 ms` | Tracks packet timing stability |
| **Missed DL** | Execution time exceeding 10 ms budget | `0` | Proves hard real-time scheduling guarantees |
| **Control Rate** | High-resolution period frequency | `100.0 Hz` | Confirms consistent 10 ms control loop execution |

#### 5. Wireless Communication Protocol (`ESP-NOW`)
Control packets (`data_packet_t`) are packed into a compact binary structure and transmitted as 2.4 GHz peer-to-peer unicast packets to the receiver ESP32 on the rover:
```c
typedef struct {
    uint8_t  button_data;  // 5-bit tactile button mask
    uint8_t  speed;        // Commanded throttle (0–255)
    uint16_t joystick_x;   // Filtered X analog deflection
    uint16_t joystick_y;   // Filtered Y analog deflection
    uint8_t  imu_x;        // Reserved for IMU tilt control
    uint8_t  imu_y;        // Reserved for IMU tilt control
    uint8_t  mode;         // Active operating mode
} data_packet_t;
```

## Hardware

| Component       | Purpose                                   |
| --------------- | ----------------------------------------- |
| STM32G431KB     | Main rover controller                     |
| QTRX-MD-08A     | 8-channel reflectance sensor array        |
| MCP3208         | External ADC for reflectance measurements |
| FIT0484         | DC drive motors                           |
| TB6612FNG       | Dual DC motor driver                      |
| ESP32-S3 ×3     | Wireless transmitter and receiver         |
| LM2596          | Buck converters for voltage regulation    |
| SG90 Servo      | Mechanical actuation                      |
| LADDA Batteries | Main power source                         |

## Software and Communication

* Embedded C
* STM32 firmware
* UART
* SPI
* ESP-NOW
* PuTTY
* 8-bit command packet protocol

## Roadmap & Next Steps

```text
Current: Full Rover + UART Telemetry
                ↓
Phase 1: ESP32-to-STM32 Bridge & Wireless Joystick Control
                ↓
Phase 2: IMU Orientation & Heading Stabilization
                ↓
Phase 3: Time-of-Flight (ToF) Collision Detection & Auto-Braking
```

### Phase 1: ESP32-to-STM32 Bridge & Wireless Joystick Controller
* **Inter-MCU Communication**: Bridge the receiver ESP32-S3 directly to the STM32G431KB via hardware UART, forwarding decoded wireless packets into real-time drive and mode commands.
* **Analog Joystick Transmitter**: Upgrade the handheld controller with a 2-axis analog joystick to provide smooth, continuous proportional throttle and steering rather than discrete button presses.

### Phase 2: IMU Integration & Dynamic Heading Stabilization
* Integrate a 6-DOF / 9-DOF **Inertial Measurement Unit (IMU)** via I2C/SPI on the STM32.
* Implement closed-loop attitude estimation and yaw-rate compensation to maintain straight-line tracking, prevent drift, and detect chassis tilt over uneven terrain.

### Phase 3: Time-of-Flight (ToF) Collision Detection
* Integrate forward-facing **Time-of-Flight (ToF) distance sensors** (e.g., VL53L0X / VL53L1X).
* Implement real-time proximity sensing with dynamic speed reduction and autonomous emergency braking (AEB) to avoid obstacles during both manual and autonomous line-following modes.

## Current Status Summary

### Completed
- [x] Bidirectional DC motor control with TB6612FNG & PWM
- [x] 8-Channel reflectance array acquisition via MCP3208 SPI ADC
- [x] Autonomous line-following navigation with PID control
- [x] 3D-printed chassis assembly and mechanical integration
- [x] Servo-actuated steering & suspension control (45°–135° limits)
- [x] Multi-subsystem UART diagnostic & control dashboard
- [x] Dual ESP32-S3 ESP-NOW wireless link with 8-bit command packet protocol
- [x] Handheld wireless button controller prototype with serial debugging

### In Progress
- [x] Direct UART communication bridge between receiver ESP32-S3 and STM32G431KB
- [x] Analog 2-axis joystick integration on wireless transmitter & 240x320 LCD telemetry dashboard

### Planned
- [ ] IMU sensor integration & closed-loop heading stabilization
- [ ] Time-of-Flight (ToF) sensor integration for forward collision avoidance
- [ ] Follow-me feature using the FireBeetle 2 Board ESP32-S3 (N16R8) AIoT Microcontroller with Camera.
- [ ] Return to Home Feature using IMUs (Adafruit LSM6DS3TR-C 6-DoF Accel + Gyro IMU & Adafruit MPU-6050 6-DoF Accel and Gyro Sensor)

