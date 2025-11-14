/*
 * foc_loop.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#include "App/config.h"
#include "FOC/foc_loop.h"
#include "FOC/controller_utils.h"
#include "FOC/speed_control.h"
#include "math.h"
#include "main.h"


static TIM_HandleTypeDef* foc_htim;
static ADC_HandleTypeDef* foc_hadc;

static PI_Controller pi_id = { .kp = PI_KP_ID, .ki = PI_KI_ID, .limit = PI_LIMIT_ID, .integral = 0.0f, .dt = PWM_PERIOD_SEC};
static PI_Controller pi_iq = { .kp = PI_KP_IQ, .ki = PI_KI_IQ, .limit = PI_LIMIT_IQ, .integral = 0.0f, .dt = PWM_PERIOD_SEC};

// IQ, ID controller
static abc_current_t currents;
volatile dq_ref_t i_ref = {0.0f, 0.0f};

// Speed estimator
volatile float actual_speed_rpm = 0.0f;

// RAMP
volatile dq_ref_t ramp_i_ref;
static const float iq_step = 0.0001f; // przyrost prądu na 1 krok
static float iq_current;

// FLAGS
volatile bool ramp_active = false;
volatile bool spi_angle_ready = false;
volatile bool foc_data_ready = false;
volatile bool sensor_aligned = false;

// ENCODER
static int sensor_direction = 0; // 1 - CW, -1 - CCW
static float zero_electric_angle = 0.0f;
volatile float theta_mech_latest = 0.0f;
volatile float theta_el_latest = 0.0f;

// Debug - cubemonitor
volatile float debug_id = 0.0f;
volatile float debug_iq = 0.0f;
volatile float debug_id_ref = 0.0f;
volatile float debug_iq_ref = 0.0f;
volatile float debug_vd = 0.0f;
volatile float debug_vq = 0.0f;
volatile float debug_speed = 0.0f;

void FOC_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim, SPI_HandleTypeDef *hspi)
{
	printf("FOC: Init...\n");
	foc_htim = htim;
	foc_hadc = hadc;

	AS5048_Init(hspi);

	HAL_TIM_Base_Start(foc_htim);
	HAL_TIM_OC_Start(foc_htim, TIM_CHANNEL_4); 	// Start CH4 -> wyzwalanie ADC

	HAL_Delay(50);

	CurrentSense_Init(hadc);

    SVPWM_Init(foc_htim);
    FOC_AlignSensor();

    // Odczyt kąta przed uruchomieniem pętli FOC
    float mech0 = AS5048_GetAngleRad();
    if (mech0 >= 0.0f) {
        theta_mech_latest = mech0;
        theta_el_latest = FOC_GetElecticalAngle(mech0);
    }

    HAL_TIM_Base_Stop(foc_htim);

    // Uruchomienie DMA dla SPI
    if (g_spi_ready) AS5048_ReadAngleDMA();
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

inline float FOC_GetElecticalAngle(float mech){
    return normalize_angle((float)(sensor_direction * MOTOR_POLE_PAIRS) * mech - zero_electric_angle);
}

// Funkcja pomocnicza do obliczania kąta elektrycznego BEZ offsetu(potrzebna w kalibracji)
static float FOC_GetElecticalAngle_NoOffset(float mechanical_angle) {
    return normalize_angle((float)sensor_direction * MOTOR_POLE_PAIRS * mechanical_angle);
}

bool FOC_AlignSensor() {

	if(!sensor_aligned){
		printf("\n--- Rozpoczynam procedure kalibracji (metoda SimpleFOC) ---\n");

		int exit_flag = 1;

		// --- KROK 1: Wykrywanie kierunku metodą "przód-tył" ---
		printf("Krok 1: Wykrywanie kierunku...\n");

		// Obrót "w przód" o jeden obrót elektryczny
		for (int i = 0; i <= 500; i++) {
			float angle = _3PI_2 + ((float)i / 500.0f) * M_TWOPI;
			FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, angle);
			HAL_Delay(2);
		}
		float mid_angle = AS5048_GetAngleRad();
		if (mid_angle < 0.0f) { exit_flag = 0; }

		if (exit_flag) {
			// Obrót "w tył"
			for (int i = 500; i >= 0; i--) {
				float angle = _3PI_2 + ((float)i / 500.0f) * M_TWOPI;
				FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, angle);
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
			FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, _3PI_2);
			HAL_Delay(700);

			// Odczytaj kąt mechaniczny z sensora
			float mechanical_angle_at_known_el_pos = AS5048_GetAngleRad();
			if (mechanical_angle_at_known_el_pos < 0.0f) {
				exit_flag = 0;
			} else {
				// Oblicz kąt elektryczny, jaki wynika z tego pomiaru (bez offsetu)
				float calculated_el_angle = FOC_GetElecticalAngle_NoOffset(mechanical_angle_at_known_el_pos);

				// Offset to różnica między tym, gdzie pole POWINNO być, a tym, co obliczyliśmy
				// Ale SimpleFOC robi to prościej: po prostu zapisuje obliczoną wartość.
				// Zróbmy to tak samo.

				zero_electric_angle = normalize_angle(calculated_el_angle - _3PI_2);

				printf("  Wynik: Znaleziony offset ELEKTRYCZNY: %.3f rad\n", zero_electric_angle);
			}
		}

		// Zakończenie
		FOC_SetPhaseVoltage(0, 0, 0);
		if (exit_flag) {
			printf("--- Kalibracja zakonczona POMYSLNIE! ---\n\n");
			sensor_aligned = true;
		} else {
			printf("--- Kalibracja ZAKONCZONA BLEDEM! ---\n\n");
			sensor_aligned = false;
		}
	}
	return sensor_aligned;
}

void FOC_LinearRamp()
{
    if (ramp_active)
    {
        if (iq_current < i_ref.q)
        {
            iq_current += iq_step;
            if (iq_current > i_ref.q) {
                iq_current = i_ref.q;
                ramp_active = false; // Zakończ rampę
            }
        }
        else if (iq_current > i_ref.q)
        {
            iq_current -= iq_step;
            if (iq_current < i_ref.q) {
                iq_current = i_ref.q;
                ramp_active = false; // Zakończ rampę
            }
        }
        else
        {
            ramp_active = false;
        }
    }
    ramp_i_ref.q = iq_current;
}

void FOC_SetIqTarget_Ramp(float new_target)
{
	i_ref.q = new_target;
    ramp_active = true;
}

void FOC_SetIqTarget(float new_target)
{
	i_ref.q = new_target;
	ramp_active = false; // Wymuś wyłączenie rampy

	// Zsynchronizuj stan rampy, aby uniknąć nagłego skoku
	iq_current = new_target;
	ramp_i_ref.q = new_target;
}


void FOC_SetTorqueTarget(float torque_mNm)
{
    // Iq = (T_mNm / 1000) / Kt
    float target_iq = (torque_mNm / 1000.0f) / MOTOR_TORQUE_CONSTANT;
    FOC_SetIqTarget(target_iq);
}

void FOC_Update(float theta_el)
{
    float ialpha, ibeta;
    float id, iq;
    float vd, vq;
    float valpha, vbeta;
    float target_iq; // Lokalna zmienna dla celu PI

	if (ramp_active) {
		FOC_LinearRamp();
		target_iq = ramp_i_ref.q; // Użyj wyjścia z rampy
	} else {
		target_iq = i_ref.q; // Użyj globalnej wartości zadanej
	}

    float sin_theta = sinf(theta_el);
    float cos_theta = cosf(theta_el);

    // Clarke
    ClarkeTransform(currents.a, currents.b, &ialpha, &ibeta);

    // Park
    ParkTransformTrig(ialpha, ibeta, &sin_theta, &cos_theta, &id, &iq);

    // PI
    vd = pi_control(&pi_id, i_ref.d - id);
    vq = pi_control(&pi_iq, target_iq - iq);

    debug_id = id;
    debug_iq = iq;
    debug_id_ref = i_ref.d;
    debug_iq_ref = target_iq;
    debug_vd = vd;
    debug_vq = vq;
    debug_speed = actual_speed_rpm;

    // InvPark
    InvParkTransformTrig(vd, vq, &sin_theta, &cos_theta, &valpha, &vbeta);

    // SVPWM
    SVPWM_Update(valpha, vbeta);
}

void FOC_Stop(){
	// TODO: w trybie 6PWM trzeba wyłączyć wszystkie kanały + __HAL_TIM_MOE_DISABLE(foc_htim);
    HAL_TIM_PWM_Stop(foc_htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(foc_htim, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(foc_htim, TIM_CHANNEL_3);

    HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_RESET);

    HAL_ADCEx_InjectedStop(foc_hadc);
}

void FOC_Start(){

    HAL_TIM_Base_Start_IT(foc_htim);
    HAL_TIM_OC_Start(foc_htim, TIM_CHANNEL_4);
	HAL_ADCEx_InjectedStart_IT(foc_hadc);

	HAL_TIM_PWM_Start(foc_htim, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(foc_htim, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(foc_htim, TIM_CHANNEL_3);

	HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_SET);
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
    	// Synchronizacja kąta
    	if (g_new_encoder_data_ready)
		{
			g_new_encoder_data_ready = false;

			theta_mech_latest = AS5048_GetMechanicalAngle();
			theta_el_latest = FOC_GetElecticalAngle(theta_mech_latest);
		}

        // Pomiar prądu w tym cyklu
        CurrentSense_Process();
        CurrentSense_Read(&currents);

        // Estymacja prędkości
        SpeedEstimator_Update(theta_mech_latest, &actual_speed_rpm);

        // Pętla FOC
        FOC_Update(theta_el_latest);

        // Start spi do kolejnego cyklu
        if (g_spi_ready) {
            AS5048_ReadAngleDMA();
        }
        else{
        	// critical error, pętla foc jest szybsza niż SPI
        }
    }
}




