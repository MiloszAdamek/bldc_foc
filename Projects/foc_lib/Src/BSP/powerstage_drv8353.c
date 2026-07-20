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
#include "stm32g4xx_hal.h"
#include "spi.h"
#include "tim.h"
#include "stdio.h"

static DRV8353_HandleTypeDef g_drv;

static PowerStage_Pins_t ps_pins =
{
    .IN_H_A = {IN_H_A_GPIO_Port, IN_H_A_Pin, GPIO_AF6_TIM1},
    .IN_H_B = {IN_H_B_GPIO_Port, IN_H_B_Pin, GPIO_AF6_TIM1},
    .IN_H_C = {IN_H_C_GPIO_Port, IN_H_C_Pin, GPIO_AF6_TIM1},

    .IN_L_A = {IN_L_A_GPIO_Port, IN_L_A_Pin, GPIO_AF6_TIM1},
    .IN_L_B = {IN_L_B_GPIO_Port, IN_L_B_Pin, GPIO_AF6_TIM1},
    .IN_L_C = {IN_L_C_GPIO_Port, IN_L_C_Pin, GPIO_AF6_TIM1},
};

static void PowerStage_SetPinsToPWM(PowerStage_Pins_t *pins);
static void PowerStage_SetPinsToGPIO(PowerStage_Pins_t *pins);
static void PowerStage_DisableAllHalfBridges(void);
static void PowerStage_TestPWM(float duty_a, float duty_b, float duty_c);

PowerStage_Status_t PowerStage_Init(void)
{
    DRV8353_Init(&g_drv, &hspi2, &htim1);
    DRV8353_SetOutputState(&g_drv, DRV_OUTPUT_RUN);
    // PowerStage_Off();
    return POWERSTAGE_OK;
}

void PowerStage_Off(void)
{
    DRV8353_SetOutputState(&g_drv, DRV_OUTPUT_COAST);
    PowerStage_StopPWM(&htim1);
    PowerStage_SetPinsToGPIO(&ps_pins);
    PowerStage_DisableAllHalfBridges();
}

void PowerStage_On(void)
{
    // DRV8353_SetOutputState(&g_drv, DRV_OUTPUT_RUN);
    // PowerStage_SetPinsToPWM(&ps_pins);
    // DRV8353_PWMEnable(&g_drv);
}

void PowerStage_StartPWM(TIM_HandleTypeDef *htim)
{
    __HAL_TIM_SET_COUNTER(htim, 0);
    HAL_TIM_Base_Start(htim);

    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
}

void PowerStage_StopPWM(TIM_HandleTypeDef *htim)
{
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
}

void PowerStage_SetPWMMode(PowerStage_PWM_Mode_t pwm_mode){
    
    if (pwm_mode == POWERSTAGE_PWM_MODE_3PWM) {
        DRV8353_SetPWMMode(&g_drv, DRV8353_PWM_MODE_3PWM);
    } else if (pwm_mode == POWERSTAGE_PWM_MODE_6PWM) {
        DRV8353_SetPWMMode(&g_drv, DRV8353_PWM_MODE_6PWM);
    }
}

void PowerStage_CheckFaults(void)
{
    DRV8353_GetFaults(&g_drv, &g_drv.faults);
    DRV8353_PrintFaults(&g_drv.faults);
}

void PowerStage_Tests(void)
{
    DRV8353_SetOutputState(&g_drv, DRV_OUTPUT_RUN);
    // DRV8353_PWMDisable(&g_drv);

    // // Test 1: Force all low side MOSFETs ON for a short duration to charge bootstrap capacitors
    // Force_AllHalfBridges(POWERSTATE_ALL_HIGH);

    // Test 2: PWM duty cycle test
    PowerStage_TestPWM(0.4f, 0.5f, 0.6f); 
}

static void PowerStage_DisableAllHalfBridges(void)
{
    HAL_GPIO_WritePin(IN_H_A_GPIO_Port, IN_H_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN_H_B_GPIO_Port, IN_H_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN_H_C_GPIO_Port, IN_H_C_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(IN_L_A_GPIO_Port, IN_L_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN_L_B_GPIO_Port, IN_L_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN_L_C_GPIO_Port, IN_L_C_Pin, GPIO_PIN_RESET);
}

static void PowerStage_SetPinsToGPIO(PowerStage_Pins_t *pins)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    PowerPin_t *all[] =
    {
        &pins->IN_H_A, &pins->IN_H_B, &pins->IN_H_C,
        &pins->IN_L_A, &pins->IN_L_B, &pins->IN_L_C
    };

    for (int i = 0; i < 6; i++)
    {
        GPIO_InitStruct.Pin = all[i]->pin;
        HAL_GPIO_Init(all[i]->port, &GPIO_InitStruct);
    }
}

static void PowerStage_SetPinsToPWM(PowerStage_Pins_t *pins)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    PowerPin_t *all[] =
    {
        &pins->IN_H_A, &pins->IN_H_B, &pins->IN_H_C,
        &pins->IN_L_A, &pins->IN_L_B, &pins->IN_L_C
    };

    for (int i = 0; i < 6; i++)
    {
        GPIO_InitStruct.Pin       = all[i]->pin;
        GPIO_InitStruct.Alternate = all[i]->alternate;
        HAL_GPIO_Init(all[i]->port, &GPIO_InitStruct);
    }
}

void Force_AllHalfBridges(PowerTestState_t state)
{
    PowerStage_StopPWM(&htim1);

    HAL_Delay(1);

    PowerStage_SetPinsToGPIO(&ps_pins);

    // Turn off all half-bridges to ensure a safe starting point
    PowerStage_DisableAllHalfBridges();

    HAL_Delay(1);  // deadtime bezpieczeństwa

    // Set state
    if (state == POWERSTATE_ALL_LOW)
    {
        HAL_GPIO_WritePin(IN_L_A_GPIO_Port, IN_L_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_L_B_GPIO_Port, IN_L_B_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_L_C_GPIO_Port, IN_L_C_Pin, GPIO_PIN_SET);
    }
    else if (state == POWERSTATE_ALL_HIGH)
    {
        // Bootstrap capacitors need to be charged first, so we first turn on the low side MOSFETs for a short time

        HAL_GPIO_WritePin(IN_L_A_GPIO_Port, IN_L_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_L_B_GPIO_Port, IN_L_B_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_L_C_GPIO_Port, IN_L_C_Pin, GPIO_PIN_SET);

        HAL_Delay(2);

        HAL_GPIO_WritePin(IN_L_A_GPIO_Port, IN_L_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(IN_L_B_GPIO_Port, IN_L_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(IN_L_C_GPIO_Port, IN_L_C_Pin, GPIO_PIN_RESET);

        HAL_Delay(1);  // deadtime

        HAL_GPIO_WritePin(IN_H_A_GPIO_Port, IN_H_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_H_B_GPIO_Port, IN_H_B_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN_H_C_GPIO_Port, IN_H_C_Pin, GPIO_PIN_SET);
    }

    printf("All half-bridges forced to %s state.\n", (state == POWERSTATE_ALL_LOW) ? "LOW" : "HIGH");
}

void PowerStage_TestPWM(float duty_a, float duty_b, float duty_c)
{
    // Ograniczenie zakresu 0.0–1.0

    if (duty_a < 0.0f) duty_a = 0.0f;
    if (duty_a > 1.0f) duty_a = 1.0f;

    if (duty_b < 0.0f) duty_b = 0.0f;
    if (duty_b > 1.0f) duty_b = 1.0f;

    if (duty_c < 0.0f) duty_c = 0.0f;
    if (duty_c > 1.0f) duty_c = 1.0f;

    // PowerStage_SetPinsToPWM(&ps_pins);

    DRV8353_SetOutputState(&g_drv, DRV_OUTPUT_RUN);
    
    uint32_t ccr1 = (uint32_t)(duty_a * PWM_PERIOD_ARR);
    uint32_t ccr2 = (uint32_t)(duty_b * PWM_PERIOD_ARR);
    uint32_t ccr3 = (uint32_t)(duty_c * PWM_PERIOD_ARR);

    // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr1);
    // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr2);
    // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, ccr3);

    PowerStage_StartPWM(&htim1);

    printf("PWM Test: Duty A: %.2f, Duty B: %.2f, Duty C: %.2f\n", duty_a, duty_b, duty_c);
}

#endif
