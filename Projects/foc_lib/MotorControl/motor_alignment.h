/*
 * motor_alignment.h
 *
 *  Created on: Aug 31, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_MOTOR_ALIGNMENT_H_
#define INC_MOTOR_ALIGNMENT_H_

#include "motor_types.h"

extern Motor_Calibration_t g_motor_calib;

bool MotorAlignment_AlignSensor(void);
bool MotorAlignment_IsAligned(void);

float MotorAlignment_GetElectricalAngle(float mech);

void Motor_Motion_Test(void);

#endif /* INC_MOTOR_ALIGNMENT_H_ */