/*
 * speed_control.h
 *
 *  Created on: Nov 14, 2025
 *      Author: Miloush
 */

#ifndef INC_FOC_SPEED_CONTROL_H_
#define INC_FOC_SPEED_CONTROL_H_

#include <stdint.h>

extern volatile float actual_speed_rpm;

void SpeedControl_Init(void);

// Aktualizacja estymatora prędkości, pętla 20 kHz
void SpeedEstimator_Update(float theta_mech, volatile float *out_rpm);

// Aktualizacja regulatora prędkości (wywoływane w 1 kHz)
float SpeedController_Update(float speed_ref_rpm);

void SpeedController_Reset();

#endif /* INC_FOC_SPEED_CONTROL_H_ */
