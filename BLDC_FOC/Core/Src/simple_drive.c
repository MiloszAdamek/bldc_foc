/*
 * simple_drive.c
 *
 *  Created on: Apr 5, 2025
 *      Author: Miloush
 */

#include "simple_drive.h"
#include "math.h"
#include "main.h"

#define TWO_PI       6.283185f
#define AMPLITUDE    0.9f              // 0.0–1.0 zakres napięcia (nie przekraczaj 0.95)
#define FREQ_HZ      2.0f               // Częstotliwość obrotów (Hz)
#define PWM_PERIOD   4249              // Dla 20 kHz PWM i TIM clock = 170 MHz

static float angle = 0.0f;
static float angle_step = 0.0f;
static uint32_t last_tick = 0;

void SimpleDrive_Init(float freq_hz)
{
    angle = 0.0f;
    last_tick = HAL_GetTick();
    angle_step = TWO_PI * freq_hz * 0.001f;  // zakładając update co 1ms

    HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_SET);

}

void SimpleDrive_Run(TIM_HandleTypeDef *htim)
{
    if (HAL_GetTick() - last_tick >= 1)
    {
        last_tick = HAL_GetTick();

        // Krok kąta
        angle += angle_step;
        if (angle > TWO_PI) angle -= TWO_PI;

        // Trójfazowa sinusoida przesunięta o 120°
        float va = AMPLITUDE * sinf(angle);
        float vb = AMPLITUDE * sinf(angle - TWO_PI / 3.0f);
        float vc = AMPLITUDE * sinf(angle - 2.0f * TWO_PI / 3.0f);

        // Napięcie -1.0..1.0 → PWM 0..ARR
        uint16_t duty_a = (uint16_t)((va * 0.5f + 0.5f) * PWM_PERIOD);
        uint16_t duty_b = (uint16_t)((vb * 0.5f + 0.5f) * PWM_PERIOD);
        uint16_t duty_c = (uint16_t)((vc * 0.5f + 0.5f) * PWM_PERIOD);

        // Ustawienie wartości PWM na kanałach
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, duty_a);
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, duty_b);
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, duty_c);

//        printf("angle: %.2f | duty_a: %u | duty_b: %u | duty_c: %u\r\n", angle, duty_a, duty_b, duty_c);

    }
}

