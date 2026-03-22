/*
 * foc_loop.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

#include <FOC/lut_sincos.h>
#include "App/config.h"
#include "FOC/foc_loop.h"
#include "FOC/controller_utils.h"
#include "FOC/speed_control.h"
#include "BSP/as5048a.h"
#include "math.h"
#include "main.h"

TIM_HandleTypeDef* foc_htim;
TIM_HandleTypeDef* enc_htim;
ADC_HandleTypeDef* foc_hadc;

// REGULATORY PI
static PI_Controller pi_id = { .kp = PI_KP_ID, .ki = PI_KI_ID, .limit = PI_LIMIT_ID, .integral = 0.0f, .dt = PWM_PERIOD_SEC};
static PI_Controller pi_iq = { .kp = PI_KP_IQ, .ki = PI_KI_IQ, .limit = PI_LIMIT_IQ, .integral = 0.0f, .dt = PWM_PERIOD_SEC};
volatile dq_ref_t i_ref = {0.0f, 0.0f};
static abc_current_t currents;

// RAMPA
#ifdef ENABLE_RAMP
	static const float iq_step = 0.0001f; // przyrost prądu na 1 krok
	static float iq_ramp_out;
#endif

// FLAGI
volatile bool ramp_active = false;
volatile bool spi_angle_ready = false;
volatile bool foc_data_ready = false;
volatile bool sensor_aligned = false;
volatile bool new_current_data_ready = false;
volatile bool encoder_prev_ready = false;

// ENKODER
static int sensor_direction = 0; // 1 - CW, -1 - CCW
static float zero_electric_angle = 0.0f;
volatile float theta_mech_latest = 0.0f;
volatile float theta_mech_latest_shifed = 0.0f;
volatile float theta_el_latest = 0.0f;

// Debug - cubemonitor
volatile MonitorData_t monitor_data __attribute__((section(".fixed_logs_section")));

volatile uint32_t foc_loop_ok = 0;
volatile uint32_t foc_loop_err = 0;
volatile uint32_t err_encoder = 0;
volatile uint32_t err_current = 0;

static inline void Log_To_CubeMonitor(float id, float iq, float target_iq)
{
    monitor_data.current_a = currents.a;
    monitor_data.current_b = currents.b;
    monitor_data.current_c = currents.c;

    monitor_data.id = id;
    monitor_data.iq = iq;
    monitor_data.iq_ref = target_iq;

    monitor_data.theta_el = theta_el_latest;
    monitor_data.theta_mech = theta_mech_latest;

    monitor_data.id_ref = i_ref.d;
    monitor_data.speed_ref = speed_ramp_out;
    monitor_data.speed = estimated_speed_rpm;
}

void FOC_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim_foc, TIM_HandleTypeDef *htim_enc, SPI_HandleTypeDef *hspi)
{
	printf("FOC: Init...\n");
	foc_htim = htim_foc;
	enc_htim = htim_enc;
	foc_hadc = hadc;

	AS5048_Init(hspi);

	HAL_TIM_Base_Stop_IT(foc_htim);
	HAL_TIM_Base_Stop_IT(enc_htim);

	__HAL_TIM_SET_COUNTER(foc_htim, 0);
	__HAL_TIM_SET_COUNTER(enc_htim, 0);

	HAL_TIM_Base_Start(foc_htim);
	HAL_TIM_Base_Start_IT(enc_htim);
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
}

void FOC_Start(){

    HAL_TIM_OC_Start(foc_htim, TIM_CHANNEL_4);
	HAL_ADCEx_InjectedStart_IT(foc_hadc);

	HAL_TIM_PWM_Start(foc_htim, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(foc_htim, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(foc_htim, TIM_CHANNEL_3);

	HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_SET);

	// Pobranie danych przed uruchomieniem pętli FOC
	new_current_data_ready = false;
	new_encoder_data_ready = false;
    spi_ready = true;

    err_current = 0;
    foc_loop_err = 0;
    foc_loop_ok = 0;

    AS5048_ReadAngleDMA();

    HAL_TIM_Base_Start_IT(foc_htim);
}

void FOC_Stop()
{

    HAL_TIM_PWM_Stop(foc_htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(foc_htim, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(foc_htim, TIM_CHANNEL_3);

    HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_RESET);

    HAL_ADCEx_InjectedStop_IT(foc_hadc);

    new_current_data_ready = false;
    encoder_prev_ready = false;
    spi_ready = false;

    ramp_active = false;

#ifdef ENABLE_RAMP
	iq_ramp_out = i_ref.q;
#endif

    i_ref.q = 0.0f;
    i_ref.d = 0.0f;

    pi_id.integral = 0.0f;
    pi_iq.integral = 0.0f;

    foc_loop_ok = 0;
    foc_loop_err = 0;
    err_current = 0;
}

void FOC_LinearRamp()
{
#ifdef ENABLE_RAMP
    if (ramp_active)
    {
        if (iq_ramp_out < i_ref.q)
        {
            iq_ramp_out += iq_step;
            if (iq_ramp_out > i_ref.q) {
                iq_ramp_out = i_ref.q;
                ramp_active = false; // Zakończ rampę
            }
        }
        else if (iq_ramp_out > i_ref.q)
        {
            iq_ramp_out -= iq_step;
            if (iq_ramp_out < i_ref.q) {
                iq_ramp_out = i_ref.q;
                ramp_active = false; // Zakończ rampę
            }
        }
        else
        {
            ramp_active = false;
        }
    }
#endif
}
void FOC_RunLoop()
{
	if (!new_current_data_ready)
	    {
	        foc_loop_err++;
	        return;
	    }

	new_current_data_ready = false;

	// --- Pętla FOC ---

	// Kąt (zapisany przez SPI DMA)
	if (encoder_prev_ready){
		encoder_prev_ready = false;

		theta_mech_latest = AS5048_GetMechanicalAngle();
		theta_mech_latest_shifed = AS5048_GetMechanicalAngleShifted();
		theta_el_latest = FOC_GetElecticalAngle(theta_mech_latest);
		// Estymacja predkosci
		SpeedEstimator_Update(theta_mech_latest_shifed);
	}

	// Prąd (zapisany przez ADC ISR)
	CurrentSense_CalculatePhases();
	CurrentSense_Read(&currents);

	// Pętla FOC
	FOC_Update(theta_el_latest);

	// Koniec pętli FOC ---
	foc_loop_ok++;
}

// Algorytm FOC
void FOC_Update(float theta_el)
{
    float ialpha, ibeta;
    float id, iq;
    float vd, vq;
    float valpha, vbeta;
    float target_iq; // Lokalna zmienna dla celu PI
    float sin_theta, cos_theta;

#ifdef ENABLE_RAMP
	if (ramp_active) {
		FOC_LinearRamp();
		target_iq = iq_ramp_out; // Użyj wyjścia z rampy
	} else {
		target_iq = i_ref.q; // Użyj globalnej wartości zadanej
	}
#else
    // Jeśli rampa wyłączona, użyj bezpośrednio wartości zadanej
    target_iq = i_ref.q;
#endif

    LUT_SinCos(theta_el, &sin_theta, &cos_theta); // Pobranie wartosci sin,cos z LUT

    // Clarke
    ClarkeTransform(currents.a, currents.b, &ialpha, &ibeta);

    // Park
    ParkTransform(ialpha, ibeta, &sin_theta, &cos_theta, &id, &iq);

    // PI
    vd = pi_control(&pi_id, i_ref.d - id);
    vq = pi_control(&pi_iq, target_iq - iq);

    // CubeMonitor log data
    Log_To_CubeMonitor(id, iq, target_iq);

    // InvPark
    InvParkTransform(vd, vq, &sin_theta, &cos_theta, &valpha, &vbeta);

    // SVPWM
    SVPWM_Update(valpha, vbeta);
}


inline float FOC_GetElecticalAngle(float mech)
{
    return normalize_angle((float)(sensor_direction * MOTOR_POLE_PAIRS) * mech - zero_electric_angle);
}

void FOC_SetIqTarget_Ramp(float new_target)
{
#ifdef ENABLE_RAMP
	i_ref.q = new_target;
    ramp_active = true;
#else
    // Jeśli rampa wyłączona, ustaw wartość natychmiast
    FOC_SetIqTarget(new_target);
#endif
}

void FOC_SetIqTarget(float new_target)
{
	i_ref.q = new_target;

#ifdef ENABLE_RAMP
	ramp_active = false; // Wymuś wyłączenie rampy
	// Zsynchronizuj stan rampy, aby uniknąć nagłego skoku przy kolejnym włączeniu
	iq_ramp_out = new_target;
#endif
}


void FOC_SetTorqueTarget(float torque_mNm)
{
    // Iq = (T_mNm / 1000) / Kt
    float target_iq = (torque_mNm / 1000.0f) / MOTOR_TORQUE_CONSTANT;
    FOC_SetIqTarget(target_iq);
}

// Funkcja używana tylko do kalibracji, w FOC_Align_Sensor()
void FOC_SetPhaseVoltage(float Uq, float Ud, float angle_el)
{
    // Ograniczenie wektora napięcia
    float Uref = sqrtf(Ud * Ud + Uq * Uq);
    float Umax = VOLTAGE_SUPPLY / M_SQRT3;

    if (Uref > Umax) {
        float scale = Umax / Uref;
        Ud *= scale;
        Uq *= scale;
    }

    float sin_t, cos_t;
    LUT_SinCos(angle_el, &sin_t, &cos_t); // Pobranie sin cos z tablicy LUT


    float Ualpha, Ubeta;
    InvParkTransform(Ud, Uq, &sin_t, &cos_t, &Ualpha, &Ubeta);

    float Ua, Ub, Uc;
    InvClarkeTransform(Ualpha, Ubeta, &Ua, &Ub, &Uc);

    // Mapowanie na PWM dla drivera 3-PWM ---
    float dc_a = Ua / VOLTAGE_SUPPLY;
    float dc_b = Ub / VOLTAGE_SUPPLY;
    float dc_c = Uc / VOLTAGE_SUPPLY;

    // Dodajemy 0.5, aby przesunąć zakres. Centrowanie dla drivera 3-PWM.
    dc_a += 0.5f;
    dc_b += 0.5f;
    dc_c += 0.5f;

    // Przeliczamy współczynniki wypełnienia (0.0 do 1.0) na wartości dla rejestru timera.
    uint32_t pwm_a = (uint32_t)(dc_a * PWM_PERIOD_ARR);
    uint32_t pwm_b = (uint32_t)(dc_b * PWM_PERIOD_ARR);
    uint32_t pwm_c = (uint32_t)(dc_c * PWM_PERIOD_ARR);

    // Zabezpieczenie
    if (pwm_a > PWM_PERIOD_ARR) pwm_a = PWM_PERIOD_ARR;
    if (pwm_b > PWM_PERIOD_ARR) pwm_b = PWM_PERIOD_ARR;
    if (pwm_c > PWM_PERIOD_ARR) pwm_c = PWM_PERIOD_ARR;

    // Ustawienie wartości w rejestrach timera
    __HAL_TIM_SET_COMPARE(foc_htim, TIM_CHANNEL_1, pwm_a);
    __HAL_TIM_SET_COMPARE(foc_htim, TIM_CHANNEL_2, pwm_b);
    __HAL_TIM_SET_COMPARE(foc_htim, TIM_CHANNEL_3, pwm_c);
}

bool FOC_AlignSensor()
{
	static float _3PI_2 = 4.71238898038f;

    if (sensor_aligned) {
        return true;
    }

    printf("\n--- Start kalibracji sensora (SimpleFOC) ---\n");
    printf("Krok 1: Wykrywanie kierunku...\n");

    // --- Obrót w przód ---
    for (int i = 0; i <= 500; i++) {
        float angle = _3PI_2 + (i * (M_TWOPI / 500.0f));
        FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, angle);
        HAL_Delay(2);
    }

    float mid_angle = AS5048_GetAngleRad();
    if (mid_angle < 0.0f) {
        printf("Błąd: odczyt kąta (mid)\n");
        return (sensor_aligned = false);
    }

    // --- Obrót w tył ---
    for (int i = 500; i >= 0; i--) {
        float angle = _3PI_2 + (i * (M_TWOPI / 500.0f));
        FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, angle);
        HAL_Delay(2);
    }

    float end_angle = AS5048_GetAngleRad();
    if (end_angle < 0.0f) {
        printf("Błąd: odczyt kąta (end)\n");
        return (sensor_aligned = false);
    }

    // Analiza ruchu
    float moved = mid_angle - end_angle;

    if (moved < -M_PI) moved += M_TWOPI;
    if (moved >  M_PI) moved -= M_TWOPI;

    if (fabsf(moved) < 0.1f) {
        printf("Błąd: silnik nie poruszył się!\n");
        return (sensor_aligned = false);
    }

    sensor_direction = (moved > 0) ? SENSOR_DIRECTION_CCW : SENSOR_DIRECTION_CW;
    printf("Kierunek sensora: %d (%s)\n",
           sensor_direction,
           sensor_direction == SENSOR_DIRECTION_CW ? "CW" : "CCW");

    // Sprawdzenie par biegunów
    float expected = M_TWOPI / MOTOR_POLE_PAIRS;
    if (fabsf(fabsf(moved) - expected) > 0.5f) {
        printf("Ostrzeżenie: możliwy błąd liczby par biegunów\n");
    } else {
        printf("Weryfikacja par biegunów: OK\n");
    }

    printf("Krok 2: Wyrównywanie do zera elektrycznego...\n");

    // Ustaw znaną elektryczną pozycję
    FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, _3PI_2);
    HAL_Delay(700);

    float mech_angle = AS5048_GetAngleRad();
    if (mech_angle < 0.0f) {
        printf("Błąd odczytu kąta przy wyznaczaniu zera.\n");
        return (sensor_aligned = false);
    }

    // Kąt elektryczny
    float el_angle = normalize_angle(
        (float)sensor_direction * MOTOR_POLE_PAIRS * mech_angle
    );

    zero_electric_angle = normalize_angle(el_angle - _3PI_2);

    printf("Offset elektryczny: %.4f rad\n", zero_electric_angle);

    // Zatrzymanie silnika
    FOC_SetPhaseVoltage(0, 0, 0);
    printf("--- Kalibracja zakończona pomyślnie! ---\n\n");

    return (sensor_aligned = true);
}


