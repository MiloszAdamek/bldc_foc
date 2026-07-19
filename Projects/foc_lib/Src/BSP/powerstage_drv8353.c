/*
 * powerstage_drv8353.c
 *
 *  Created on: Jul 18, 2026
 *      Author: Milosz Adamek
 */
    
#include "App/config.h"

#ifdef DRV8353

#include "BSP/powerstage.h"
#include "BSP/drv8353.h"
#include "spi.h"
#include "tim.h"
#include "stm32g4xx_hal.h"

static DRV8353_HandleTypeDef g_drv;

void PowerStage_Init(void)
{
    DRV8353_Init(&g_drv, &hspi2, &htim1);
    // PowerStage_Off();
    PowerStage_Tests();
}

void PowerStage_Off(void)
{
    DRV8353_SetOutputState(&g_drv, DRV_OUTPUT_COAST);
    DRV8353_PWMDisable(&g_drv);
}

void PowerStage_On(void)
{
    
}

void PowerStage_StartPWM(TIM_HandleTypeDef *htim)
{
    // start timera
}

void PowerStage_StopPWM(TIM_HandleTypeDef *htim)
{
    DRV8353_PWMDisable(&g_drv);
}

void PowerStage_CheckFaults(void)
{
    DRV8353_GetFaults(&g_drv, &g_drv.faults);
    DRV8353_PrintFaults(&g_drv.faults);
}

void PowerStage_Tests(void)
{
    DRV8353_SetOutputState(&g_drv, DRV_OUTPUT_RUN); // Enable low side MOSFETs to brake the motor
    DRV8353_PWMDisable(&g_drv);


    // Test: Force all low side MOSFETs ON for a short duration to charge bootstrap capacitors
    Force_AllHalfBridges(POWERSTATE_ALL_LOW);
    HAL_Delay(5);   // pozwól bootstrapowi się naładować
    Force_AllHalfBridges(POWERSTATE_ALL_HIGH);

}

void Force_AllHalfBridges(PowerTestState_t state)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Zatrzymaj PWM */
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);

    HAL_Delay(1);

    /* Przełącz wszystkie IN_H_x i IN_L_x na GPIO */
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /* High side pins */
    GPIO_InitStruct.Pin = IN_H_A_Pin;
    HAL_GPIO_Init(IN_H_A_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = IN_H_B_Pin;
    HAL_GPIO_Init(IN_H_B_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = IN_H_C_Pin;
    HAL_GPIO_Init(IN_H_C_GPIO_Port, &GPIO_InitStruct);

    /* Low side pins */
    GPIO_InitStruct.Pin = IN_L_A_Pin;
    HAL_GPIO_Init(IN_L_A_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = IN_L_B_Pin;
    HAL_GPIO_Init(IN_L_B_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = IN_L_C_Pin;
    HAL_GPIO_Init(IN_L_C_GPIO_Port, &GPIO_InitStruct);

    /* Najpierw wyłącz wszystko (bezpieczny stan) */
    HAL_GPIO_WritePin(IN_H_A_GPIO_Port, IN_H_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN_H_B_GPIO_Port, IN_H_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN_H_C_GPIO_Port, IN_H_C_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(IN_L_A_GPIO_Port, IN_L_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN_L_B_GPIO_Port, IN_L_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN_L_C_GPIO_Port, IN_L_C_Pin, GPIO_PIN_RESET);

    HAL_Delay(1);  // deadtime bezpieczeństwa

    /* Ustaw docelowy stan */
    if (state == POWERSTATE_ALL_LOW)
    {
        HAL_GPIO_WritePin(IN_L_A_GPIO_Port, IN_L_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_L_B_GPIO_Port, IN_L_B_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_L_C_GPIO_Port, IN_L_C_Pin, GPIO_PIN_SET);
    }
    else if (state == POWERSTATE_ALL_HIGH)
    {
        HAL_GPIO_WritePin(IN_H_A_GPIO_Port, IN_H_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_H_B_GPIO_Port, IN_H_B_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_H_C_GPIO_Port, IN_H_C_Pin, GPIO_PIN_SET);
    }
}

#endif
