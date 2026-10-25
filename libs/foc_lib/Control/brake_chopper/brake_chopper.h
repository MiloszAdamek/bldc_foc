/*
 * brake_chopper.h
 *
 *  Created on: Oct 11, 2026
 *      Author: Milosz Adamek
 */

#ifndef BRAKE_CHOPPER_H_
#define BRAKE_CHOPPER_H_

#include "motor_types.h"

typedef struct
{
    bool  running;
    bool  latched;
    float duty;
    float integ;
    float power_w;
    float temp_c;
} BrakeChopperState_t;

void BrakeChopper_Init(void);
void BrakeChopper_Update(Motor_Measurements_t *meas, float dt);
void BrakeChopper_Reset(void);

#endif /* BRAKE_CHOPPER_H_ */
