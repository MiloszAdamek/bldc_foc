/*
 * powerstage_drv8353.c
 *
 *  Created on: Jul 18, 2026
 *      Author: Milosz Adamek
 */
    
#include "App/config.h"
#include "main.h"

#ifdef DRV8353

#include "BSP/powerstage.h"
#include "BSP/drv8353.h"
#include "stm32g4xx_hal.h"
#include "spi.h"
#include "tim.h"
#include "stdio.h"

static PowerStage_Status_t PowerStage_TestPWM(PowerStage_HandleTypeDef *ps,float duty_a, float duty_b, float duty_c);

PowerStage_Status_t PowerStage_Init(
    PowerStage_HandleTypeDef *ps,
    SPI_HandleTypeDef *hspi,
    TIM_HandleTypeDef *htim,
    const PowerStage_Pins_t *pins)
{
    ps->htim = htim;
    ps->pins = *pins;
    ps->status = POWERSTAGE_OK;

    if (DRV8353_Init(&ps->drv, hspi, htim, ps->pwm_mode) != DRV8353_OK)
    {
        ps->status = POWERSTAGE_ERROR;
        return POWERSTAGE_ERROR;
    }

    return POWERSTAGE_OK;
}

PowerStage_Status_t PowerStage_Off(PowerStage_HandleTypeDef *ps)
{
    __HAL_TIM_MOE_DISABLE(ps->htim);

    __HAL_TIM_SET_COMPARE(ps->htim, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(ps->htim, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(ps->htim, TIM_CHANNEL_3, 0);

    DRV8353_SetOutputState(&ps->drv, DRV_OUTPUT_COAST);

    return POWERSTAGE_OK;
}

PowerStage_Status_t PowerStage_On(PowerStage_HandleTypeDef *ps)
{
    __HAL_TIM_MOE_ENABLE(ps->htim);

    DRV8353_SetOutputState(&ps->drv, DRV_OUTPUT_RUN);

    return POWERSTAGE_OK;
}

PowerStage_Status_t PowerStage_CheckFaults(PowerStage_HandleTypeDef *ps)
{
    DRV8353_GetFaults(&ps->drv, &ps->drv.faults);
    DRV8353_PrintFaults(&ps->drv.faults);

    return POWERSTAGE_OK;
}

PowerStage_Status_t PowerStage_Tests(PowerStage_HandleTypeDef *ps)
{
    DRV8353_SetOutputState(&ps->drv, DRV_OUTPUT_RUN);
    // DRV8353_PWMDisable(&ps->drv);

    // // Test 1: Force all low side MOSFETs ON for a short duration to charge bootstrap capacitors
    // Force_AllHalfBridges(POWERSTATE_ALL_HIGH);

    // Test 2: PWM duty cycle test
    PowerStage_TestPWM(ps,  0.4f, 0.5f, 0.6f);

    return POWERSTAGE_OK;
}

PowerStage_Status_t PowerStage_SetCalibrationMode(PowerStage_HandleTypeDef *ps, bool enable)
{
    DRV8353_SetCalibrationMode(&ps->drv, enable);
    return POWERSTAGE_OK;
}

// PowerStage_Status_t Force_AllHalfBridges(PowerStage_HandleTypeDef *ps, PowerTestState_t state)
// {
//     PowerStage_StopPWM(ps);

//     HAL_Delay(1);

//     PowerStage_SetPinsToGPIO(ps);

//     // Turn off all half-bridges to ensure a safe starting point
//     PowerStage_DisableAllHalfBridges(ps);

//     HAL_Delay(1);  // deadtime bezpieczeństwa

//     // Set state
//     if (state == POWERSTATE_ALL_LOW)
//     {
//         HAL_GPIO_WritePin(ps->pins.IN_L_A.port, ps->pins.IN_L_A.pin, GPIO_PIN_SET);
//         HAL_GPIO_WritePin(ps->pins.IN_L_B.port, ps->pins.IN_L_B.pin, GPIO_PIN_SET);
//         HAL_GPIO_WritePin(ps->pins.IN_L_C.port, ps->pins.IN_L_C.pin, GPIO_PIN_SET);
//     }
//     else if (state == POWERSTATE_ALL_HIGH)
//     {
//         // Bootstrap capacitors need to be charged first, so we first turn on the low side MOSFETs for a short time

//         HAL_GPIO_WritePin(ps->pins.IN_L_A.port, ps->pins.IN_L_A.pin, GPIO_PIN_SET);
//         HAL_GPIO_WritePin(ps->pins.IN_L_B.port, ps->pins.IN_L_B.pin, GPIO_PIN_SET);
//         HAL_GPIO_WritePin(ps->pins.IN_L_C.port, ps->pins.IN_L_C.pin, GPIO_PIN_SET);

//         HAL_Delay(2);

//         HAL_GPIO_WritePin(ps->pins.IN_L_A.port, ps->pins.IN_L_A.pin, GPIO_PIN_RESET);
//         HAL_GPIO_WritePin(ps->pins.IN_L_B.port, ps->pins.IN_L_B.pin, GPIO_PIN_RESET);
//         HAL_GPIO_WritePin(ps->pins.IN_L_C.port, ps->pins.IN_L_C.pin, GPIO_PIN_RESET);

//         HAL_Delay(1);  // deadtime

//         HAL_GPIO_WritePin(ps->pins.IN_H_A.port, ps->pins.IN_H_A.pin, GPIO_PIN_SET);
//         HAL_GPIO_WritePin(ps->pins.IN_H_B.port, ps->pins.IN_H_B.pin, GPIO_PIN_SET);
//         HAL_GPIO_WritePin(ps->pins.IN_H_C.port, ps->pins.IN_H_C.pin, GPIO_PIN_SET);
//     }

//     printf("All half-bridges forced to %s state.\n", (state == POWERSTATE_ALL_LOW) ? "LOW" : "HIGH");
//     return POWERSTAGE_OK;
// }

// PowerStage_Status_t PowerStage_TestPWM(PowerStage_HandleTypeDef *ps, float duty_a, float duty_b, float duty_c)
// {
//     // Ograniczenie zakresu 0.0–1.0

//     if (duty_a < 0.0f) duty_a = 0.0f;
//     if (duty_a > 1.0f) duty_a = 1.0f;

//     if (duty_b < 0.0f) duty_b = 0.0f;
//     if (duty_b > 1.0f) duty_b = 1.0f;

//     if (duty_c < 0.0f) duty_c = 0.0f;
//     if (duty_c > 1.0f) duty_c = 1.0f;

//     // PowerStage_SetPinsToPWM(&ps_pins);

//     DRV8353_SetOutputState(&ps->drv, DRV_OUTPUT_RUN);
    
//     uint32_t ccr1 = (uint32_t)(duty_a * PWM_PERIOD_ARR);
//     uint32_t ccr2 = (uint32_t)(duty_b * PWM_PERIOD_ARR);
//     uint32_t ccr3 = (uint32_t)(duty_c * PWM_PERIOD_ARR);

//     // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr1);
//     // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr2);
//     // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, ccr3);

//     PowerStage_StartPWM(ps);

//     printf("PWM Test: Duty A: %.2f, Duty B: %.2f, Duty C: %.2f\n", duty_a, duty_b, duty_c);

//     return POWERSTAGE_OK;
// }

#endif
