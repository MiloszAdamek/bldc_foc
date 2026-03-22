/*
 * position_control.c
 *
 *  Created on: Mar 22, 2026
 *      Author: Miloush
 */
#include "FOC/controller_utils.h"
#include "FOC/speed_control.h"
#include "BSP/as5048a.h"
#include "math.h"

#define RAD_TO_DEG (360.0f / (2.0f * M_PI))

static PI_Controller pi_position = { .kp = PI_KP_P, .ki = PI_KI_P, .limit = PI_LIMIT_P, .integral = 0.0f, .dt = POSITION_PERIOD_SEC};
volatile float position_ref_deg = 0.0f;

float _get_angle_deg(){ return RAD_TO_DEG * AS5048_GetMechanicalAngle();};
float _get_angle_rad(){ return AS5048_GetMechanicalAngle();}

void PositionController_Init();

void PositionController_Update()
{
    float error = position_ref_deg - _get_angle_rad();
    float velocity_ref = pi_control(&pi_position, error);
    SpeedController_SetTarget_Ramp(velocity_ref);
}

void PositionController_SetTarget(float new_target_position){
	position_ref_deg = new_target_position;
}

void PositionController_Reset(void)
{
    pi_position.integral = 0.0f;
}


