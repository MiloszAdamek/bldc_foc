/*
 * svpwm.c
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#include "App/config.h"
#include "FOC/svpwm.h"
#include "math.h"
#include "main.h"

#define _PI_3 (M_PI / 3.0f)

volatile uint16_t debug_Ta = 0.0f;
volatile uint16_t debug_Tb = 0.0f;
volatile uint16_t debug_Tc = 0.0f;

static TIM_HandleTypeDef* svpwm_htim;

const float TEST_VOLTAGE_AMPLITUDE = 3.0f;
float theta = 0.0f;

void SVPWM_Init(TIM_HandleTypeDef *htim) {

    if (htim == NULL) {
        while(1);
    }

    svpwm_htim = htim;

    HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_SET);

    HAL_TIM_PWM_Start(svpwm_htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(svpwm_htim, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(svpwm_htim, TIM_CHANNEL_3);
}

void SVPWM_Update(float Ualpha, float Ubeta) {
    float Ta, Tb, Tc; // Ostateczne czasy włączenia faz (w tickach timera)

    // --- Krok 1: Ograniczenie napięcia ---
    float U_ref = sqrtf(Ualpha * Ualpha + Ubeta * Ubeta);
    if (U_ref > VOLTAGE_SUPPLY / M_SQRT3) {
        float scale = (VOLTAGE_SUPPLY / M_SQRT3) / U_ref;
        Ualpha *= scale;
        Ubeta *= scale;
        U_ref *= scale; // Zaktualizuj też U_ref
    }

    // --- Krok 2: Obliczenie kąta i sektora ---
    float angle = atan2f(Ubeta, Ualpha);
    if (angle < 0) {
        angle += M_TWOPI;
    }
    // Sektor od 0 do 5, co odpowiada sektorom 1-6
    int sector = (int)(angle / _PI_3);
    if (sector >= 6) sector = 5;

    // --- Krok 3: Obliczenie czasów T1, T2 (w sekundach) ---
    // T1 i T2 to czasy trwania sąsiadujących wektorów bazowych.
    float T1, T2;
    // Współczynnik modulacji (0.0 do 1.0)
    float m = M_SQRT3 * U_ref / VOLTAGE_SUPPLY;
    // Kąt wewnątrz bieżącego sektora
    float angle_in_sector = angle - (float)sector * _PI_3;

    T1 = m * sinf(_PI_3 - angle_in_sector) * PWM_PERIOD_SEC;
    T2 = m * sinf(angle_in_sector) * PWM_PERIOD_SEC;

    // --- Krok 4: Obliczenie czasów włączenia dla każdej fazy (w sekundach) ---
    // T0 to czas, przez który używane są wektory zerowe (gdy wszystkie tranzystory
    // są w tym samym stanie). Rozdzielamy go symetrycznie.
    float T0 = PWM_PERIOD_SEC - T1 - T2;

    switch (sector) {
        case 0: // Sektor 1 (wektory V1, V2)
            Ta = T1 + T2 + T0 / 2.0f;
            Tb = T2 + T0 / 2.0f;
            Tc = T0 / 2.0f;
            break;
        case 1: // Sektor 2 (wektory V2, V3)
            Ta = T1 + T0 / 2.0f;
            Tb = T1 + T2 + T0 / 2.0f;
            Tc = T0 / 2.0f;
            break;
        case 2: // Sektor 3 (wektory V3, V4)
            Ta = T0 / 2.0f;
            Tb = T1 + T2 + T0 / 2.0f;
            Tc = T2 + T0 / 2.0f;
            break;
        case 3: // Sektor 4 (wektory V4, V5)
            Ta = T0 / 2.0f;
            Tb = T1 + T0 / 2.0f;
            Tc = T1 + T2 + T0 / 2.0f;
            break;
        case 4: // Sektor 5 (wektory V5, V6)
            Ta = T2 + T0 / 2.0f;
            Tb = T0 / 2.0f;
            Tc = T1 + T2 + T0 / 2.0f;
            break;
        case 5: // Sektor 6 (wektory V6, V1)
            Ta = T1 + T2 + T0 / 2.0f;
            Tb = T0 / 2.0f;
            Tc = T1 + T0 / 2.0f;
            break;
        default: // Powinno się nigdy nie zdarzyć
            Ta = Tb = Tc = PWM_PERIOD_SEC / 2.0f;
            break;
    }
    //  Mapowanie na PWM: Ostateczne czasy włączenia dla każdej fazy (Ta, Tb, Tc)
    //  są w zakresie od 0 do PWM_PERIOD_SEC (okres PWM w sekundach). Dzielimy je przez PWM_PERIOD_SEC,
    //	aby uzyskać współczynnik wypełnienia od 0.0 do 1.0, a następnie mnożymy przez PWM_PERIOD,
    //	aby uzyskać wartość do wpisania do rejestru compare timera.
    // --- Krok 5: Przeskaluj czasy [0, PWM_PERIOD_SEC] na wartości compare [0, PWM_PERIOD_ARR] ---
    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_1, (uint32_t)(Ta / PWM_PERIOD_SEC * PWM_PERIOD_ARR));
    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_2, (uint32_t)(Tb / PWM_PERIOD_SEC * PWM_PERIOD_ARR));
    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_3, (uint32_t)(Tc / PWM_PERIOD_SEC * PWM_PERIOD_ARR));
}

void SVPWM_Test_Run(float test_freq_hz)
{
	theta += M_TWOPI * test_freq_hz * PWM_PERIOD_SEC;
	if (theta > M_TWOPI) theta -= M_TWOPI;

	float Ualpha = TEST_VOLTAGE_AMPLITUDE * cosf(theta);
	float Ubeta = TEST_VOLTAGE_AMPLITUDE * sinf(theta);

	SVPWM_Update(Ualpha, Ubeta);
}


