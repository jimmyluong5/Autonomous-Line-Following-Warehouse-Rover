# Reflectance-Sensing Autonomous Rover
<img width="480" height="853" alt="line_following_github" src="https://github.com/user-attachments/assets/8277d7d8-333a-4ad6-8d29-b165df40b6e0" />




An autonomous line-following rover built around the **STM32G431KB**, using an 8-channel reflectance sensor array for navigation, UART for diagnostics and control, and ESP-NOW for wireless communication.

The project began as a basic motor-control prototype and has gradually evolved into a full rover platform with autonomous navigation, sensor monitoring, multiple operating modes, and wireless control.

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

The first stage of the project focused on getting the drivetrain working.

Two **FIT0484 DC motors** were controlled through a **TB6612FNG dual motor driver**, with the STM32G431KB acting as the main controller.

The power system uses **LM2596 buck converters** to provide regulated voltages to the motors and embedded electronics.

Once reliable bidirectional motor control was established, the drivetrain was mounted onto a rough cardboard chassis for early testing.

<img width="480" height="853" alt="motors_github" src="https://github.com/user-attachments/assets/ca83976e-c8b5-4a69-80d2-1aaa2ccd849a" />




### 2. Cardboard Prototype

A temporary cardboard rover was constructed to test the complete drivetrain before moving to a permanent chassis.

This prototype made it possible to verify

* Motor direction and speed control
* Wheel placement and mechanical alignment
* Breadboard wiring
* Initial sensor placement
* Basic autonomous movement

The prototype successfully demonstrated that the embedded control system and drivetrain could operate together before the mechanical design was finalized.

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

### 6. 3D-Printed Chassis

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


### 7. Servo-Based Suspension Testing

After transferring the rover to the 3D-printed chassis, I tested a **servo-actuated front suspension system** using an SG90 servo.

The servo was used to adjust the front of the chassis and verify that the suspension mechanism could move reliably under control from the STM32.

A short demonstration clip is included below.
<img width="480" height="853" alt="sensor_github" src="https://github.com/user-attachments/assets/9212897a-4fb7-46e2-af77-010e2313b5a3" />


The test helped validate the mechanical design before continuing with wireless control and additional rover integration.


### 8. ESP-NOW Wireless Communication

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

The goal is to expand this system into full wireless rover control.

## Hardware

| Component       | Purpose                                   |
| --------------- | ----------------------------------------- |
| STM32G431KB     | Main rover controller                     |
| QTRX-MD-08A     | 8-channel reflectance sensor array        |
| MCP3208         | External ADC for reflectance measurements |
| FIT0484         | DC drive motors                           |
| TB6612FNG       | Dual DC motor driver                      |
| ESP32-S3 ×2     | Wireless transmitter and receiver         |
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

## Current Status

Completed

* Motor control
* Power regulation
* Cardboard drivetrain prototype
* Reflectance sensor acquisition
* MCP3208 ADC integration
* UART diagnostic interface
* Reflectance sensor calibration
* Autonomous line following
* 3D-printed chassis integration
* Initial ESP-NOW communication

In progress

* Expanding ESP-NOW command functionality
* Wireless rover control
* Integration of additional autonomous behaviors
* Further mechanical and control-system refinement

## Planned Development

The final project is intended to incorporate an FPGA-based control subsystem using a **Terasic DE10-Lite**.

The FPGA portion has not yet been integrated and is therefore not part of the rover's current control architecture.

Future work will include

* FPGA integration
* Additional hardware-accelerated control or peripheral logic
* Improved autonomous navigation
* Expanded wireless command functionality
