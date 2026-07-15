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
#define Motor_BIN2_Pin GPIO_PIN_0
#define Motor_BIN2_GPIO_Port GPIOA
#define Motor_BIN1_Pin GPIO_PIN_1
#define Motor_BIN1_GPIO_Port GPIOA
#define Motor_STBY_Pin GPIO_PIN_4
#define Motor_STBY_GPIO_Port GPIOA
#define Motor_AIN2_Pin GPIO_PIN_5
#define Motor_AIN2_GPIO_Port GPIOA
#define Motor_AIN1_Pin GPIO_PIN_6
#define Motor_AIN1_GPIO_Port GPIOA
#define PWMA_Pin GPIO_PIN_7
#define PWMA_GPIO_Port GPIOA
#define Encoder_Phase_B2_Pin GPIO_PIN_8
#define Encoder_Phase_B2_GPIO_Port GPIOA
#define Encoder_Phase_A2_Pin GPIO_PIN_9
#define Encoder_Phase_A2_GPIO_Port GPIOA
#define PWMB_Pin GPIO_PIN_10
#define PWMB_GPIO_Port GPIOA
#define Encoder_Phase_A1_Pin GPIO_PIN_11
#define Encoder_Phase_A1_GPIO_Port GPIOA
#define Encoder_Phase_B1_Pin GPIO_PIN_12
#define Encoder_Phase_B1_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define ADC_CS_Pin GPIO_PIN_6
#define ADC_CS_GPIO_Port GPIOB
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
