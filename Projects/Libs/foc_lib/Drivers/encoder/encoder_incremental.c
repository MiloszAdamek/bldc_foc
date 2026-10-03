/*
 * encoder_incremental.c
 *
 *  Created on: Oct 3, 2026
 *      Author: Milosz Adamek
 */

#include "main.h"
#include "encoder_hub.h"
#include "math_consts.h"
#include "encoder_incremental.h"

#define ENCODER_INCREMENTAL_CPR 2048 // Liczba impulsów na obrót

static TIM_HandleTypeDef* s_htim; // Wskaźnik do struktury timera enkodera inkrementalnego

void EncoderIncremental_Init(TIM_HandleTypeDef* htim)
{
    s_htim = htim;
    HAL_TIM_Encoder_Start(s_htim, TIM_CHANNEL_ALL);
}

float EncoderIncremental_CountToAngle(int32_t count)
{
    // Zakładamy, że enkoder inkrementalny ma 2048 impulsów na obrót (CPR = 2048)
    const int32_t CPR = ENCODER_INCREMENTAL_CPR;
    float angle_rad = ((float)count / (float)CPR) * M_TWOPI; // Przeliczamy na radiany
    return angle_rad;
}

void EncoderIncremental_Update(void)
{
    int32_t count = (int32_t)__HAL_TIM_GET_COUNTER(&htim4);

    EncoderSample_t sample = {
        .raw = count,
        .theta_mech = EncoderIncremental_CountToAngle(count),
        .tick = DWT->CYCCNT,
        .valid = true
    };

    EncoderHub_PublishSample(&sample);
}

float EncoderIncremental_GetMechanicalAngle(void)
{
    return EncoderIncremental_CountToAngle((int32_t)__HAL_TIM_GET_COUNTER(&htim4));
}