/*
 * foc_loop.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

#include "motor_algorithm.h"
#include "lut_sincos.h"
#include "foc_loop.h"
#include "foc_utils.h"
#include "speed_control.h"
#include "speed_estimator.h"
#include "position_control.h"
#include "config.h"
#include "as5048a.h"
#include "encoder_hub.h"
#include "board.h"
#include "math.h"
#include "main.h"
#include <string.h>
#include "svpwm.h"

// Włącz jeśli SVPWM ma zamianę faz B↔C
// #define SVPWM_PHASE_SWAP_BC
// #define CALIB_SVPWM

PI_FOC_State_t s_foc;

extern BoardHandleTypeDef board;

// RAMPA
#define RAMP_STEP_DEFAULT 0.0001f

// FLAGI
volatile bool currents_ready = false;

static void FOC_LinearRamp(PI_FOC_State_t *state, float target_iq);
static void PI_Reset(PI_Controller *pi);
static void Ramp_Reset(Ramp_t *ramp);
static void Flags_Reset(FocFlags_t *flags);
static void FOCStats_Reset(void);

void FOC_Init(void *ctx)
{
	printf("FOC: Init...\n");

    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;

    state->pi_id = (PI_Controller){
        .kp = PI_KP_ID,
        .ki = PI_KI_ID,
        .limit = PI_LIMIT_ID,
        .integral = 0.0f,
        .dt = PWM_PERIOD_SEC
    };

    state->pi_iq = (PI_Controller){
        .kp = PI_KP_IQ,
        .ki = PI_KI_IQ,
        .limit = PI_LIMIT_IQ,
        .integral = 0.0f,
        .dt = PWM_PERIOD_SEC
    };

    state->i_ref = (dq_ref_t){0.0f, 0.0f};

    #ifdef ENABLE_RAMP
        state->ramp.step = RAMP_STEP_DEFAULT;
        state->ramp.output = 0.0f;
        state->ramp.active = false;
    #endif
}

void FOC_Start(void *ctx){
	// Pobranie danych przed uruchomieniem pętli FOC
	currents_ready = false;
    spi_ready = true;

    FOCStats_Reset();

    AS5048_ReadAngleDMA();
}

void FOC_Stop(void *ctx){

    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;

    Flags_Reset(&state->flags);

    state->ramp.active = false;

#ifdef ENABLE_RAMP
    state->ramp.output = state->i_ref.q;
#endif  

    state->i_ref = (dq_ref_t){0.0f, 0.0f};

    PI_Reset(&state->pi_id);
    PI_Reset(&state->pi_iq);

    FOCStats_Reset();
}

static void FOC_Update(void *ctx, const Motor_Measurements_t *meas, const Motor_References_t *ref, Motor_Output_t *out)
{
    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;
    float sin_theta, cos_theta;
    float target_iq;

#ifdef ENABLE_RAMP
    // Odpowiednik starego FOC_SetIqTarget_Ramp - automatycznie załącza rampę 
    // jeśli zadana wartość (ref) zmieniła się względem aktualnego wyjścia rampy
    if (fabsf(state->ramp.output - ref->torque_iq_ref) > 0.001f && !state->ramp.active) {
        state->ramp.active = true;
    }

    if (state->ramp.active) {
        FOC_LinearRamp(state, ref->torque_iq_ref);
        target_iq = state->ramp.output;
    } else {
        target_iq = ref->torque_iq_ref;
        state->ramp.output = target_iq; // Synchronizacja
    }
#else
    target_iq = ref->torque_iq_ref;
#endif

    LUT_SinCos(meas->theta_el, &sin_theta, &cos_theta);

#ifdef SVPWM_PHASE_SWAP_BC
    ClarkeTransform(meas->currents.a, meas->currents.c, &state->i_alpha, &state->i_beta);
#else
    ClarkeTransform(meas->currents.a, meas->currents.b, &state->i_alpha, &state->i_beta);
#endif
    
    ParkTransform(state->i_alpha, state->i_beta, &sin_theta, &cos_theta, &state->id, &state->iq);
    
    // Obliczenia PI - wpisane sztywne Id = 0.0f
    state->v_d = pi_control(&state->pi_id, state->i_ref.d - state->id);
    state->v_q = pi_control(&state->pi_iq, target_iq - state->iq);
    
    InvParkTransform(state->v_d, state->v_q, &sin_theta, &cos_theta, &state->v_alpha, &state->v_beta);
    
    SVPWM_Update(state->v_alpha, state->v_beta);
}

ControlAlgorithm_t PI_FOC_Create(void)
{
    ControlAlgorithm_t algo = {
        .ctx = &s_foc,
        .Init = FOC_Init,
        .Start = FOC_Start,
        .Stop = FOC_Stop,
        .Update = FOC_Update, // Tym zajmiemy się w następnym kroku
        .GetTelemetry = FOC_GetTelemetry
    };
    return algo;
}

// void FOC_Update(float theta_el)
// {
//     float ialpha, ibeta;
//     float id, iq;
//     float vd, vq;
//     float valpha, vbeta;
//     float target_iq; // Lokalna zmienna dla celu PI
//     float sin_theta, cos_theta;

// #ifdef ENABLE_RAMP
// 	if (s_foc.ramp.active) {
// 		FOC_LinearRamp();
// 		target_iq = s_foc.ramp.output; // Użyj wyjścia z rampy
// 	} else {
// 		target_iq = s_foc.i_ref.q; // Użyj globalnej wartości zadanej
// 	}
// #else
//     // Jeśli rampa wyłączona, użyj bezpośrednio wartości zadanej
//     target_iq = s_foc.i_ref.q;
// #endif

//     LUT_SinCos(theta_el, &sin_theta, &cos_theta); // Pobranie wartosci sin,cos z LUT

// #ifdef SVPWM_PHASE_SWAP_BC
//     // Kompensacja zamiany faz w SVPWM
//     ClarkeTransform(s_foc.currents.a, s_foc.currents.c, &ialpha, &ibeta);
// #else
//     ClarkeTransform(s_foc.currents.a, s_foc.currents.b, &ialpha, &ibeta);
// #endif

//     ParkTransform(ialpha, ibeta, &sin_theta, &cos_theta, &id, &iq);

//     vd = pi_control(&s_foc.pi_id, s_foc.i_ref.d - id);
//     vq = pi_control(&s_foc.pi_iq, target_iq - iq);

//     // static uint32_t cnt = 0;
//     // if (++cnt % 5000 == 0) {
//     //     printf("FOC: theta=%.2f id=%.3f iq=%.3f vd=%.2f vq=%.2f\n",
//     //            theta_el, id, iq, vd, vq);
//     //     printf("     Ia=%.3f Ib=%.3f Ic=%.3f\n",
//     //            s_foc.currents.a, s_foc.currents.b, s_foc.currents.c);
//     // }

//     // CubeMonitor log data
//     Log_To_CubeMonitor(id, iq, target_iq);

//     InvParkTransform(vd, vq, &sin_theta, &cos_theta, &valpha, &vbeta);
//     SVPWM_Update(valpha, vbeta);
// }

// float FOC_GetElectricalAngle(float mech)
// {
//     return normalize_angle((float)(s_foc.calib.direction * MOTOR_POLE_PAIRS) * mech - s_foc.calib.zero_electric_angle);
// }

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

static void FOC_GetTelemetry(const void *ctx, Motor_Telemetry_t *telem)
{
    const PI_FOC_State_t *state = (const PI_FOC_State_t*)ctx;
    
    telem->id_meas = state->id;
    telem->iq_meas = state->iq;
    telem->id_ref = state->i_ref.d; // Uaktualnione referencje, jesli dodasz je do zapisu w stanie
    telem->iq_ref = state->ramp.active ? state->ramp.output : state->i_ref.q;
    telem->vd_out = state->v_d;
    telem->vq_out = state->v_q;
    telem->active_algo = 1; // 1 to PI-FOC
}

static void FOC_LinearRamp(PI_FOC_State_t *state, float target_iq)
{
#ifdef ENABLE_RAMP
    if (state->ramp.active)
    {
        if (state->ramp.output < target_iq)
        {
            state->ramp.output += state->ramp.step;
            if (state->ramp.output > target_iq) {
                state->ramp.output = target_iq;
                state->ramp.active = false;
            }
        }
        else if (state->ramp.output > target_iq)
        {
            state->ramp.output -= state->ramp.step;
            if (state->ramp.output < target_iq) {
                state->ramp.output = target_iq;
                state->ramp.active = false;
            }
        }
        else
        {
            state->ramp.active = false;
        }
    }
#endif
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

static void FOCStats_Reset(void)
{
    s_foc.stats.loop_ok  = 0;
    s_foc.stats.loop_err = 0;
    s_foc.stats.err_current = 0;
}

void Motor_Motion_Test(void)
{
    static uint32_t t0 = 0;

    float Uq = 0.6f * (VOLTAGE_SUPPLY / M_SQRT3);
    float Ud = 0.0f;

    uint32_t t_ms = HAL_GetTick() - t0;   // czas od startu w ms
    float t = t_ms * 0.001f;              // sekundy

    float freq = 20.0f;                   // 20 Hz elektryczne
    float angle = 2.0f * M_PI * freq * t;

    // zawijanie kąta (opcjonalne)
    angle = fmodf(angle, 2.0f * M_PI);

    FOC_SetPhaseVoltage(Uq, Ud, angle);
}
