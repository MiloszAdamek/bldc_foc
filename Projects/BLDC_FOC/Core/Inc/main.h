/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#define RCC_OSC_IN_Pin GPIO_PIN_0
#define RCC_OSC_IN_GPIO_Port GPIOF
#define RCC_OSC_OUT_Pin GPIO_PIN_1
#define RCC_OSC_OUT_GPIO_Port GPIOF
#define CURR_AMPL_U_Pin GPIO_PIN_1
#define CURR_AMPL_U_GPIO_Port GPIOA
#define CURR_AMPL_W_Pin GPIO_PIN_0
#define CURR_AMPL_W_GPIO_Port GPIOB
#define CURR_AMPL_V_Pin GPIO_PIN_1
#define CURR_AMPL_V_GPIO_Port GPIOB
#define PWM_EN_FAULT_Pin GPIO_PIN_12
#define PWM_EN_FAULT_GPIO_Port GPIOB
#define PWM_EN_U_Pin GPIO_PIN_13
#define PWM_EN_U_GPIO_Port GPIOB
#define PWM_EN_V_Pin GPIO_PIN_14
#define PWM_EN_V_GPIO_Port GPIOB
#define PWM_EN_W_Pin GPIO_PIN_15
#define PWM_EN_W_GPIO_Port GPIOB
#define PWM_UH_Pin GPIO_PIN_8
#define PWM_UH_GPIO_Port GPIOA
#define PWM_VH_Pin GPIO_PIN_9
#define PWM_VH_GPIO_Port GPIOA
#define PWM_WH_Pin GPIO_PIN_10
#define PWM_WH_GPIO_Port GPIOA
#define SPI3_CS_Pin GPIO_PIN_2
#define SPI3_CS_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
