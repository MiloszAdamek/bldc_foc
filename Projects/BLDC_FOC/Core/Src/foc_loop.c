/*
 * foc_loop.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#include "motor_config.h"
#include "foc_loop.h"
#include "math.h"
#include "main.h"

static TIM_HandleTypeDef* foc_htim;
static ADC_HandleTypeDef* foc_hadc;

static PI_Controller pi_id = { .kp = PI_KP_ID, .ki = PI_KI_ID, .limit = PI_LIMIT_ID, .integral = 0.0f };
static PI_Controller pi_iq = { .kp = PI_KP_IQ, .ki = PI_KI_IQ, .limit = PI_LIMIT_IQ, .integral = 0.0f };
static abc_current_t currents;

volatile dq_ref_t i_ref = {0.0f, 0.0f};

// RAMP
volatile dq_ref_t ramp_i_ref;
static const float iq_step = 0.001f; // przyrost na 1 krok (ok. 20kHz pętla -> ~50ms czas)
static float iq_threshold = 0.05f;
static float iq_current;

// FLAGS
volatile bool ramp_active = false;
volatile bool spi_angle_ready = false;
volatile bool foc_data_ready = false;

// ENCODER - calibration
static int sensor_direction = 0; // 1 - CW, -1 - CCW
static float zero_electric_angle = 0.0f;
static float VOLTAGE_SENSOR_ALIGN = 4.0f;

// Debug - cubemonitor
volatile float debug_id = 0.0f;
volatile float debug_iq = 0.0f;
volatile float debug_id_ref = 0.0f;
volatile float debug_iq_ref = 0.0f;
volatile float debug_vd = 0.0f;
volatile float debug_vq = 0.0f;

//static inline float pi_control(PI_Controller *pi, float error)
//{
//    pi->integral += error * pi->ki * PWM_PERIOD_SEC; // Ts = 50 us
//
//    if (pi->integral > pi->limit) pi->integral = pi->limit;
//    else if (pi->integral < -pi->limit) pi->integral = -pi->limit;
//
//    float output = pi->kp * error + pi->integral;
//
//    if (output > pi->limit) output = pi->limit;
//    else if (output < -pi->limit) output = -pi->limit;
//
//    return output;
//}

float pi_control(PI_Controller *pi, float error)
{
    float u_p = pi->kp * error;
    pi->integral += pi->ki * error * PWM_PERIOD_SEC;

    float u = u_p + pi->integral;
    if (u > pi->limit) { u = pi->limit; pi->integral = u - u_p; }
    else if (u < -pi->limit) { u = -pi->limit; pi->integral = u - u_p; }

    return u;
}

void FOC_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim, SPI_HandleTypeDef *hspi)
{
		printf("FOC: Init...\n");
		foc_htim = htim;
	    foc_hadc = hadc;

        AS5048_Init(hspi);

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
    float dc_a = Ua / VOLTAGE_SUPPLY;
    float dc_b = Ub / VOLTAGE_SUPPLY;
    float dc_c = Uc / VOLTAGE_SUPPLY;

    // Dodajemy 0.5, aby przesunąć zakres.
    // Np. jeśli dc_a było w [-0.4, +0.4], teraz będzie w [0.1, 0.9].
    // To centrowanie jest kluczowe dla drivera 3-PWM.
    dc_a += 0.5f;
    dc_b += 0.5f;
    dc_c += 0.5f;

    // Przeliczamy współczynniki wypełnienia (0.0 do 1.0) na wartości dla rejestru timera.
    uint32_t pwm_a = (uint32_t)(dc_a * PWM_PERIOD_ARR);
    uint32_t pwm_b = (uint32_t)(dc_b * PWM_PERIOD_ARR);
    uint32_t pwm_c = (uint32_t)(dc_c * PWM_PERIOD_ARR);

    // Zabezpieczenie na wszelki wypadek, choć przy poprawnym ograniczeniu Uq/Ud nie powinno być potrzebne.
    if (pwm_a > PWM_PERIOD_ARR) pwm_a = PWM_PERIOD_ARR;
    if (pwm_b > PWM_PERIOD_ARR) pwm_b = PWM_PERIOD_ARR;
    if (pwm_c > PWM_PERIOD_ARR) pwm_c = PWM_PERIOD_ARR;

    // --- Krok 5: Ustawienie wartości w rejestrach timera ---
    __HAL_TIM_SET_COMPARE(foc_htim, TIM_CHANNEL_1, pwm_a);
    __HAL_TIM_SET_COMPARE(foc_htim, TIM_CHANNEL_2, pwm_b);
    __HAL_TIM_SET_COMPARE(foc_htim, TIM_CHANNEL_3, pwm_c);
}

/**
 * @brief Przeprowadza pełną procedurę kalibracji sensora.
 * @note  Ta funkcja jest blokująca i używa HAL_Delay(). Powinna być wywoływana tylko raz, podczas inicjalizacji.
 * @retval None. Wyniki są zapisywane w globalnych zmiennych `sensor_direction` i `zero_electric_angle`.
 */

static float normalize_angle(float angle) {
    float result = fmodf(angle, M_TWOPI);
    return result >= 0 ? result : result + M_TWOPI;
}

// Funkcja pomocnicza do obliczania kąta elektrycznego BEZ offsetu
// (potrzebna w kalibracji)
static float FOC_GetElecticalAngle_without_offset(float mechanical_angle, int direction, int pole_pairs) {
    return normalize_angle((float)direction * pole_pairs * mechanical_angle);
}

// --- Funkcja do obliczania kąta elektrycznego (do użycia w pętli FOC) ---
static float FOC_GetElectricalAngle() {

//	if (spi_ready) AS5048_ReadAngleDMA();

    float mechanical_angle = AS5048_GetMechanicalAngle();
    if (mechanical_angle < 0.0f) return 0.0f;

    float electrical_angle = (float)(sensor_direction * MOTOR_POLE_PAIRS) * mechanical_angle - zero_electric_angle;

    return normalize_angle(electrical_angle);
}

static void FOC_GetSinCosTheta(float *sin_theta_el, float *cos_theta_el){
	float theta_el = FOC_GetElectricalAngle();
	*cos_theta_el = cosf(theta_el);
	*sin_theta_el = sinf(theta_el);
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
        float angle = _3PI_2 + ((float)i / 500.0f) * M_TWOPI;
        FOC_SetPhaseVoltage(VOLTAGE_SENSOR_ALIGN, 0, angle);
        HAL_Delay(2);
    }
    float mid_angle = AS5048_GetAngleRad();
    if (mid_angle < 0.0f) { exit_flag = 0; }

    if (exit_flag) {
        // Obrót "w tył"
        for (int i = 500; i >= 0; i--) {
            float angle = _3PI_2 + ((float)i / 500.0f) * M_TWOPI;
            FOC_SetPhaseVoltage(VOLTAGE_SENSOR_ALIGN, 0, angle);
            HAL_Delay(2);
        }
        float end_angle = AS5048_GetAngleRad();
        if (end_angle < 0.0f) { exit_flag = 0; }

        if (exit_flag) {
            // Analiza ruchu
            float moved = mid_angle - end_angle;
            // W kodzie SimpleFOC jest proste porównanie, ale normalizacja jest bezpieczniejsza
            if (moved < - M_PI) moved += M_TWOPI;
            if (moved > M_PI)  moved -= M_TWOPI;

            if (fabs(moved) < 0.1f) {
                printf("  BLAD: Silnik sie nie poruszyl!\n");
                exit_flag = 0;
            } else {
                // Ta logika jest trochę inna niż w Twoim wklejonym kodzie, ale bardziej intuicyjna
            	sensor_direction = (moved > 0) ? -1 : 1;
                if (sensor_direction == 1) {
                       printf("  Wynik: Kierunek sensora: 1 (CW - zgodny z ruchem wskazowek zegara)\n");
                   } else {
                       printf("  Wynik: Kierunek sensora: -1 (CCW - przeciwny do ruchu wskazowek zegara)\n");
                   }

                // Weryfikacja par biegunów
                float expected_movement = M_TWOPI / MOTOR_POLE_PAIRS;
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
        float mechanical_angle_at_known_el_pos = AS5048_GetAngleRad();
        if (mechanical_angle_at_known_el_pos < 0.0f) {
            exit_flag = 0;
        } else {
            // Oblicz kąt elektryczny, jaki wynika z tego pomiaru (bez offsetu)
            float calculated_el_angle = FOC_GetElecticalAngle_without_offset(mechanical_angle_at_known_el_pos, sensor_direction, MOTOR_POLE_PAIRS);

            // Offset to różnica między tym, gdzie pole POWINNO być, a tym, co obliczyliśmy
            // Ale SimpleFOC robi to prościej: po prostu zapisuje obliczoną wartość.
            // Zróbmy to tak samo.
            zero_electric_angle = normalize_angle(calculated_el_angle + _3PI_2);

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

void FOC_LinearRamp()
{
	// TODO: Rampa powinna być aktywna tylko gdy silnik stoi, a nie po każdej zmianie Iq - maszyna stanów
    if (ramp_active) {
        if (iq_current < i_ref.q) {
            iq_current += iq_step;
            if (iq_current >= (i_ref.q - iq_threshold)) {
                iq_current = i_ref.q;
                ramp_active = false;  // osiągnięto próg — wyłącz rampę
                printf("Rampa zakonczona, iq = %.3f A\n", iq_current);
            }
        } else {
            iq_current = i_ref.q;
            ramp_active = false;
        }
    }
    ramp_i_ref.q = iq_current;
}

void FOC_SetIqTarget(float new_target)
{
	i_ref.q = new_target;
	iq_threshold = new_target / 2;
    ramp_active = true;
}

void FOC_Update()
{
    float ialpha, ibeta;
    float id, iq;
    float vd, vq;
    float valpha, vbeta;
    float cos_theta_el, sin_theta_el;

    if (ramp_active){
        FOC_LinearRamp();
        i_ref.q = ramp_i_ref.q;
    }

    FOC_GetSinCosTheta(&sin_theta_el, &cos_theta_el);

    // 1. Clarke's transformation – abc → αβ
    ClarkeTransform(currents.a, currents.b, &ialpha, &ibeta);

    // 2. Park's transformation – αβ → dq
    ParkTransformTrig(ialpha, ibeta, &sin_theta_el, &cos_theta_el, &id, &iq);

    // 3. PI regulators
    vd = pi_control(&pi_id, i_ref.d - id);
    vq = pi_control(&pi_iq, i_ref.q - iq);

    debug_id = id;
    debug_iq = iq;
    debug_id_ref = i_ref.d;
    debug_iq_ref = i_ref.q;
    debug_vd = vd;
    debug_vq = vq;

    // 4. Inverse Park – dq → αβ
    InvParkTransformTrig(vd, vq, &sin_theta_el, &cos_theta_el, &valpha, &vbeta);

    // 5. SVPWM
    SVPWM_Update(valpha, vbeta);
}

// Cała pętla FOC + pomiar prądu
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
    	CurrentSense_Process(hadc);
        CurrentSense_Read(&currents);
        if (spi_ready){
        	AS5048_ReadAngleDMA();
        }
        foc_data_ready = true;
//        SVPWM_Test_Run(40.0f)
    }
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if (htim->Instance == TIM1)
//    {
//        if (spi_ready)
//        {
//            spi_ready = false;
//            AS5048_ReadAngleDMA();   // wystartuj DMA
//        }
//    }
//
//}




