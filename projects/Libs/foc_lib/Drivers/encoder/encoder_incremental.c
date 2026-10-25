/*
 * encoder_incremental.c
 *
 *  Created on: Oct 3, 2026
 *      Author: Milosz Adamek
 */

#include "math_consts.h"
#include "encoder_incremental.h"

#define ENCODER_INCREMENTAL_PPR 2048 // Liczba impulsów na obrót - specyfikacja AMT102-V
#define ENCODER_INCREMENTAL_CPR (ENCODER_INCREMENTAL_PPR * 4) // Liczba zliczeń na obrót (kwadratura)
#define CNT_TO_RAD (2.0f * M_PI / (float)ENCODER_INCREMENTAL_CPR)

static TIM_HandleTypeDef* s_htim;

void EncoderIncremental_Init(TIM_HandleTypeDef* htim)
{
    s_htim = htim;
    HAL_TIM_Encoder_Start(s_htim, TIM_CHANNEL_ALL);
}

float EncoderIncremental_GetMechanicalAngle(void)
{
    return ((float)__HAL_TIM_GET_COUNTER(s_htim) * CNT_TO_RAD); // Zakres 0 do 2*PI
}

void EncoderIncremental_SetZero()
{
    __HAL_TIM_SET_COUNTER(s_htim, 0);
}