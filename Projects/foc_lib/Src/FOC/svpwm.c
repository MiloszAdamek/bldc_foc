/*
 * svpwm.c
 *
 *  Created on: Mar 30, 2025
 *      Author: Milosz Adamek
 */

#include <FOC/lut_sincos.h>
#include "App/config.h"
#include "FOC/svpwm.h"
#include "FOC/foc_utils.h"
#include "BSP/powerstage.h"
#include "math.h"
#include "main.h"

#define _PI_3 (M_PI / 3.0f)

static TIM_HandleTypeDef* svpwm_htim;

const float TEST_VOLTAGE_AMPLITUDE = 3.0f;
float theta = 0.0f;

void SVPWM_Init(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        while(1);
    }

    svpwm_htim = htim;

    // PowerStage_On();
    // PowerStage_StartPWM(svpwm_htim);
}

void SVPWM_GetDutyCycles(float Ualpha, float Ubeta,
                         float *dc_a, float *dc_b, float *dc_c)
{
    float Ta, Tb, Tc; // Czasy włączenia faz (w tickach timera)

    float Uref = sqrtf(Ualpha * Ualpha + Ubeta * Ubeta);
    float Umax = VOLTAGE_SUPPLY / M_SQRT3;

    if (Uref > Umax) {
        float scale = Umax / Uref;
        Ualpha *= scale;
        Ubeta  *= scale;
        Uref = Umax;
    }
    // Obliczenie kąta i sektora
    float angle = atan2f(Ubeta, Ualpha);
    if (angle < 0) angle += M_TWOPI;
    // Sektor od 0 do 5, co odpowiada sektorom 1-6
    int sector = (int)(angle / _PI_3);
    if (sector >= 6) sector = 5;
    // Kąt wewnątrz bieżącego sektora
    float angle_in_sector = angle - (float)sector * _PI_3;

    // Obliczenie czasów T1, T2 (w sekundach)
    // T1 i T2 to czasy trwania sąsiadujących wektorów bazowych.
    // Obliczenie czasów włączenia dla każdej fazy (w sekundach)
    // T0 to czas, przez który używane są wektory zerowe (gdy wszystkie tranzystory
    // są w tym samym stanie). Rozdzielamy go symetrycznie.
    float k = (M_SQRT3 * PWM_PERIOD_SEC) / VOLTAGE_SUPPLY;
    float T1 = Uref * LUT_Sin(_PI_3 - angle_in_sector) * k;
    float T2 = Uref * LUT_Sin(angle_in_sector) * k;
    float T0 = PWM_PERIOD_SEC - T1 - T2;

    switch (sector) {
        case 0: Ta = T1 + T2 + T0/2; Tb = T2 + T0/2;      Tc = T0/2;           break; // Sektor 1 (wektory V1, V2)
        case 1: Ta = T1 + T0/2;      Tb = T1 + T2 + T0/2; Tc = T0/2;           break; // Sektor 2 (wektory V2, V3)
        case 2: Ta = T0/2;           Tb = T1 + T2 + T0/2; Tc = T2 + T0/2;      break; // Sektor 3 (wektory V3, V4)
        case 3: Ta = T0/2;           Tb = T1 + T0/2;      Tc = T1 + T2 + T0/2; break; // Sektor 4 (wektory V4, V5)
        case 4: Ta = T2 + T0/2;      Tb = T0/2;           Tc = T1 + T2 + T0/2; break; // Sektor 5 (wektory V5, V6)
        case 5: Ta = T1 + T2 + T0/2; Tb = T0/2;           Tc = T1 + T0/2;      break; // Sektor 6 (wektory V6, V1)
        default: Ta = Tb = Tc = PWM_PERIOD_SEC / 2.0f;   					   break; // Nie powinno sie zdarzyć
    }

    // Przeskalowanie [sekundy] → [ticki timera]
    //  Mapowanie na PWM: Ostateczne czasy włączenia dla każdej fazy (Ta, Tb, Tc)
    //  są w zakresie od 0 do PWM_PERIOD_SEC (okres PWM w sekundach). Dzielimy je przez PWM_PERIOD_SEC,
    //	aby uzyskać współczynnik wypełnienia od 0.0 do 1.0, a następnie mnożymy przez PWM_PERIOD,
    //	aby uzyskać wartość do wpisania do rejestru compare timera.

#ifdef CALIB_SVPWM
		*dc_a = Ta / PWM_PERIOD_SEC;
		*dc_b = Tc / PWM_PERIOD_SEC;
		*dc_c = Tb / PWM_PERIOD_SEC;
#else
        *dc_a = Ta / PWM_PERIOD_SEC;
        *dc_b = Tb / PWM_PERIOD_SEC;
        *dc_c = Tc / PWM_PERIOD_SEC;
#endif

}

void SVPWM_Update(float Ualpha, float Ubeta)
{
    float dc_a, dc_b, dc_c;
    SVPWM_GetDutyCycles(Ualpha, Ubeta, &dc_a, &dc_b, &dc_c);

    uint32_t ccr1 = (uint32_t)(dc_a * PWM_PERIOD_ARR);
    uint32_t ccr2 = (uint32_t)(dc_b * PWM_PERIOD_ARR);
    uint32_t ccr3 = (uint32_t)(dc_c * PWM_PERIOD_ARR);

    if (ccr1 > PWM_PERIOD_ARR) ccr1 = PWM_PERIOD_ARR;
    if (ccr2 > PWM_PERIOD_ARR) ccr2 = PWM_PERIOD_ARR;
    if (ccr3 > PWM_PERIOD_ARR) ccr3 = PWM_PERIOD_ARR;

	__HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_1, ccr1);
	__HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_2, ccr2);
	__HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_3, ccr3);
}
