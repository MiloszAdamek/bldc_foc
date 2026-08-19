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
#define V_DC_Pin GPIO_PIN_3
#define V_DC_GPIO_Port GPIOC
#define CurrentA_Pin GPIO_PIN_0
#define CurrentA_GPIO_Port GPIOA
#define CurrentB_Pin GPIO_PIN_1
#define CurrentB_GPIO_Port GPIOA
#define CurrentC_Pin GPIO_PIN_2
#define CurrentC_GPIO_Port GPIOA
#define DRV8353_nFAULT_Pin GPIO_PIN_6
#define DRV8353_nFAULT_GPIO_Port GPIOA
#define IN_L_A_Pin GPIO_PIN_7
#define IN_L_A_GPIO_Port GPIOA
#define IN_L_B_Pin GPIO_PIN_0
#define IN_L_B_GPIO_Port GPIOB
#define IN_L_C_Pin GPIO_PIN_1
#define IN_L_C_GPIO_Port GPIOB
#define DRV8353_CS_Pin GPIO_PIN_12
#define DRV8353_CS_GPIO_Port GPIOB
#define AS5048A_CS_Pin GPIO_PIN_6
#define AS5048A_CS_GPIO_Port GPIOC
#define IN_H_A_Pin GPIO_PIN_8
#define IN_H_A_GPIO_Port GPIOA
#define IN_H_B_Pin GPIO_PIN_9
#define IN_H_B_GPIO_Port GPIOA
#define IN_H_C_Pin GPIO_PIN_10
#define IN_H_C_GPIO_Port GPIOA
#define SPI3_CS_Pin GPIO_PIN_2
#define SPI3_CS_GPIO_Port GPIOD
#define DRV8353_ENABLE_Pin GPIO_PIN_4
#define DRV8353_ENABLE_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
