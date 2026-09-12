/*
 * speed_control.c
 *
 *  Created on: Nov 14, 2025
 *      Author: Milosz Adamek
 */

#include "speed_control.h"
#include "foc_utils.h"
#include "foc_loop.h"
#include "speed_estimator.h"
#include "math.h"

#define RAD_TO_RPM          (60.0f / (2.0f * M_PI))

volatile float speed_ref_rpm = 0.0f;
static SpeedMode_t speed_mode = SPEED_MODE_DIRECT;

// RAMPA
volatile bool speed_ramp_active = false;
static const float speed_step = 1.0f; // przyrost prędkości na 1 krok przy aktywacji rampy liniowej
volatile float speed_ramp_out = 0.0f;

static PI_Controller pi_speed = { .kp = PI_KP_V, .ki = PI_KI_V, .limit = PI_LIMIT_V, .integral = 0.0f, .dt = SPEED_PERIOD_SEC};

void SpeedController_Update(Motor_References_t *ref)
{
    if (speed_mode == SPEED_MODE_RAMP) {
        SpeedController_LinearRamp();
    }

    float target = (speed_mode == SPEED_MODE_RAMP)
                   ? speed_ramp_out
                   : speed_ref_rpm;

    float estimated_speed_rpm = SpeedEstimator_GetOmegaRPM();
    float error = target - estimated_speed_rpm;

    float iq_ref = pi_control(&pi_speed, error);

	// Przepisanie wartości zadanej prądu Iq do struktury referencji dla algorytmu sterowania
	ref->torque_iq_ref = iq_ref;
}

void SpeedController_SetTarget_Ramp(float new_target_rpm)
{
	speed_mode = SPEED_MODE_RAMP;
	speed_ramp_out = SpeedEstimator_GetOmegaRPM(); //start rampy od aktualnej prędkości
    speed_ref_rpm = new_target_rpm;
    speed_ramp_active = true;
}

void SpeedController_SetTarget(float new_target_rpm)
{
    speed_mode = SPEED_MODE_DIRECT;
    speed_ref_rpm = new_target_rpm;
    speed_ramp_active = false;
    speed_ramp_out = new_target_rpm;
}

// Ref z regulatora pozycji
void SpeedController_SetReference(float rpm)
{
    speed_ref_rpm = rpm;
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

