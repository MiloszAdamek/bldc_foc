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
#include "config.h"
#include "board.h"
#include <string.h>
#include "svpwm.h"
#include "stdio.h"
#include "as5048a.h"

extern BoardHandleTypeDef board;

static PI_FOC_State_t s_foc;

extern Motor_Calibration_t g_calibration;
extern Motor_Stats_t g_stats;

extern volatile Motor_Measurements_t g_meas;

// RAMPA
#define RAMP_STEP_DEFAULT 0.0001f

// FLAGI
volatile bool currents_ready = false;
extern volatile bool spi_ready;

#ifdef ENABLE_RAMP
    static void FOC_LinearRamp(PI_FOC_State_t *state, const Motor_References_t *ref);
#endif

static void PI_Reset(PI_Controller *pi);
static void Flags_Reset(FocFlags_t *flags);
static void FOC_StatsReset(void);
static void FOC_GetTelemetry(const void *ctx, Motor_Telemetry_t *telem);
static void FOC_Update(void *ctx, const Motor_Measurements_t *meas, const Motor_References_t *ref, Motor_Output_t *out);

ControlAlgorithm_t PI_FOC_Create(void)
{
    ControlAlgorithm_t algo = {
        .ctx = &s_foc,
        .Init = FOC_Init,
        .Start = FOC_Start,
        .Stop = FOC_Stop,
        .Update = FOC_Update,
        .GetTelemetry = FOC_GetTelemetry
    };
    return algo;
}

void FOC_Init(void *ctx)
{
	printf("FOC: Init...\n");

    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;

    state->pi_id = (PI_Controller){
        .kp = PI_KP_ID,
        .ki = PI_KI_ID,
        .limit = PI_LIMIT_ID,
        .integral = 0.0f,
        .dt = FOC_PERIOD_SEC
    };

    state->pi_iq = (PI_Controller){
        .kp = PI_KP_IQ,
        .ki = PI_KI_IQ,
        .limit = PI_LIMIT_IQ,
        .integral = 0.0f,
        .dt = FOC_PERIOD_SEC
    };

    state->id_setpoint = 0.0f;
    state->iq_setpoint = 0.0f;

    #ifdef ENABLE_RAMP
        state->ramp.step = RAMP_STEP_DEFAULT;
        state->ramp.output = 0.0f;
        state->ramp.active = false;
    #endif
}

void FOC_Start(void *ctx){
    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;

    PI_Reset(&state->pi_id);
    PI_Reset(&state->pi_iq);

    state->id_setpoint = 0.0f;
    state->iq_setpoint = 0.0f;
    state->iq_target_raw = 0.0f;

    #ifdef ENABLE_RAMP
        state->ramp.active = false;
        state->ramp.output = 0.0f; 
    #endif

    currents_ready = false;
    spi_ready = true;

    FOC_StatsReset();
    
    // Rozpoczęcie pobierania danych
    AS5048_ReadAngleDMA();
}

void FOC_Stop(void *ctx){

    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;

    Flags_Reset(&state->flags);
    
    #ifdef ENABLE_RAMP
        state->ramp.active = false;
        state->ramp.output = 0.0f;
    #endif  

    state->id_setpoint = 0.0f;
    state->iq_setpoint = 0.0f;
    state->iq_target_raw = 0.0f;

    PI_Reset(&state->pi_id);
    PI_Reset(&state->pi_iq);

    FOC_StatsReset();
}

static void FOC_Update(void *ctx, const Motor_Measurements_t *meas, const Motor_References_t *ref, Motor_Output_t *out)
{
    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;
    float sin_theta, cos_theta;

    state->iq_target_raw = ref->torque_iq_ref;

    #ifdef ENABLE_RAMP
        if (ref->iq_ramp_enabled) {
            // Rampa FOC WŁĄCZONA (Tryb Momentu)

            // Jeśli zadana wartość (ref) zmieniła się względem aktualnego wyjścia rampy
            if (fabsf(state->ramp.output - state->iq_target_raw) > 0.001f && !state->ramp.active) {
                state->ramp.active = true;
            }
            if (state->ramp.active) {
                FOC_LinearRamp(state, ref);
            } else {
                state->ramp.output = state->iq_target_raw; // Synchronizacja w stanie ustalonym
            }
            state->iq_setpoint = state->ramp.output;
        } else {
            // Rampa FOC WYŁĄCZONA (Tryb Prędkości / Pozycji)
            state->ramp.active = false;
            state->ramp.output = state->iq_target_raw; // Synchronizacja na wypadek powrotu do trybu momentu
            state->iq_setpoint = state->iq_target_raw; // Prąd z regulatora nadrzędnego idzie bez opóźnień
        }
    #else
        state->iq_setpoint = state->iq_target_raw;
    #endif

    LUT_SinCos(meas->theta_el, &sin_theta, &cos_theta);

    ClarkeTransform(meas->currents.a, meas->currents.b, &state->i_alpha, &state->i_beta);

    ParkTransform(state->i_alpha, state->i_beta, &sin_theta, &cos_theta, &state->id, &state->iq);
    
    state->v_d = pi_control(&state->pi_id, state->id_setpoint - state->id);
    state->v_q = pi_control(&state->pi_iq, state->iq_setpoint - state->iq);
    
    // float vd = state->v_d;
    // float vq = state->v_q;
    // const float VMAX = (VOLTAGE_LIMIT / M_SQRT3);

    // float vmag = sqrtf(vd*vd + vq*vq);
    // if (vmag > VMAX) {
    //     float k = VMAX / vmag;
    //     vd *= k;
    //     vq *= k;
    //     state->v_d = vd;
    //     state->v_q = vq;
    // }

    InvParkTransform(state->v_d, state->v_q, &sin_theta, &cos_theta, &state->v_alpha, &state->v_beta);
    
    SVPWM_Update(state->v_alpha, state->v_beta);
}

static void FOC_GetTelemetry(const void *ctx, Motor_Telemetry_t *telem)
{
    const PI_FOC_State_t *state = (const PI_FOC_State_t*)ctx;
    
    telem->id_meas = state->id;
    telem->iq_meas = state->iq;
    telem->id_ref = state->id_setpoint;
    telem->iq_ref = state->iq_setpoint;
    telem->vd_out = state->v_d;
    telem->vq_out = state->v_q;
    telem->active_algo = 1; // 1 to PI-FOC
}

#ifdef ENABLE_RAMP
static void FOC_LinearRamp(PI_FOC_State_t *state, const Motor_References_t *ref)
{
    if (state->ramp.active)
    {
        if (state->ramp.output < ref->torque_iq_ref)
        {
            state->ramp.output += state->ramp.step;
            if (state->ramp.output > ref->torque_iq_ref) {
                state->ramp.output = ref->torque_iq_ref;
                state->ramp.active = false;
            }
        }
        else if (state->ramp.output > ref->torque_iq_ref)
        {
            state->ramp.output -= state->ramp.step;
            if (state->ramp.output < ref->torque_iq_ref) {
                state->ramp.output = ref->torque_iq_ref;
                state->ramp.active = false;
            }
        }
        else
        {
            state->ramp.active = false;
        }
    }
}
#endif

static void PI_Reset(PI_Controller *pi)
{
    pi->integral = 0.0f;
}

static void Flags_Reset(FocFlags_t *flags)
{
    memset(flags, 0, sizeof(*flags));
}

static void FOC_StatsReset(void)
{
    g_stats.loop_ok  = 0;
    g_stats.loop_err = 0;
    g_stats.currents_err = 0;
}