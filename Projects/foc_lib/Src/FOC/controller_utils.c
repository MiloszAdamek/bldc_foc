/*
 * controller_utils.c
 *
 *  Created on: Apr 12, 2026
 *      Author: Miloush
 */

#include "FOC/controller_utils.h"

void start_pwm(TIM_HandleTypeDef *htim){
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
}

void stop_pwm(TIM_HandleTypeDef *htim){
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_3);
}

void enable_driver(void){
#ifdef MODE_3PWM
    HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_SET);
#endif

}

void disable_driver(void){
#ifdef MODE_3PWM
    HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port,     PWM_EN_W_Pin,     GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port,     PWM_EN_V_Pin,     GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port,     PWM_EN_U_Pin,     GPIO_PIN_RESET);
#endif
}
