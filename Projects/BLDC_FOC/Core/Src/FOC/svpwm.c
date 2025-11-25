/*
 * svpwm.c
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#include <FOC/lut_sincos.h>
#include "App/config.h"
#include "FOC/svpwm.h"
#include "math.h"
#include "main.h"

#define _PI_3 (M_PI / 3.0f)
#define ONE_OVER_SQRT_3 (1.0f / M_SQRT3)

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

//void SVPWM_Update(float Ualpha, float Ubeta) {
//    float Ta, Tb, Tc; // Czasy włączenia faz (w tickach timera)
//
//    // Krok 1: Ograniczenie napięcia
//    float U_ref = sqrtf(Ualpha * Ualpha + Ubeta * Ubeta);
//    if (U_ref > VOLTAGE_SUPPLY / M_SQRT3) {
//        float scale = (VOLTAGE_SUPPLY / M_SQRT3) / U_ref;
//        Ualpha *= scale;
//        Ubeta *= scale;
//        U_ref *= scale; // Zaktualizuj też U_ref
//    }
//
//    // Krok 2: Obliczenie kąta i sektora
//    float angle = atan2f(Ubeta, Ualpha);
//    if (angle < 0) {
//        angle += M_TWOPI;
//    }
//    // Sektor od 0 do 5, co odpowiada sektorom 1-6
//    int sector = (int)(angle / _PI_3);
//    if (sector >= 6) sector = 5;
//
//    // Krok 3: Obliczenie czasów T1, T2 (w sekundach)
//    // T1 i T2 to czasy trwania sąsiadujących wektorów bazowych.
//    float T1, T2;
//    // Współczynnik modulacji (0.0 do 1.0)
//    float m = M_SQRT3 * U_ref / VOLTAGE_SUPPLY;
//    // Kąt wewnątrz bieżącego sektora
//    float angle_in_sector = angle - (float)sector * _PI_3;
//
//    T1 = m * LUT_Sin(_PI_3 - angle_in_sector) * PWM_PERIOD_SEC;
//    T2 = m * LUT_Sin(angle_in_sector) * PWM_PERIOD_SEC;
//
//    // Krok 4: Obliczenie czasów włączenia dla każdej fazy (w sekundach)
//    // T0 to czas, przez który używane są wektory zerowe (gdy wszystkie tranzystory
//    // są w tym samym stanie). Rozdzielamy go symetrycznie.
//    float T0 = PWM_PERIOD_SEC - T1 - T2;
//
//    switch (sector) {
//        case 0: // Sektor 1 (wektory V1, V2)
//            Ta = T1 + T2 + T0 / 2.0f;
//            Tb = T2 + T0 / 2.0f;
//            Tc = T0 / 2.0f;
//            break;
//        case 1: // Sektor 2 (wektory V2, V3)
//            Ta = T1 + T0 / 2.0f;
//            Tb = T1 + T2 + T0 / 2.0f;
//            Tc = T0 / 2.0f;
//            break;
//        case 2: // Sektor 3 (wektory V3, V4)
//            Ta = T0 / 2.0f;
//            Tb = T1 + T2 + T0 / 2.0f;
//            Tc = T2 + T0 / 2.0f;
//            break;
//        case 3: // Sektor 4 (wektory V4, V5)
//            Ta = T0 / 2.0f;
//            Tb = T1 + T0 / 2.0f;
//            Tc = T1 + T2 + T0 / 2.0f;
//            break;
//        case 4: // Sektor 5 (wektory V5, V6)
//            Ta = T2 + T0 / 2.0f;
//            Tb = T0 / 2.0f;
//            Tc = T1 + T2 + T0 / 2.0f;
//            break;
//        case 5: // Sektor 6 (wektory V6, V1)
//            Ta = T1 + T2 + T0 / 2.0f;
//            Tb = T0 / 2.0f;
//            Tc = T1 + T0 / 2.0f;
//            break;
//        default: // Powinno się nigdy nie zdarzyć
//            Ta = Tb = Tc = PWM_PERIOD_SEC / 2.0f;
//            break;
//    }
//    //  Mapowanie na PWM: Ostateczne czasy włączenia dla każdej fazy (Ta, Tb, Tc)
//    //  są w zakresie od 0 do PWM_PERIOD_SEC (okres PWM w sekundach). Dzielimy je przez PWM_PERIOD_SEC,
//    //	aby uzyskać współczynnik wypełnienia od 0.0 do 1.0, a następnie mnożymy przez PWM_PERIOD,
//    //	aby uzyskać wartość do wpisania do rejestru compare timera.
//    //  Krok 5: Przeskaluj czasy [0, PWM_PERIOD_SEC] na wartości compare [0, PWM_PERIOD_ARR] ---
//    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_1, (uint32_t)(Ta / PWM_PERIOD_SEC * PWM_PERIOD_ARR));
//    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_2, (uint32_t)(Tb / PWM_PERIOD_SEC * PWM_PERIOD_ARR));
//    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_3, (uint32_t)(Tc / PWM_PERIOD_SEC * PWM_PERIOD_ARR));
//}

static inline int SVPWM_GetSector(float Ualpha, float Ubeta)
{
    const float ONE_OVER_SQRT3 = 0.57735026919f; // 1 / √3
    const float SQRT3 = 1.73205080757f;

    int sector;

    if (Ubeta >= 0.0f)
    {
        if (Ualpha >= 0.0f)
        {
            // SECTOR 0 vs 1
            // Granica: Ualpha >= √3 * Ubeta   (≈ 60°)
            if (SQRT3 * Ubeta <= Ualpha)
                sector = 0;
            else
                sector = 1;
        }
        else
        {
            // Ualpha < 0, Ubeta > 0
            // SECTOR 1 vs 2
            if (-SQRT3 * Ubeta <= Ualpha)
                sector = 2;
            else
                sector = 1;
        }
    }
    else  // Ubeta < 0.0
    {
        if (Ualpha < 0.0f)
        {
            // SECTOR 3 vs 4
            if (-SQRT3 * Ubeta >= -Ualpha)
                sector = 3;
            else
                sector = 4;
        }
        else
        {
            // SECTOR 4 vs 5
            if (SQRT3 * (-Ubeta) <= Ualpha)
                sector = 5;
            else
                sector = 4;
        }
    }

    return sector;  // 0..5
}

//void SVPWM_Update(float Ualpha, float Ubeta)
//{
//    float T1, T2;
//
//    float Uref = sqrtf(Ualpha * Ualpha + Ubeta * Ubeta);
//
//    // --- 1. Normalizacja napięcia (Circle Limitation) ---
//    // Zapobiega przesterowaniu i deformacji prądu przy dużych prędkościach
//    if (Uref > VOLTAGE_SUPPLY / M_SQRT3) {
//        float scale = (VOLTAGE_SUPPLY / M_SQRT3) / Uref;
//        Ualpha *= scale;
//        Ubeta *= scale;
//    }
//
//    int sector = SVPWM_GetSector(Ualpha, Ubeta);
//
//
//    float X = Ubeta;
//    float Y = (Ualpha - Ubeta * ONE_OVER_SQRT_3);
//    float Z = (Ualpha + Ubeta * ONE_OVER_SQRT_3);
//
//    switch (sector) {
//        case 0: T1 = Z;      T2 = X;      break;
//        case 1: T1 = X;      T2 = -Y;     break;
//        case 2: T1 = -Y;     T2 = -Z;     break;
//        case 3: T1 = -Z;     T2 = -X;     break;
//        case 4: T1 = -X;     T2 = Y;      break;
//        default:T1 = Y;      T2 = Z;      break;
//    }
//
//    // --- 4. Obliczenie czasów PWM
//
//    // Chcemy przeliczyć napięcie (X,Y,Z) na czas.
//    // Współczynnik K = (T_period * sqrt(3)) / V_bus
//    // T_period mamy w sekundach, wynik chcemy w sekundach (potem zamiana na ARR)
//    // ALBO prościej: od razu celujmy w ARR.
//
//    // Stała skalująca (oblicz raz):
//    // K = (ARR * sqrt(3)) / V_BUS
//    float time_scaler = (PWM_PERIOD_ARR * M_SQRT3) / VOLTAGE_SUPPLY;
//
//    // Teraz T1 i T2 są liniowo zależne od Ualpha/Ubeta
//    T1 *= time_scaler;
//    T2 *= time_scaler;
//
//    // --- 5. Wyznaczanie wektora zerowego ---
//    // Ponieważ T1 i T2 są już w tickach (ARR), T0 też liczymy w tickach.
//    float T0 = PWM_PERIOD_ARR - (T1 + T2);
//
//    // Zabezpieczenie (choć saturacja w pkt 1 powinna temu zapobiec)
//    if (T0 < 0.0f) T0 = 0.0f;
//
//    float half_T0 = T0 * 0.5f;
//
//    float Ta, Tb, Tc;
//
//    switch (sector) {
//        case 0: Ta = T1 + T2 + half_T0; Tb = T2 + half_T0;      Tc = half_T0;           break;
//        case 1: Ta = T1 + half_T0;      Tb = T1 + T2 + half_T0; Tc = half_T0;           break;
//        case 2: Ta = half_T0;           Tb = T1 + T2 + half_T0; Tc = T2 + half_T0;      break;
//        case 3: Ta = half_T0;           Tb = T1 + half_T0;      Tc = T1 + T2 + half_T0; break;
//        case 4: Ta = T2 + half_T0;      Tb = half_T0;           Tc = T1 + T2 + half_T0; break;
//        default:Ta = T1 + T2 + half_T0; Tb = half_T0;           Tc = T1 + half_T0;      break;
//    }
//
//    // --- 6. Wpisanie do rejestrów ---
//    // Ta, Tb, Tc są już w jednostkach ARR (Tickach)
//    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_1, (uint32_t)Ta);
//    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_2, (uint32_t)Tb);
//    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_3, (uint32_t)Tc);
//}

void SVPWM_Test_Run(float test_freq_hz)
{
	theta += M_TWOPI * test_freq_hz * PWM_PERIOD_SEC;
	if (theta > M_TWOPI) theta -= M_TWOPI;

	float Ualpha = TEST_VOLTAGE_AMPLITUDE * cosf(theta);
	float Ubeta = TEST_VOLTAGE_AMPLITUDE * sinf(theta);

	SVPWM_Update(Ualpha, Ubeta);
}

#define _PI_3          (M_PI / 3.0f)
#define INV_SQRT3      (0.57735026919f)    // 1/sqrt(3)
#define SQRT3          (1.73205080757f)
#define HALF_SQRT3     (0.86602540378f)    // sqrt(3)/2

// Środkowe kierunki sektorów (30°, 90°, 150°, 210°, 270°, 330°)
static const float cos_mid[6] = {
    0.86602540378f,   // cos 30°
    0.0f,             // cos 90°
   -0.86602540378f,   // cos 150°
   -0.86602540378f,   // cos 210°
    0.0f,             // cos 270°
    0.86602540378f    // cos 330°
};

static const float sin_mid[6] = {
    0.5f,             // sin 30°
    1.0f,             // sin 90°
    0.5f,             // sin 150°
   -0.5f,             // sin 210°
   -1.0f,             // sin 270°
   -0.5f              // sin 330°
};

// Granice sektorów: k * 60°
static const float cos_sec[6] = {
    1.0f,             // cos 0°
    0.5f,             // cos 60°
   -0.5f,             // cos 120°
   -1.0f,             // cos 180°
   -0.5f,             // cos 240°
    0.5f              // cos 300°
};

static const float sin_sec[6] = {
    0.0f,             // sin 0°
    HALF_SQRT3,       // sin 60°
    HALF_SQRT3,       // sin 120°
    0.0f,             // sin 180°
   -HALF_SQRT3,       // sin 240°
   -HALF_SQRT3        // sin 300°
};

void SVPWM_Update(float Ualpha, float Ubeta)
{
	float T1, T2, T0; // Czasy włączenia faz (w tickach sekundach)
    float Ta, Tb, Tc; // Czasy włączenia faz (w tickach timera)

    float Uref_sq = Ualpha * Ualpha + Ubeta * Ubeta;
    float Uref = sqrtf(Uref_sq);

    // 1. Ograniczenie napięcia: U <= Vdc / sqrt(3
    const float Umax = VOLTAGE_SUPPLY / M_SQRT3;
    if (Uref > VOLTAGE_SUPPLY / M_SQRT3) {
        float scale = Umax / Uref;
        Ualpha *= scale;
        Ubeta  *= scale;
        Uref    = Umax;
    }

    // Sin i cos kąta wektora (bez atan2)
    float cos_theta = Ualpha / Uref;
    float sin_theta = Ubeta  / Uref;

    // Detekcja sektora: wybierz najbliższy wektor środkowy (30°, 90°, ...)
    int sector = 0;
    float max_dot = cos_theta * cos_mid[0] + sin_theta * sin_mid[0]; // inicjalizacja - zaczynamy od sektora 0

    for (int k = 1; k < 6; ++k) {
        float dot = cos_theta * cos_mid[k] + sin_theta * sin_mid[k];
        if (dot > max_dot) {
            max_dot = dot;
            sector = k;
        }
    }
    // sector ∈ {0..5}, granice między sektorami są dokładnie co 60°

    //    BEZ ATAN, nie znamy θ_s
    // --- 4. Wyznaczenie sin(θ_s) i sin(π/3 − θ_s) dla danego sektora
    // --- Wyznaczenie sin(θ_s) oraz sin(π/3 − θ_s) bez użycia atan2
    //
    // θ_s = θ − θ_k           (θ_k = k * 60° – kąt wektora bazowego sektora)
    //
    // Z tożsamości: sin(θ − φ) = sinθ·cosφ − cosθ·sinφ
    //
    // Dlatego:
    //   sin(θ_s)          = sinθ*cosθ_k − cosθ*sinθ_k
    //   sin(π/3 − θ_s)    = sinθ_(k+1)*cosθ − cosθ_(k+1)*sinθ
    //
    // Wartości te odpowiadają projekcjom Vref na dwa wektory aktywne
    // i służą do obliczenia czasów T1 i T2 w SVPWM.

    int k      = sector;
    int k_next = (sector + 1) % 6;

    float cos_k     = cos_sec[k];
    float sin_k     = sin_sec[k];
    float cos_k1    = cos_sec[k_next];
    float sin_k1    = sin_sec[k_next];

    float sin_theta_s  = sin_theta * cos_k  - cos_theta * sin_k;
    float sin_pi3_min  = sin_k1   * cos_theta - cos_k1    * sin_theta;

    // Zabezpieczenie przed drobnymi błędami numerycznymi
    if (sin_theta_s < 0.0f)   sin_theta_s = 0.0f;
    if (sin_pi3_min < 0.0f)   sin_pi3_min = 0.0f;

    // Współczynnik modulacji m od 0 do 1
    float m = M_SQRT3 * Uref / VOLTAGE_SUPPLY;

    //  Czasy T1, T2 w sekundach (jak w wersji z atan2)
    T1 = m * sin_pi3_min * PWM_PERIOD_SEC;
    T2 = m * sin_theta_s * PWM_PERIOD_SEC;

    // Czas wektora zerowego
    T0 = PWM_PERIOD_SEC - T1 - T2;
    float half_T0 = 0.5f * T0;

    // Wyznacz Ta, Tb, Tc (sekundy)
	switch (sector) {
		case 0: // Sektor 1 (wektory V1, V2)
			Ta = T1 + T2 + half_T0;
			Tb = T2 + half_T0;
			Tc = half_T0;
			break;
		case 1: // Sektor 2 (wektory V2, V3)
			Ta = T1 + half_T0;
			Tb = T1 + T2 + T0 / 2.0f;
			Tc = T0 / 2.0f;
			break;
		case 2: // Sektor 3 (wektory V3, V4)
			Ta = half_T0;
			Tb = T1 + T2 + half_T0;
			Tc = T2 + half_T0;
			break;
		case 3: // Sektor 4 (wektory V4, V5)
			Ta = half_T0;
			Tb = T1 + half_T0;
			Tc = T1 + T2 + half_T0;
			break;
		case 4: // Sektor 5 (wektory V5, V6)
			Ta = T2 + half_T0;
			Tb = half_T0;
			Tc = T1 + T2 + half_T0;
			break;
		case 5: // Sektor 6 (wektory V6, V1)
			Ta = T1 + T2 + half_T0;
			Tb = half_T0;
			Tc = T1 + half_T0;
			break;
		default: // Powinno się nigdy nie zdarzyć
			Ta = Tb = Tc = PWM_PERIOD_SEC / 2.0f;
			break;
	}
    // Przeskalowanie [sekundy] → [ticki timera]
    const float sec_to_arr = (float)PWM_PERIOD_ARR / PWM_PERIOD_SEC;

    uint32_t ccr1 = (uint32_t)(Ta * sec_to_arr);
    uint32_t ccr2 = (uint32_t)(Tb * sec_to_arr);
    uint32_t ccr3 = (uint32_t)(Tc * sec_to_arr);

    if (ccr1 > PWM_PERIOD_ARR) ccr1 = PWM_PERIOD_ARR;
    if (ccr2 > PWM_PERIOD_ARR) ccr2 = PWM_PERIOD_ARR;
    if (ccr3 > PWM_PERIOD_ARR) ccr3 = PWM_PERIOD_ARR;

    debug_Ta = (uint16_t)ccr1;
    debug_Tb = (uint16_t)ccr2;
    debug_Tc = (uint16_t)ccr3;

    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_1, ccr1);
    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_2, ccr2);
    __HAL_TIM_SET_COMPARE(svpwm_htim, TIM_CHANNEL_3, ccr3);
}


