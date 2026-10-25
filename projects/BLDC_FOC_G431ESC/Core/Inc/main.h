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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUS_VOLTAGE_Pin GPIO_PIN_0
#define BUS_VOLTAGE_GPIO_Port GPIOA
#define CURR_SHUNT_U_Pin GPIO_PIN_1
#define CURR_SHUNT_U_GPIO_Port GPIOA
#define OPAMP1_OUT_Pin GPIO_PIN_2
#define OPAMP1_OUT_GPIO_Port GPIOA
#define OPAMP1_INT_GAIN_Pin GPIO_PIN_3
#define OPAMP1_INT_GAIN_GPIO_Port GPIOA
#define OPAMP2_INT_GAIN_Pin GPIO_PIN_5
#define OPAMP2_INT_GAIN_GPIO_Port GPIOA
#define OPAMP2_OUT_Pin GPIO_PIN_6
#define OPAMP2_OUT_GPIO_Port GPIOA
#define CURR_SHUNT_V_Pin GPIO_PIN_7
#define CURR_SHUNT_V_GPIO_Port GPIOA
#define CURR_SHUNT_W_Pin GPIO_PIN_0
#define CURR_SHUNT_W_GPIO_Port GPIOB
#define OPAMP3_OUT_Pin GPIO_PIN_1
#define OPAMP3_OUT_GPIO_Port GPIOB
#define OPAMP3_INT_GAIN_Pin GPIO_PIN_2
#define OPAMP3_INT_GAIN_GPIO_Port GPIOB
#define TEMPERATURE_Pin GPIO_PIN_14
#define TEMPERATURE_GPIO_Port GPIOB
#define SPI3_CS_Pin GPIO_PIN_15
#define SPI3_CS_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
