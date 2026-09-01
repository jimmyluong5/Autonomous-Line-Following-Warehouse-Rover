#ifndef LSM6DS3_H
#define LSM6DS3_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define LSM6DS3_CS_GPIO_Port GPIOB
#define LSM6DS3_CS_Pin       GPIO_PIN_6

#define LSM6DS3_WHO_AM_I         0x0F
#define LSM6DS3_WHO_AM_I_VAL     0x69  // Classic LSM6DS3
#define LSM6DS3TR_C_WHO_AM_I_VAL 0x6A  // LSM6DS3TR-C / LSM6DSO / LSM6DSM

void LSM6DS3_Init(void);
uint8_t LSM6DS3_ReadReg(uint8_t reg);
void LSM6DS3_WriteReg(uint8_t reg, uint8_t value);
uint8_t LSM6DS3_GetWhoAmI(void);
void LSM6DS3_DumpRegisters(void);

#endif /* LSM6DS3_H */
