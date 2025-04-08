/*
 * encoder.c
 *
 *  Created on: Apr 2, 2025
 *      Author: Miloush
 */

#include "encoder.h"

static volatile uint32_t pulse_width = 0;
static volatile float angle_deg = 0;

void Encoder_Init(TIM_HandleTypeDef *htim) {
    HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1);
}

void Encoder_ProcessCapture(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        static uint32_t rise = 0, fall = 0;
        static uint8_t state = 0;

        if (state == 0)
        {
            rise = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
            state = 1;
        }
        else
        {
            fall = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            uint32_t ticks = (fall >= rise)
                ? (fall - rise)
                : ((htim->Init.Period - rise) + fall);

            pulse_width = ticks;

            float pulse_us = ticks * 0.25f;
            float angle_us = pulse_us - 6.0f;
            if (angle_us < 0) angle_us = 0;

            angle_deg = (angle_us / 996.0f) * 360.0f;

            printf("Ticks: %lu, Angle: %.2f\r\n", pulse_width, angle_deg);

            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
            state = 0;
        }
    }
}

float Encoder_GetAngle(void) {
    return angle_deg;
}


