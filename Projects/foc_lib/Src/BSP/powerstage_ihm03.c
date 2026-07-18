/*
 * powerstage_ihm03.c
 *
 *  Created on: Jul 18, 2026
 *      Author: Milosz Adamek
 */

 #include "App/config.h"

 #ifdef IHM03

 #include "gpio.h"
 #include "powerstage.h"

 void PowerStage_Init(void)
{
    // brak SPI, tylko GPIO
}

void PowerStage_On(void)
{
    #ifdef MODE_3PWM
        HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_SET);
    #endif
}

void PowerStage_Off(void)
{
    #ifdef MODE_3PWM
        HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port,     PWM_EN_W_Pin,     GPIO_PIN_RESET);
        HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port,     PWM_EN_V_Pin,     GPIO_PIN_RESET);
        HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port,     PWM_EN_U_Pin,     GPIO_PIN_RESET);
    #endif
}

void PowerStage_StartPWM(TIM_HandleTypeDef *htim)
{
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
}

void PowerStage_StopPWM(TIM_HandleTypeDef *htim)
{
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_3);
}

#endif
