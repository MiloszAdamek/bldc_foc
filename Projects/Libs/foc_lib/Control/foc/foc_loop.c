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

// RAMPA
#define IQ_RAMP_RATE_A_S 0.1f // A/s

// FIELD WEAKENING
#define FIELD_WEAKENING_THRESHOLD 0.95f // 95% napięcia szyny DC (Vbus / sqrt(3))

// FLAGI
volatile bool currents_ready = false;
extern volatile bool spi_ready;

#ifdef ENABLE_RAMP
    static void FOC_LinearRamp(PI_FOC_State_t *state, const Motor_References_t *ref);
#endif

static void PI_Reset(PI_Controller *pi);
static void Flags_Reset(FocFlags_t *flags);
static void FOC_StatsReset(void);
static void FOC_GetTelemetry(const void *ctx, volatile Motor_Telemetry_t *telem);
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

    state->pi_field_weakening = (PI_Controller){
        .kp = PI_KP_FW,
        .ki = PI_KI_FW,
        .limit = PI_LIMIT_FW,
        .integral = 0.0f,
        .dt = FOC_PERIOD_SEC
    };

    state->id_setpoint = 0.0f;
    state->iq_setpoint = 0.0f;

    #ifdef ENABLE_RAMP
        state->ramp.step = IQ_RAMP_RATE_A_S * FOC_PERIOD_SEC;
        state->ramp.output = 0.0f;
    #endif
}

void FOC_Start(void *ctx){
    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;

    PI_Reset(&state->pi_id);
    PI_Reset(&state->pi_iq);
    PI_Reset(&state->pi_field_weakening);
    state->id_setpoint = 0.0f;
    state->iq_setpoint = 0.0f;
    state->iq_target_raw = 0.0f;

    #ifdef ENABLE_RAMP
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
        state->ramp.output = 0.0f;
    #endif  

    state->id_setpoint = 0.0f;
    state->iq_setpoint = 0.0f;
    state->iq_target_raw = 0.0f;

    PI_Reset(&state->pi_id);
    PI_Reset(&state->pi_iq);
    PI_Reset(&state->pi_field_weakening);
    FOC_StatsReset();
}

static void FOC_Update(void *ctx, const Motor_Measurements_t *meas, const Motor_References_t *ref, Motor_Output_t *out)
{
    PI_FOC_State_t *state = (PI_FOC_State_t*)ctx;
    float sin_theta, cos_theta;

    state->iq_target_raw = ref->torque_iq_ref;

    #ifdef ENABLE_RAMP
        FOC_LinearRamp(state, ref);
        state->iq_setpoint = state->ramp.output;
    #else
        state->iq_setpoint = state->iq_target_raw;
    #endif

    // FW od prędkości
    // #ifdef ENABLE_FIELD_WEAKENING

    //     const float BASE_SPEED_RPM = 1500.0f;
    //     const float FW_MAX_RPM     = 2500.0f;
    //     const float FW_ID_MAX      = 1.0f;

    //     float rpm = fabsf(meas->omega_mech_rpm);

    //     if (rpm <= BASE_SPEED_RPM)
    //     {
    //         state->id_setpoint = 0.0f;
    //     }
    //     else if (rpm >= FW_MAX_RPM)
    //     {
    //         state->id_setpoint = -FW_ID_MAX;
    //     }
    //     else
    //     {
    //         float fw = (rpm - BASE_SPEED_RPM) / (FW_MAX_RPM - BASE_SPEED_RPM);

    //         state->id_setpoint = -FW_ID_MAX * fw;
    //     }

    // #else

    //     state->id_setpoint = 0.0f;

    // #endif

    // FW od napięcia
    float v_max = meas->v_bus / M_SQRT3;
    float v_max_sq = v_max * v_max;

    #ifdef ENABLE_FIELD_WEAKENING
            
        const float V_HIGH = 0.97f;
        const float V_LOW  = 0.93f;
        const float ID_STEP = 0.001f;
        const float ID_MAX = 1.0f;
        
        float v_mag_sq = state->v_d * state->v_d + state->v_q * state->v_q;

        if (v_mag_sq > (V_HIGH * V_HIGH * v_max_sq))
        {
            state->id_setpoint -= ID_STEP;

            if (state->id_setpoint < -ID_MAX)
                state->id_setpoint = -ID_MAX;
        }
        else if (v_mag_sq < (V_LOW * V_LOW * v_max_sq))
        {
            state->id_setpoint += ID_STEP;

            if (state->id_setpoint > 0.0f)
                state->id_setpoint = 0.0f;
        }
    #else

        state->id_setpoint = 0.0f;
        
    #endif

    #ifdef ENABLE_CURRENT_LIMIT

        if (state->id_setpoint > 0.0f)  state->id_setpoint = 0.0f;
        if (state->id_setpoint < -CURRENT_LIMIT) state->id_setpoint = -CURRENT_LIMIT;

        if (state->iq_setpoint > CURRENT_LIMIT) state->iq_setpoint = CURRENT_LIMIT;
        if (state->iq_setpoint < -CURRENT_LIMIT) state->iq_setpoint = -CURRENT_LIMIT;

        float i_max_sq = CURRENT_LIMIT * CURRENT_LIMIT;
        float id_sq = state->id_setpoint * state->id_setpoint;
        float iq_limit_sq = i_max_sq - id_sq;
        float iq_max_limit = (iq_limit_sq > 0.0f) ? sqrtf(iq_limit_sq) : 0.0f;

        if (state->iq_setpoint > iq_max_limit)  state->iq_setpoint = iq_max_limit;
        if (state->iq_setpoint < -iq_max_limit) state->iq_setpoint = -iq_max_limit;

    #endif

    LUT_SinCos(meas->theta_el, &sin_theta, &cos_theta);

    ClarkeTransform(meas->currents.a, meas->currents.b, &state->i_alpha, &state->i_beta);

    ParkTransform(state->i_alpha, state->i_beta, &sin_theta, &cos_theta, &state->id, &state->iq);
    
    // === Wariant z priorytetem osi D (ograniczenie na osi Q w zależności od napięcia na osi D)===
    // float v_max = meas->v_bus / M_SQRT3;
    // float v_max_sq = v_max * v_max;

    // state->pi_id.limit = v_max;
    // state->v_d = pi_control(&state->pi_id, state->id_setpoint - state->id);

    // // Dostępne napięcie dla osi Q po uwzględnieniu ograniczenia na osi D
    // float vd_sq = state->v_d * state->v_d;
    // float vq_limit_sq = v_max_sq - vd_sq;

    // float max_vq = (vq_limit_sq > 0.0f) ? sqrtf(vq_limit_sq) : 0.0f;

    // state->pi_iq.limit = max_vq; // Dynamiczna zmiana limitu dla regulatora PI na osi Q
    // state->v_q = pi_control(&state->pi_iq, state->iq_setpoint - state->iq);

    // Wariant bez priorytetu osi D - dla wersji z FW
    // 1. Obliczenie nieskorelowanych napięć z regulatorów PI
    state->pi_id.limit = v_max;
    state->v_d = pi_control(&state->pi_id, state->id_setpoint - state->id);

    state->pi_iq.limit = v_max;
    state->v_q = pi_control(&state->pi_iq, state->iq_setpoint - state->iq);

    // 2. Skalowanie wektorowe (brak uprzywilejowania którejkolwiek osi)
    float v_mag_sq = state->v_d * state->v_d + state->v_q * state->v_q;

    if (v_mag_sq > v_max_sq)
    {
        float v_mag = sqrtf(v_mag_sq);
        float scale = v_max / v_mag;

        state->v_d *= scale;
        state->v_q *= scale;

        // WAŻNE: Anti-windup dla regulatorów PI!
        // Jeśli Twoja funkcja pi_control ma wbudowane anti-windup,
        // należy zaktualizować stan całkujący o faktycznie podane napięcie,
        // w przeciwnym razie regulatory będą się nasycać.
    }

    InvParkTransform(state->v_d, state->v_q, &sin_theta, &cos_theta, &state->v_alpha, &state->v_beta);
    
    SVPWM_Update(state->v_alpha, state->v_beta);
}

static void FOC_GetTelemetry(const void *ctx, volatile Motor_Telemetry_t *telem)
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
    // Jeśli rampa jest wyłączona to ustawiamy wyjście rampy bezpośrednio na wartość referencyjną
    if (!ref->iq_ramp_enabled)
    {
        state->ramp.output = ref->torque_iq_ref;
        return;
    }

    float diff = ref->torque_iq_ref - state->ramp.output;

    if (diff > state->ramp.step)
    {
        state->ramp.output += state->ramp.step;
    }
    else if (diff < -state->ramp.step)
    {
        state->ramp.output -= state->ramp.step;
    }
    else
    {
        state->ramp.output = ref->torque_iq_ref;
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