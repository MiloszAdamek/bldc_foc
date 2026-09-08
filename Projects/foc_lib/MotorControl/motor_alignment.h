/*
 * motor_alignment.h
 *
 *  Created on: Aug 31, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_MOTOR_ALIGNMENT_H_
#define INC_MOTOR_ALIGNMENT_H_

#include "motor_types.h"

extern MotorCalibration_t g_motor_calib;

bool MotorAlignment_AlignSensor(void);
bool MotorAlignment_IsAligned(void);

float MotorAlignment_GetElectricalAngle(float mech);

#endif /* INC_MOTOR_ALIGNMENT_H_ */