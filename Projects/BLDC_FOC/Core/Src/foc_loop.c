/*
 * foc_loop.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#include "foc_loop.h"
#include "config.h"
#include "main.h"

static PI_Controller pi_id = { .kp = 2.0f, .ki = 200.0f, .limit = 10.0f, .integral = 0.0f };
static PI_Controller pi_iq = { .kp = 2.0f, .ki = 200.0f, .limit = 10.0f, .integral = 0.0f };
static abc_current_t currents;

volatile dq_ref_t current_ref;

volatile AS5048_ReadResult raw = {0};
volatile float theta_el = 0.0f;

// FLAGS
volatile bool currents_ready = false;
volatile bool encoder_ready = false;
volatile bool encoder_trigger = false;
volatile bool encoder_calibrated = false;

// DEBUG
volatile float debug_angle_deg = 0.0f;
volatile float debug_theta_el = 0.0f;
volatile float debug_ia = 0.0f, debug_ib = 0.0f, debug_ic = 0.0f;
volatile uint16_t raw_copy = 0;

static TIM_HandleTypeDef* foc_htim;
static ADC_HandleTypeDef* foc_hadc;
#define PWM_PERIOD (htim1.Init.Period)
//#define PWM_PERIOD 8499;

// Parametry odczytane podczas kalibracji
static int sensor_direction = 0; // 1 - CW, -1 - CCW
static float zero_electric_angle = 0.0f;

// --- Zmienne konfiguracyjne (nie 'extern') ---
static const int POLE_PAIRS = 7;
static const float VOLTAGE_POWER_SUPPLY = 12.0f;
static const float VOLTAGE_LIMIT = 10.0f;
static float VOLTAGE_SENSOR_ALIGN = 8.0f;

static inline float pi_control(PI_Controller *pi, float error)
{
    pi->integral += error * pi->ki * 0.00005f; // Ts = 50 us

    if (pi->integral > pi->limit) pi->integral = pi->limit;
    else if (pi->integral < -pi->limit) pi->integral = -pi->limit;

    float output = pi->kp * error + pi->integral;

    if (output > pi->limit) output = pi->limit;
    else if (output < -pi->limit) output = -pi->limit;

    return output;
}

void FOC_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim)
{
		printf("FOC: Init...\n");
		foc_htim = htim;
	    foc_hadc = hadc;

        AS5048_Init();

        HAL_TIM_Base_Start(foc_htim);
        HAL_TIM_OC_Start(foc_htim, TIM_CHANNEL_4); 	// Start CH4 -> wyzwalanie ADC

        HAL_Delay(50);

        // Kalibracja przed aktywowaniem drivera PWM
        CurrentSense_Init(hadc);

        SVPWM_Init(foc_htim); // Włączenie driverów i PWM
        FOC_AlignSensor();

        HAL_TIM_Base_Stop(foc_htim);
        HAL_TIM_Base_Start_IT(foc_htim);

        HAL_TIM_OC_Start(foc_htim, TIM_CHANNEL_4);
        HAL_ADCEx_InjectedStart_IT(hadc);
}

void FOC_SetPhaseVoltage(float Uq, float Ud, float angle_el) {
    // --- Krok 1: Ograniczenie wektora napięcia ---
    // Zapewnia, że żądane napięcie nie przekracza fizycznych możliwości systemu.
    float U_ref = sqrtf(Uq * Uq + Ud * Ud);
    if (U_ref > VOLTAGE_LIMIT) {
        Uq *= VOLTAGE_LIMIT / U_ref;
        Ud *= VOLTAGE_LIMIT / U_ref;
    }

    // --- Krok 2: Odwrotna transformacja Parka ---
    // Przekształca napięcia z wirującego układu współrzędnych (d-q)
    // na stacjonarny układ współrzędnych (alpha-beta).
    float Ualpha = -sinf(angle_el) * Uq + cosf(angle_el) * Ud;
    float Ubeta  =  cosf(angle_el) * Uq + sinf(angle_el) * Ud;

    // --- Krok 3: Odwrotna transformacja Clarke'a ---
    // Przekształca napięcia z układu alpha-beta na napięcia
    // dla trzech fizycznych faz silnika (a, b, c).
    float Ua = Ualpha;
    float Ub = -0.5f * Ualpha - _SQRT3_2 * Ubeta;
    float Uc = -0.5f * Ualpha + _SQRT3_2 * Ubeta;

    // --- Krok 4: Mapowanie na PWM dla drivera 3-PWM ---
    // Napięcia fazowe Ua, Ub, Uc są teraz w zakresie [-VOLTAGE_LIMIT, +VOLTAGE_LIMIT].
    // Musimy je przeskalować i przesunąć do zakresu [0, PWM_PERIOD] dla timera.

    // Dzielimy przez napięcie zasilania, aby uzyskać współczynnik w zakresie [-x, +x],
    // gdzie x = VOLTAGE_LIMIT / VOLTAGE_POWER_SUPPLY.
    float dc_a = Ua / VOLTAGE_POWER_SUPPLY;
    float dc_b = Ub / VOLTAGE_POWER_SUPPLY;
    float dc_c = Uc / VOLTAGE_POWER_SUPPLY;

    // Dodajemy 0.5, aby przesunąć zakres.
    // Np. jeśli dc_a było w [-0.4, +0.4], teraz będzie w [0.1, 0.9].
    // To centrowanie jest kluczowe dla drivera 3-PWM.
    dc_a += 0.5f;
    dc_b += 0.5f;
    dc_c += 0.5f;

    // Przeliczamy współczynniki wypełnienia (0.0 do 1.0) na wartości dla rejestru timera.
    uint32_t pwm_a = (uint32_t)(dc_a * PWM_PERIOD);
    uint32_t pwm_b = (uint32_t)(dc_b * PWM_PERIOD);
    uint32_t pwm_c = (uint32_t)(dc_c * PWM_PERIOD);

    // Zabezpieczenie na wszelki wypadek, choć przy poprawnym ograniczeniu Uq/Ud nie powinno być potrzebne.
    if (pwm_a > PWM_PERIOD) pwm_a = PWM_PERIOD;
    if (pwm_b > PWM_PERIOD) pwm_b = PWM_PERIOD;
    if (pwm_c > PWM_PERIOD) pwm_c = PWM_PERIOD;

    // --- Krok 5: Ustawienie wartości w rejestrach timera ---
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_a);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_b);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_c);
}

/**
 * @brief Przeprowadza pełną procedurę kalibracji sensora.
 * @note  Ta funkcja jest blokująca i używa HAL_Delay(). Powinna być wywoływana tylko raz, podczas inicjalizacji.
 * @retval None. Wyniki są zapisywane w globalnych zmiennych `sensor_direction` i `zero_electric_angle`.
 */

static float normalize_angle(float angle) {
    float result = fmodf(angle, _2PI);
    return result >= 0 ? result : result + _2PI;
}
static float readSensorAngle() {
    AS5048_ReadResult raw_angle;
    AS5048_Get_Raw_Position(&raw_angle);
    if (raw_angle.status != AS5048_OK) { return -1.0f; }
    return ((float)raw_angle.position / 16384.0f) * _2PI;
}

// Funkcja pomocnicza do obliczania kąta elektrycznego BEZ offsetu
// (potrzebna w kalibracji)
static float FOC_GetElecticalAngle_without_offset(float mechanical_angle, int direction, int pole_pairs) {
    return normalize_angle((float)direction * pole_pairs * mechanical_angle);
}

// --- Funkcja do obliczania kąta elektrycznego (do użycia w pętli FOC) ---
float FOC_GetElectricalAngle() {
    float mechanical_angle = readSensorAngle();
    if (mechanical_angle < 0.0f) return 0.0f;

    // Używamy wzoru zgodnego z logiką SimpleFOC
    float angle_electrical = (float)(sensor_direction * POLE_PAIRS) * mechanical_angle - zero_electric_angle;

    return normalize_angle(angle_electrical);
}

// --- Implementacje funkcji publicznych ---
// FOC_Init i FOC_SetPhaseVoltage z poprzednich odpowiedzi są OK, zostawiamy je.

// ============================================================================
// ===      FINALNA IMPLEMENTACJA FOC_AlignSensor (W STYLU SIMPLEFOC)       ===
// ============================================================================
void FOC_AlignSensor() {
    printf("\n--- Rozpoczynam procedure kalibracji (metoda SimpleFOC) ---\n");
    int exit_flag = 1;

    // --- KROK 1: Wykrywanie kierunku metodą "przód-tył" ---
    printf("Krok 1: Wykrywanie kierunku...\n");

    // Obrót "w przód" o jeden obrót elektryczny
    for (int i = 0; i <= 500; i++) {
        float angle = _3PI_2 + ((float)i / 500.0f) * _2PI;
        FOC_SetPhaseVoltage(VOLTAGE_SENSOR_ALIGN, 0, angle);
        HAL_Delay(2);
    }
    float mid_angle = readSensorAngle();
    if (mid_angle < 0.0f) { exit_flag = 0; }

    if (exit_flag) {
        // Obrót "w tył"
        for (int i = 500; i >= 0; i--) {
            float angle = _3PI_2 + ((float)i / 500.0f) * _2PI;
            FOC_SetPhaseVoltage(VOLTAGE_SENSOR_ALIGN, 0, angle);
            HAL_Delay(2);
        }
        float end_angle = readSensorAngle();
        if (end_angle < 0.0f) { exit_flag = 0; }

        if (exit_flag) {
            // Analiza ruchu
            float moved = mid_angle - end_angle;
            // W kodzie SimpleFOC jest proste porównanie, ale normalizacja jest bezpieczniejsza
            if (moved < -_PI) moved += _2PI;
            if (moved > _PI)  moved -= _2PI;

            if (fabs(moved) < 0.1f) {
                printf("  BLAD: Silnik sie nie poruszyl!\n");
                exit_flag = 0;
            } else {
                // Ta logika jest trochę inna niż w Twoim wklejonym kodzie, ale bardziej intuicyjna
                sensor_direction = (moved > 0) ? 1 : -1;
                if (sensor_direction == 1) {
                       printf("  Wynik: Kierunek sensora: 1 (CW - zgodny z ruchem wskazowek zegara)\n");
                   } else {
                       printf("  Wynik: Kierunek sensora: -1 (CCW - przeciwny do ruchu wskazowek zegara)\n");
                   }

                // Weryfikacja par biegunów
                float expected_movement = _2PI / POLE_PAIRS;
                if (fabs(fabs(moved) - expected_movement) > 0.5f) {
                    printf("  OSTRZEZENIE: Sprawdzenie par biegunow nie powiodlo sie!\n");
                    // exit_flag = 0; // Możesz zdecydować, czy to ma być błąd krytyczny
                } else {
                    printf("  Wynik: Sprawdzenie par biegunow: OK!\n");
                }
            }
        }
    }

    // --- KROK 2: Znalezienie zerowego kąta elektrycznego ---
    if (exit_flag) {
        printf("\nKrok 2: Wyrównywanie do zera elektrycznego...\n");
        // Ustaw wirnik w znanej pozycji elektrycznej (_3PI_2)
        FOC_SetPhaseVoltage(VOLTAGE_SENSOR_ALIGN, 0, _3PI_2);
        HAL_Delay(700);

        // Odczytaj kąt mechaniczny z sensora
        float mechanical_angle_at_known_el_pos = readSensorAngle();
        if (mechanical_angle_at_known_el_pos < 0.0f) {
            exit_flag = 0;
        } else {
            // Oblicz kąt elektryczny, jaki wynika z tego pomiaru (bez offsetu)
            float calculated_el_angle = FOC_GetElecticalAngle_without_offset(mechanical_angle_at_known_el_pos, sensor_direction, POLE_PAIRS);

            // Offset to różnica między tym, gdzie pole POWINNO być, a tym, co obliczyliśmy
            // Ale SimpleFOC robi to prościej: po prostu zapisuje obliczoną wartość.
            // Zróbmy to tak samo.
            zero_electric_angle = calculated_el_angle;

            printf("  Wynik: Znaleziony offset ELEKTRYCZNY: %.3f rad\n", zero_electric_angle);
        }
    }

    // Zakończenie
    FOC_SetPhaseVoltage(0, 0, 0);
    if (exit_flag) {
        printf("--- Kalibracja zakonczona POMYSLNIE! ---\n\n");
    } else {
        printf("--- Kalibracja ZAKONCZONA BLEDEM! ---\n\n");
    }
}

void FOC_Update(abc_current_t *currents, float sin_theta, float cos_theta, volatile dq_ref_t *i_ref)
{
    float ialpha, ibeta;
    float id, iq;
    float vd, vq;
    float valpha, vbeta;

    // 1. Clarke's transformation – abc → αβ
    ClarkeTransform(currents->a, currents->b, &ialpha, &ibeta);

    // 2. Park's transformation – αβ → dq
    ParkTransformTrig(ialpha, ibeta, sin_theta, cos_theta, &id, &iq);

    // 3. PI regulators
    vd = pi_control(&pi_id, i_ref->d - id);
    vq = pi_control(&pi_iq, i_ref->q - iq);

//    printf("vd=%.3f vq=%.3f\n", vd, vq);

    // 4. Inverse Park – dq → αβ
    InvParkTransformTrig(vd, vq, sin_theta, cos_theta, &valpha, &vbeta);

    // 5. SVPWM
//    printf("valpha=%.3f vbeta=%.3f\n", valpha, vbeta);

    SVPWM_Update(valpha, vbeta);
}

// Cała pętla FOC + pomiar prądu
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        CurrentSense_Measurement(hadc);
        CurrentSense_Read(&currents);
//        printf("\nIa: %.3f A, Ib: %.3f A, Ic: %.3f A\r\n", currents.a, currents.b, currents.c);
        currents_ready = true;

        SVPWM_Test_Run(50.0f);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {

//	  SVPWM_Test_Run(50.0f);

  }
}











//// Read encoder
//void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
//    {
//        encoder_trigger = true;
////        SVPWM_Test_Run(10.0f);
//    }
//}

// FOC loop
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if (htim->Instance == TIM1)
//    {
//        if (currents_ready && encoder_ready)
//        {
//            float sin_theta = sinf(theta_el);
//            float cos_theta = cosf(theta_el);
//
////            FOC_Update(&currents, sin_theta, cos_theta, &current_ref);
//
//            // DEBUG SECTION
//
//            debug_angle_deg = ((float)raw_copy * 360.0f) / 16384.0f;
//            debug_theta_el = theta_el;
//            debug_ia = currents.a;
//            debug_ib = currents.b;
//            debug_ic = currents.c;
//
//            // END DEBUG SECTION
//
//            currents_ready = false;
//            encoder_ready = false;
//        }
//    }

//    if (htim->Instance == TIM2)
//    {
//        SVPWM_Test_Run(20.0f); // np. 1 Hz obrót
//    }
//}

// FOC Loop
//void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
//{
//    if (hadc->Instance == ADC1)
//    {
//        // 1. Pomiar prądów i pobranie zmierzonych wartości
//        CurrentSense_Meassurement(hadc);
//        CurrentSense_Read(&currents);
//
//        // 3. Odczyt kąta z enkodera
//        AS5048_Get_Raw_Position(&raw);
//        theta_el = GetElectricalAngle(raw.position, MOTOR_POLE_PAIRS);
//
//        float sin_theta = sinf(theta_el);
//        float cos_theta = cosf(theta_el);
//
//        // 4. FOC aktualizacja
//        FOC_Update(&currents, sin_theta, cos_theta, &current_ref);
//    }
//}




