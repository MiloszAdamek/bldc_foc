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
#include "APP/motor_control.h"
#include "math.h"

#define RAD_TO_DEG (360.0f / (2.0f * M_PI))

static PI_Controller pi_position = { .kp = PI_KP_P, .ki = PI_KI_P, .limit = PI_LIMIT_P, .integral = 0.0f, .dt = POSITION_PERIOD_SEC};
static PositionUnit_t position_unit = POSITION_UNIT_RAD;
volatile float position_ref = 0.0f;

static float PositionController_GetPosition(void)
{
    float angle_rad = AS5048_GetMechanicalAngle();

    if (position_unit == POSITION_UNIT_DEG)
        return angle_rad * RAD_TO_DEG;
    else
        return angle_rad;
}

void PositionController_Init(PositionUnit_t unit)
{
    position_unit = unit;
    pi_position.integral = 0.0f;
}

void PositionController_Update()
{
    float error = position_ref - PositionController_GetPosition();
    float velocity_ref = pi_control(&pi_position, error);
    SpeedController_SetTarget_Ramp(velocity_ref);
}

void PositionController_SetTarget(float new_target_position){
	position_ref = new_target_position;
}

void PositionController_Reset(void)
{
    pi_position.integral = 0.0f;
}


