/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

#include "stm32g4xx_nucleo.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DC_Motor_BIN2_Pin GPIO_PIN_0
#define DC_Motor_BIN2_GPIO_Port GPIOA
#define DC_Motor_BIN1_Pin GPIO_PIN_1
#define DC_Motor_BIN1_GPIO_Port GPIOA
#define DC_Motor_STBY_Pin GPIO_PIN_4
#define DC_Motor_STBY_GPIO_Port GPIOA
#define DC_Motor_AIN2_Pin GPIO_PIN_5
#define DC_Motor_AIN2_GPIO_Port GPIOA
#define DC_Motor_AIN1_Pin GPIO_PIN_6
#define DC_Motor_AIN1_GPIO_Port GPIOA
#define Motor_Driver_PWMA_Pin GPIO_PIN_7
#define Motor_Driver_PWMA_GPIO_Port GPIOA
#define Servo_Suspension_Pin GPIO_PIN_0
#define Servo_Suspension_GPIO_Port GPIOB
#define Stepper_STEP_Pin GPIO_PIN_8
#define Stepper_STEP_GPIO_Port GPIOA
#define Stepper_DIR_Pin GPIO_PIN_9
#define Stepper_DIR_GPIO_Port GPIOA
#define Motor_Driver_PWMB_Pin GPIO_PIN_10
#define Motor_Driver_PWMB_GPIO_Port GPIOA
#define Stepper_ENABLE_Pin GPIO_PIN_11
#define Stepper_ENABLE_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define ADC_SPI_CS_Pin GPIO_PIN_6
#define ADC_SPI_CS_GPIO_Port GPIOB
#define Speaker_Pin GPIO_PIN_7
#define Speaker_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
// Map ADC_CS to the newly generated ADC_SPI_CS pin
#define ADC_CS_Pin ADC_SPI_CS_Pin
#define ADC_CS_GPIO_Port ADC_SPI_CS_GPIO_Port

#define RFID_RST_Pin GPIO_PIN_12
#define RFID_RST_GPIO_Port GPIOA
#define RFID_SPI_CS_Pin GPIO_PIN_15
#define RFID_SPI_CS_GPIO_Port GPIOA
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
