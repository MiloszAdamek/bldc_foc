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

#define VELOCITY_ALPHA      0.99f           // filtr LPF
#define MAX_DTHETA_RAD      0.2f            // ochrona przed glitchami AS5048A
#define RAD_TO_RPM          (60.0f / (2.0f * M_PI))

static float last_angle = 0.0f;
static float omega_lpf = 0.0f;
volatile float estimated_speed_rpm = 0.0f;
volatile float speed_ref_rpm = 0.0f;

// RAMPA
volatile bool speed_ramp_active = false;
static const float speed_step = 1.0f; // przyrost prędkości na 1 krok przy aktywacji rampy liniowej
volatile float speed_ramp_out = 0.0f;

static PI_Controller pi_speed = { .kp = PI_KP_V, .ki = PI_KI_V, .limit = PI_LIMIT_V, .integral = 0.0f, .dt = SPEED_PERIOD_SEC};

void SpeedEstimator_Update(float theta_mech)
{
    float dtheta = wrap_pi(theta_mech - last_angle);

    if (fabsf(dtheta) > MAX_DTHETA_RAD) {
        return;
    }

    float omega_raw = dtheta / FOC_PERIOD_SEC; // Estymator działa w pętli FOC 10 kHz

    omega_lpf = VELOCITY_ALPHA * omega_lpf +
               (1.0f - VELOCITY_ALPHA) * omega_raw;

    last_angle = theta_mech;

    estimated_speed_rpm = omega_lpf * RAD_TO_RPM;
}

float SpeedController_GetReference(void)
{
    return speed_ramp_out;
}

float SpeedController_Update()
{
	SpeedController_LinearRamp();

    float target = SpeedController_GetReference();
    float error = target - estimated_speed_rpm;

    return pi_control(&pi_speed, error);
}

void SpeedController_SetTarget_Ramp(float new_target_rpm)
{
	speed_ramp_out = estimated_speed_rpm; //start rampy od aktualnej prędkości
    speed_ref_rpm = new_target_rpm;
    speed_ramp_active = true;
}

void SpeedController_LinearRamp()
{
    if (!speed_ramp_active){
    	 return;
    }

	if (speed_ramp_out < speed_ref_rpm)
	{
		speed_ramp_out += speed_step;
		if (speed_ramp_out > speed_ref_rpm) {
			speed_ramp_out = speed_ref_rpm;
			speed_ramp_active = false; // Zakończ rampę
		}
	}
	else if (speed_ramp_out > speed_ref_rpm)
	{
		speed_ramp_out -= speed_step;
		if (speed_ramp_out < speed_ref_rpm) {
			speed_ramp_out = speed_ref_rpm;
			speed_ramp_active = false; // Zakończ rampę
		}
	}
	else
	{
		speed_ramp_active = false;
	}
}

void SpeedController_Reset(void)
{
    pi_speed.integral = 0.0f;
}

