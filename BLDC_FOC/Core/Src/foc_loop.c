/*
 * foc_loop.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#include "foc_loop.h"
#include "transforms.h"
#include "svpwm.h"

// PI kontrolery dla Id i Iq
static PI_Controller pi_id = { .kp = 2.0f, .ki = 200.0f, .limit = 5.0f, .integral = 0.0f };
static PI_Controller pi_iq = { .kp = 2.0f, .ki = 200.0f, .limit = 5.0f, .integral = 0.0f };

static float pi_control(PI_Controller *pi, float error)
{
    pi->integral += error * pi->ki * 0.00005f; // sample time ~50us (20kHz)

    // anty-windup
    if (pi->integral > pi->limit) pi->integral = pi->limit;
    else if (pi->integral < -pi->limit) pi->integral = -pi->limit;

    float output = pi->kp * error + pi->integral;
    if (output > pi->limit) output = pi->limit;
    else if (output < -pi->limit) output = -pi->limit;

    return output;
}

void FOC_Init(void)
{
    pi_id.integral = 0.0f;
    pi_iq.integral = 0.0f;
}

void FOC_Update(float ia, float ib, float theta)
{
    float ialpha, ibeta;
    float id, iq;
    float vd, vq;
    float valpha, vbeta;

    // 1. Clarke
    ClarkeTransform(ia, ib, &ialpha, &ibeta);

    // 2. Park
    ParkTransform(ialpha, ibeta, theta, &id, &iq);

    // 3. PI dla prądu (id_ref = 0, iq_ref = np. 1.0A)
    float id_ref = 0.0f;
    float iq_ref = 1.0f;
    vd = pi_control(&pi_id, id_ref - id);
    vq = pi_control(&pi_iq, iq_ref - iq);

    // 4. Inverse Park
    InvParkTransform(vd, vq, theta, &valpha, &vbeta);

    // 5. SVPWM
    SVPWM_Update(valpha, vbeta);
}

