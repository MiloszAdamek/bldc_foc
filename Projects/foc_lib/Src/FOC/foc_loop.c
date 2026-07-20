/*
 * foc_loop.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

#include <FOC/lut_sincos.h>
#include "FOC/foc_loop.h"
#include "FOC/foc_utils.h"
#include "FOC/speed_control.h"
#include "FOC/speed_estimator.h"
#include "FOC/position_control.h"
#include "App/config.h"
#include "BSP/as5048a.h"
#include "BSP/encoder_hub.h"
#include "BSP/powerstage.h"
#include "math.h"
#include "main.h"
#include <string.h>

// Włącz jeśli SVPWM ma zamianę faz B↔C
//#define SVPWM_PHASE_SWAP_BC
//#define CALIB_SVPWM

static FOC_HandleTypeDef s_foc;

#define ENCODER_TIMEOUT_LIMIT 100
// RAMPA
#define RAMP_STEP_DEFAULT 0.0001f

// FLAGI
volatile bool currents_ready = false;

// Debug - cubemonitor
volatile MonitorData_t monitor_data __attribute__((section(".fixed_logs_section")));

static void FOC_LinearRamp(void);
static void FOC_EnableOutputs(void);
static void FOC_DisableOutputs(void);
static void PI_Reset(PI_Controller *pi);
static void Ramp_Reset(Ramp_t *ramp);
static void Flags_Reset(FocFlags_t *flags);
static void FocStats_Reset(void);

static inline void Log_To_CubeMonitor(float id, float iq, float target_iq)
{
    monitor_data.current_a = s_foc.currents.a;
    monitor_data.current_b = s_foc.currents.b;
    monitor_data.current_c = s_foc.currents.c;
//
//    monitor_data.id = id;
//    monitor_data.iq = iq;
//    monitor_data.iq_ref = target_iq;

//    monitor_data.theta_el =  s_foc.angles.theta_el;
    monitor_data.theta_mech = s_foc.angles.theta_mech;

//    monitor_data.id_ref = i_ref.d;
//    monitor_data.speed_ref = speed_ramp_out;
    monitor_data.speed = SpeedEstimator_GetOmegaRPM();
//
//    monitor_data.position_err = position_err;
//    monitor_data.position_ref = position_ref;
//    monitor_data.position_reg_out = position_reg_out;
}

void FOC_Init(BoardHandleTypeDef *board)
{
	printf("FOC: Init...\n");
    s_foc.board = board;

    s_foc.pi_id = (PI_Controller){
        .kp = PI_KP_ID, .ki = PI_KI_ID,
        .limit = PI_LIMIT_ID, .integral = 0.0f, .dt = PWM_PERIOD_SEC
    };
    s_foc.pi_iq = (PI_Controller){
        .kp = PI_KP_IQ, .ki = PI_KI_IQ,
        .limit = PI_LIMIT_IQ, .integral = 0.0f, .dt = PWM_PERIOD_SEC
    };
    s_foc.i_ref = (dq_ref_t){0.0f, 0.0f};

#ifdef ENABLE_RAMP
    s_foc.ramp.step = RAMP_STEP_DEFAULT;
    s_foc.ramp.output = 0.0f;
    s_foc.ramp.active = false;
#endif

	// AS5048_Init(s_foc.board->hspi_enc);
    PowerStage_Init();

	HAL_TIM_Base_Stop_IT(s_foc.board->htim_pwm);
	HAL_TIM_Base_Stop_IT(s_foc.board->htim_enc);

	__HAL_TIM_SET_COUNTER(s_foc.board->htim_pwm, 0);
	__HAL_TIM_SET_COUNTER(s_foc.board->htim_enc, 0);

	HAL_TIM_Base_Start(s_foc.board->htim_pwm);
	HAL_TIM_Base_Start_IT(s_foc.board->htim_enc);
	HAL_TIM_OC_Start(s_foc.board->htim_pwm, TIM_CHANNEL_4); 	// Start CH4 -> wyzwalanie ADC

	HAL_Delay(50);

	CurrentSense_Init(s_foc.board->hadc_curr);

    // SVPWM_Init(s_foc.board->htim_pwm);
    // FOC_AlignSensor();

    // Odczyt kąta przed uruchomieniem pętli FOC
//    float mech0 = AS5048_GetAngleRad();
//    if (mech0 >= 0.0f) {
//        s_foc.angles.theta_mech = mech0;
//        s_foc.angles.theta_el = FOC_GetElecticalAngle(mech0);
//    }

    HAL_TIM_Base_Stop(s_foc.board->htim_pwm);
}

void FOC_Start(){

//    HAL_TIM_OC_Start(s_foc.board->htim_pwm, TIM_CHANNEL_4);
//	HAL_ADCEx_InjectedStart_IT(s_foc.board->hadc_curr);

//	FOC_EnableOutputs();

	// Pobranie danych przed uruchomieniem pętli FOC
	currents_ready = false;
    spi_ready = true;

    FocStats_Reset();

    AS5048_ReadAngleDMA();

    HAL_TIM_Base_Start_IT(s_foc.board->htim_pwm);
}

void FOC_Stop(){

    FOC_DisableOutputs();

    HAL_ADCEx_InjectedStop_IT(s_foc.board->hadc_curr);

    Flags_Reset(&s_foc.flags);

    s_foc.ramp.active = false;

#ifdef ENABLE_RAMP
    s_foc.ramp.output = s_foc.i_ref.q;
#endif

    s_foc.i_ref = (dq_ref_t){0.0f, 0.0f};

    PI_Reset(&s_foc.pi_id);
    PI_Reset(&s_foc.pi_iq);

    FocStats_Reset();
}

void FOC_RunLoop()
{
//	if (!currents_ready){s_foc.stats.loop_err++; return;}
//	currents_ready = false;

	// Kąt (zapisany przez SPI DMA)
	EncoderSample_t enc;
	if(EncoderHub_ConsumeSample(&enc)){
		s_foc.angles.theta_mech = enc.theta_mech;
		s_foc.angles.theta_el = FOC_GetElecticalAngle(enc.theta_mech);
		s_foc.stats.encoder_timeout = 0;

		EncoderHub_PublishAngle(
			s_foc.angles.theta_mech,
			s_foc.angles.theta_el
		);
	} else {
		s_foc.stats.encoder_stale++;
		s_foc.stats.encoder_timeout++;

		if (s_foc.stats.encoder_timeout > ENCODER_TIMEOUT_LIMIT){
			FOC_Stop();
			return;
		}
		// Użycie wartości kąta z poprzedniej iteracji
	}

	Log_To_CubeMonitor(0.0f, 0.0f, 0.0f);

	// Prąd (zapisany przez ADC ISR)
	CurrentSense_CalculatePhases();
	CurrentSense_Read(&s_foc.currents);

	// Algorytm FOC
//	FOC_Update(s_foc.angles.theta_el);

//	s_foc.stats.loop_ok++;
}

void FOC_Update(float theta_el)
{
    float ialpha, ibeta;
    float id, iq;
    float vd, vq;
    float valpha, vbeta;
    float target_iq; // Lokalna zmienna dla celu PI
    float sin_theta, cos_theta;

#ifdef ENABLE_RAMP
	if (s_foc.ramp.active) {
		FOC_LinearRamp();
		target_iq = s_foc.ramp.output; // Użyj wyjścia z rampy
	} else {
		target_iq = s_foc.i_ref.q; // Użyj globalnej wartości zadanej
	}
#else
    // Jeśli rampa wyłączona, użyj bezpośrednio wartości zadanej
    target_iq = s_foc.i_ref.q;
#endif

    LUT_SinCos(theta_el, &sin_theta, &cos_theta); // Pobranie wartosci sin,cos z LUT

#ifdef SVPWM_PHASE_SWAP_BC
    // Kompensacja zamiany faz w SVPWM
    ClarkeTransform(s_foc.currents.a, s_foc.currents.c, &ialpha, &ibeta);
#else
    ClarkeTransform(s_foc.currents.a, s_foc.currents.b, &ialpha, &ibeta);
#endif

    ParkTransform(ialpha, ibeta, &sin_theta, &cos_theta, &id, &iq);

    vd = pi_control(&s_foc.pi_id, s_foc.i_ref.d - id);
    vq = pi_control(&s_foc.pi_iq, target_iq - iq);

    static uint32_t cnt = 0;
    if (++cnt % 5000 == 0) {
        printf("FOC: theta=%.2f id=%.3f iq=%.3f vd=%.2f vq=%.2f\n",
               theta_el, id, iq, vd, vq);
        printf("     Ia=%.3f Ib=%.3f Ic=%.3f\n",
               s_foc.currents.a, s_foc.currents.b, s_foc.currents.c);
    }

    // CubeMonitor log data
    Log_To_CubeMonitor(id, iq, target_iq);

    InvParkTransform(vd, vq, &sin_theta, &cos_theta, &valpha, &vbeta);
    SVPWM_Update(valpha, vbeta);
}

inline float FOC_GetElecticalAngle(float mech)
{
    return normalize_angle((float)(s_foc.calib.direction * MOTOR_POLE_PAIRS) * mech - s_foc.calib.zero_electric_angle);
}

void FOC_SetIqTarget_Ramp(float new_target)
{
#ifdef ENABLE_RAMP
	i_ref.q = new_target;
    s_foc.ramp.active = true;
#else
    // Jeśli rampa wyłączona, ustaw wartość natychmiast
    FOC_SetIqTarget(new_target);
#endif
}

void FOC_SetIqTarget(float new_target)
{
	s_foc.i_ref.q = new_target;

#ifdef ENABLE_RAMP
	s_foc.ramp.active = false; // Wymuś wyłączenie rampy
	// Zsynchronizuj stan rampy, aby uniknąć nagłego skoku przy kolejnym włączeniu
	s_foc.ramp.output = new_target;
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
    __HAL_TIM_SET_COMPARE(s_foc.board->htim_pwm, TIM_CHANNEL_1, pwm_a);
    __HAL_TIM_SET_COMPARE(s_foc.board->htim_pwm, TIM_CHANNEL_2, pwm_b);
    __HAL_TIM_SET_COMPARE(s_foc.board->htim_pwm, TIM_CHANNEL_3, pwm_c);
}

static void FOC_ApplyVoltageVector(float Uq, float Ud, float theta_el)
{
    float sin_t, cos_t;
    LUT_SinCos(theta_el, &sin_t, &cos_t);

    float Ualpha, Ubeta;
    InvParkTransform(Ud, Uq, &sin_t, &cos_t, &Ualpha, &Ubeta);

    SVPWM_Update(Ualpha, Ubeta);
}

bool FOC_AlignSensor()
{
    if (s_foc.calib.aligned) {
        return true;
    }

    FOC_EnableOutputs();

    printf("\n--- Start kalibracji sensora (SimpleFOC) ---\n");
    printf("Krok 1: Wykrywanie kierunku...\n");

    // --- Obrót w przód ---
    for (int i = 0; i <= 500; i++) {
        float theta = _3PI_2 + (i * (M_TWOPI / 500.0f));

#ifdef CALIB_SVPWM
        FOC_ApplyVoltageVector(0, VOLTAGE_SENSOR_ALIGN, theta);
#else
        FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, theta);
#endif

        HAL_Delay(2);
    }

    float mid_angle = AS5048_GetAngleRad();
    if (mid_angle < 0.0f) {
        printf("Błąd: odczyt kąta (mid)\n");
        return (s_foc.calib.aligned = false);
    }

    // --- Obrót w tył ---
    for (int i = 500; i >= 0; i--) {
        float theta = _3PI_2 + (i * (M_TWOPI / 500.0f));

#ifdef CALIB_SVPWM
        FOC_ApplyVoltageVector(0, VOLTAGE_SENSOR_ALIGN, theta);
#else
        FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, theta);
#endif

        HAL_Delay(2);
    }

    float end_angle = AS5048_GetAngleRad();
    if (end_angle < 0.0f) {
        printf("Błąd: odczyt kąta (end)\n");
        return (s_foc.calib.aligned = false);
    }

    // Analiza ruchu
    float moved = mid_angle - end_angle;

    if (moved < -M_PI) moved += M_TWOPI;
    if (moved >  M_PI) moved -= M_TWOPI;

    if (fabsf(moved) < 0.1f) {
        printf("Błąd: silnik nie poruszył się!\n");
        return (s_foc.calib.aligned = false);
    }

    s_foc.calib.direction = (moved > 0) ? SENSOR_DIRECTION_CCW : SENSOR_DIRECTION_CW;
    printf("Kierunek sensora: %d (%s)\n",
           s_foc.calib.direction,
           s_foc.calib.direction == SENSOR_DIRECTION_CW ? "CW" : "CCW");

    // Sprawdzenie par biegunów
    float expected = M_TWOPI / MOTOR_POLE_PAIRS;
    if (fabsf(fabsf(moved) - expected) > 0.5f) {
        printf("Ostrzeżenie: możliwy błąd liczby par biegunów\n");
    } else {
        printf("Weryfikacja par biegunów: OK\n");
    }

    printf("Krok 2: Wyrównywanie do zera elektrycznego...\n");

    // Ustaw znaną elektryczną pozycję
#ifdef CALIB_SVPWM
    FOC_ApplyVoltageVector(0, VOLTAGE_SENSOR_ALIGN, _3PI_2);
#else
    FOC_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, _3PI_2);
#endif

    HAL_Delay(700);

    float mech_angle = AS5048_GetAngleRad();
    if (mech_angle < 0.0f) {
        printf("Błąd odczytu kąta przy wyznaczaniu zera.\n");
        return (s_foc.calib.aligned = false);
    }

    // Kąt elektryczny
    float el_angle = normalize_angle(
        (float)s_foc.calib.direction * MOTOR_POLE_PAIRS * mech_angle
    );

    s_foc.calib.zero_electric_angle = normalize_angle(el_angle - _3PI_2);

    printf("Offset elektryczny: %.4f rad\n", s_foc.calib.zero_electric_angle);

    // Zatrzymanie silnika
#ifdef CALIB_SVPWM
//    FOC_ApplyVoltageVector(0, 0, 0);
    SVPWM_Update(0, 0);
#else
	FOC_SetPhaseVoltage(0, 0, 0);
#endif

    printf("--- Kalibracja zakończona pomyślnie! ---\n\n");

    printf("=== DEBUG ===\n");
    printf("mid_angle: %.4f rad\n", mid_angle);
    printf("end_angle: %.4f rad\n", end_angle);
    printf("moved: %.4f rad\n", moved);
    printf("mech_angle (final): %.4f rad\n", mech_angle);
    printf("el_angle: %.4f rad\n", el_angle);
    printf("zero_electric_angle: %.4f rad\n", s_foc.calib.zero_electric_angle);
    printf("=============\n");

    return (s_foc.calib.aligned = true);
}

bool FOC_IsSensorAligned(void)
{
    return s_foc.calib.aligned;
}

static void FOC_LinearRamp(void)
{
#ifdef ENABLE_RAMP
    if (s_foc.ramp.active)
    {
        if (s_foc.ramp.output < s_foc.i_ref.q)
        {
        	s_foc.ramp.output += s_foc.ramp.step;
            if (s_foc.ramp.output > s_foc.i_ref.q) {
            	s_foc.ramp.output = s_foc.i_ref.q;
                s_foc.ramp.active = false; // Zakończ rampę
            }
        }
        else if (s_foc.ramp.output > i_ref.q)
        {
        	s_foc.ramp.output -= s_foc.ramp.step;
            if (s_foc.ramp.output < s_foc.i_ref.q) {
            	s_foc.ramp.output = s_foc.i_ref.q;
                s_foc.ramp.active = false; // Zakończ rampę
            }
        }
        else
        {
            s_foc.ramp.active = false;
        }
    }
#endif
}

static void FOC_EnableOutputs(void)
{
	PowerStage_StartPWM(s_foc.board->htim_pwm);
	PowerStage_On();
}

static void FOC_DisableOutputs(void)
{
	PowerStage_StopPWM(s_foc.board->htim_pwm);
	PowerStage_Off();
}

static void PI_Reset(PI_Controller *pi)
{
    pi->integral = 0.0f;
}

static void Ramp_Reset(Ramp_t *ramp)
{
    ramp->output = 0.0f;
    ramp->active = false;
}

static void Flags_Reset(FocFlags_t *flags)
{
    memset(flags, 0, sizeof(*flags));
}

static void FocStats_Reset(void)
{
    s_foc.stats.loop_ok  = 0;
    s_foc.stats.loop_err = 0;
    s_foc.stats.err_current = 0;
}

TIM_HandleTypeDef* FOC_GetPwmTimer(void) {return s_foc.board->htim_pwm;};
TIM_HandleTypeDef* FOC_GetEncTimer(void) {return s_foc.board->htim_enc;};

void Motor_Motion_Test(void)
{
    static float angle = 0.0f;

    float Uq = 0.2f * (VOLTAGE_SUPPLY / M_SQRT3);
    float Ud = 0.0f;

    angle += 0.001f;   // im mniejsze, tym wolniej

    if (angle > 2.0f * M_PI)
        angle -= 2.0f * M_PI;

    FOC_SetPhaseVoltage(Uq, Ud, angle);
}

