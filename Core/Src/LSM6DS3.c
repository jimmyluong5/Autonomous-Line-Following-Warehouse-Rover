#include "LSM6DS3.h"
#include <string.h>
#include <stdio.h>

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef hcom_uart[];

void LSM6DS3_Init(void) {
    // Ensure CS is high initially (inactive)
    HAL_GPIO_WritePin(LSM6DS3_CS_GPIO_Port, LSM6DS3_CS_Pin, GPIO_PIN_SET);
}

uint8_t LSM6DS3_ReadReg(uint8_t reg) {
    uint8_t tx[2];
    uint8_t rx[2] = {0};

    tx[0] = reg | 0x80;   // bit 7 = 1 for read
    tx[1] = 0x00;

    HAL_GPIO_WritePin(LSM6DS3_CS_GPIO_Port, LSM6DS3_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LSM6DS3_CS_GPIO_Port, LSM6DS3_CS_Pin, GPIO_PIN_SET);

    return rx[1];
}

void LSM6DS3_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t tx[2];

    tx[0] = reg & 0x7F;   // bit 7 = 0 for write
    tx[1] = value;

    HAL_GPIO_WritePin(LSM6DS3_CS_GPIO_Port, LSM6DS3_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, tx, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LSM6DS3_CS_GPIO_Port, LSM6DS3_CS_Pin, GPIO_PIN_SET);
}

uint8_t LSM6DS3_GetWhoAmI(void) {
    return LSM6DS3_ReadReg(LSM6DS3_WHO_AM_I);
}

void LSM6DS3_DumpRegisters(void) {
    char buf[512];
    const char *header = "\r\n========== SPI REGISTERS DUMP (0x00 - 0x3F) ==========\r\n"
                         "Reg: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n"
                         "----------------------------------------------------\r\n";
    HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)header, strlen(header), HAL_MAX_DELAY);

    for (uint8_t row = 0; row < 4; row++) {
        uint8_t start_reg = row * 16;
        int offset = snprintf(buf, sizeof(buf), "%02X: ", start_reg);
        for (uint8_t col = 0; col < 16; col++) {
            uint8_t val = LSM6DS3_ReadReg(start_reg + col);
            offset += snprintf(buf + offset, sizeof(buf) - offset, "%02X ", val);
        }
        snprintf(buf + offset, sizeof(buf) - offset, "\r\n");
        HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)buf, strlen(buf), HAL_MAX_DELAY);
    }

    // Check specific known WHO_AM_I / Chip-ID addresses
    uint8_t id_0f = LSM6DS3_ReadReg(0x0F); // ST standard (LSM6DS3 / LIS3DH)
    uint8_t id_00 = LSM6DS3_ReadReg(0x00); // Bosch (BMI160/270) or ADXL345
    uint8_t id_75 = LSM6DS3_ReadReg(0x75); // InvenSense (MPU6050/6500/9250 / ICM)
    uint8_t id_4f = LSM6DS3_ReadReg(0x4F); // ST Mag (LIS2MDL / LSM303)

    snprintf(buf, sizeof(buf),
             "----------------------------------------------------\r\n"
             "Key ID Addresses:\r\n"
             " Reg 0x0F (ST WHO_AM_I)        : 0x%02X\r\n"
             " Reg 0x75 (InvenSense WHO_AM_I) : 0x%02X\r\n"
             " Reg 0x00 (Bosch/ADXL Chip ID) : 0x%02X\r\n"
             " Reg 0x4F (ST Mag WHO_AM_I)    : 0x%02X\r\n"
             "====================================================\r\n",
             id_0f, id_75, id_00, id_4f);
    HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)buf, strlen(buf), HAL_MAX_DELAY);
}
