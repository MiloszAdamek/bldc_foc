/*
 * svpwm.c
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#include "svpwm.h"
#include "math.h"
#include "main.h"

#define SQRT3      1.73205080757f
#define PWM_PERIOD 1700.0f  // Dopasuj do ustawień TIM1

extern TIM_HandleTypeDef htim1;

void SVPWM_Update(float Valpha, float Vbeta) {
    float Uref = sqrtf(Valpha * Valpha + Vbeta * Vbeta);
    float angle = atan2f(Vbeta, Valpha);

    float pi_over_3 = M_PI / 3.0f;
    int sector = (int)(angle / pi_over_3);
    if (sector < 0) sector += 6;
    sector = (sector % 6) + 1;

    float T = PWM_PERIOD;
    float X = SQRT3 * Uref / T;
    float alpha = fmodf(angle, pi_over_3);
    float T1 = X * sinf(pi_over_3 - alpha) * T;
    float T2 = X * sinf(alpha) * T;
    float T0 = T - T1 - T2;

    float Ta, Tb, Tc;

    switch (sector) {
        case 1:
            Ta = (T1 + T2 + T0) / 2;
            Tb = (T2 + T0) / 2;
            Tc = T0 / 2;
            break;
        case 2:
            Ta = (T1 + T0) / 2;
            Tb = (T1 + T2 + T0) / 2;
            Tc = T0 / 2;
            break;
        case 3:
            Ta = T0 / 2;
            Tb = (T1 + T2 + T0) / 2;
            Tc = (T2 + T0) / 2;
            break;
        case 4:
            Ta = T0 / 2;
            Tb = (T1 + T0) / 2;
            Tc = (T1 + T2 + T0) / 2;
            break;
        case 5:
            Ta = (T2 + T0) / 2;
            Tb = T0 / 2;
            Tc = (T1 + T2 + T0) / 2;
            break;
        case 6:
            Ta = (T1 + T2 + T0) / 2;
            Tb = T0 / 2;
            Tc = (T1 + T0) / 2;
            break;
        default:
            Ta = Tb = Tc = T / 2;
            break;
    }
	printf("Ta: %7.3f | Tb: %7.3f | Tc: %7.3f\r\n", Ta, Tb, Tc);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint16_t)Ta);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint16_t)Tb);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, (uint16_t)Tc);
}

