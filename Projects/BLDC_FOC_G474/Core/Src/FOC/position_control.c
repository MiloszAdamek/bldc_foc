/*
 * position_control.c
 *
 *  Created on: Mar 22, 2026
 *      Author: Miloush
 */
#include "FOC/position_control.h"
#include "FOC/speed_control.h"
#include "FOC/controller_utils.h"
#include "BSP/as5048a.h"
#include "math.h"
#include <stdio.h>

#define RAD_TO_DEG (360.0f / (2.0f * M_PI))
#define RAD_PER_SEC_TO_RPM (60.0f / (2.0f * M_PI))
#define POS_DEADBAND 0.005f
#define POS_ERROR_LPF_ALPHA 0.85f
#define POS_INTEGRAL_DECAY  0.999f

static PI_Controller pi_position = { .kp = PI_KP_P, .ki = PI_KI_P, .limit = PI_LIMIT_P, .integral = 0.0f, .dt = POSITION_PERIOD_SEC};
static PositionUnit_t position_unit = POSITION_UNIT_RAD;
volatile float position_ref = 0.0f;
static float error_lpf = 0.0f;

//debug
volatile float position_err;
volatile float position_reg_out;
volatile float position_current_pos;

static float PositionController_GetPosition(void)
{
    float angle_rad = AS5048_GetMechanicalAngle();

    if (position_unit == POSITION_UNIT_RAD)
        return angle_rad;
    else
        return angle_rad * RAD_TO_DEG;
}

void PositionController_Init(PositionUnit_t unit)
{
    position_unit = unit;
    pi_position.integral = 0.0f;
}

void PositionController_Update()
{
	float current = PositionController_GetPosition(); // rad [0, 2pi]
	float raw_error = wrap_pi(position_ref - current); // rad [-pi, pi]

	error_lpf = POS_ERROR_LPF_ALPHA * error_lpf + (1.0f - POS_ERROR_LPF_ALPHA) * raw_error;

	float error = error_lpf;

	if(fabs(error) < POS_DEADBAND){
        error *= (fabsf(error) / POS_DEADBAND);  // skaluj liniowo do 0
        pi_position.integral *= POS_INTEGRAL_DECAY;
	}

    float velocity_ref = pi_control(&pi_position, error); // rad/s
    velocity_ref *= RAD_PER_SEC_TO_RPM; // RPM

	position_err = error;
	position_reg_out = velocity_ref;

	SpeedController_SetReference(velocity_ref); //RPM
}

void PositionController_SetTarget(float new_target_position){
	position_ref = new_target_position;
}

void PositionController_Reset(void)
{
    pi_position.integral = 0.0f;
}


