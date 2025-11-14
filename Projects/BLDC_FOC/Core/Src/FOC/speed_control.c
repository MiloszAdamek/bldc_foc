/*
 * speed_control.c
 *
 *  Created on: Nov 14, 2025
 *      Author: Miloush
 */

#include "FOC/speed_control.h"
#include "FOC/controller_utils.h"
#include "FOC/foc_loop.h"
#include "math.h"

#define VELOCITY_ALPHA      0.95f           // filtr LPF
#define MAX_DTHETA_RAD      0.2f            // ochrona przed glitchami AS5048A
#define RAD_TO_RPM          (60.0f / (2.0f * M_PI))

// Estimator state
static float last_angle = 0.0f;
static float omega_lpf = 0.0f;
static float speed_rpm = 0.0f;

static PI_Controller pi_speed = { .kp = PI_KP_V, .ki = PI_KI_V, .limit = PI_LIMIT_V, .integral = 0.0f, .dt = FSM_PERIOD_SEC};

void SpeedEstimator_Update(float theta_mech, volatile float *out_rpm)
{
    float dtheta = wrap_pi(theta_mech - last_angle);

    if (fabsf(dtheta) > MAX_DTHETA_RAD) {
        *out_rpm = speed_rpm;  // poprzednia wartość
        return;
    }

    float omega_raw = dtheta / PWM_PERIOD_SEC;

    omega_lpf = VELOCITY_ALPHA * omega_lpf +
               (1.0f - VELOCITY_ALPHA) * omega_raw;

    last_angle = theta_mech;

    speed_rpm = omega_lpf * RAD_TO_RPM;

    *out_rpm = speed_rpm;
}

float SpeedController_Update(float speed_ref_rpm)
{
    float speed_now = actual_speed_rpm;
    float error = speed_ref_rpm - speed_now;

    float iq_cmd = pi_control(&pi_speed, error);

    return iq_cmd;
}

void SpeedController_Reset(void)
{
    pi_speed.integral = 0.0f;
}

