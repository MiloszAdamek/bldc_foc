/*
 * speed_control.h
 *
 *  Created on: Nov 14, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_FOC_SPEED_CONTROL_H_
#define INC_FOC_SPEED_CONTROL_H_

#include <stdint.h>

extern volatile float estimated_speed_rpm;
extern volatile float speed_ramp_out;

typedef enum {
    SPEED_MODE_DIRECT = 0,
    SPEED_MODE_RAMP
} SpeedMode_t;


void SpeedControl_Init(void);

// Aktualizacja estymatora prędkości, pętla 10 kHz
void SpeedEstimator_Update(float theta_mech);

void SpeedEstimator_Update_1khz();

// Aktualizacja regulatora prędkości (wywoływane w pętli 1 kHz)
void SpeedController_Update();

void SpeedController_LinearRamp();

void SpeedController_SetReference(float rpm); // Dla regulatora pozycji

void SpeedController_SetTarget(float new_target_rpm); // Bez rampy liniowej

void SpeedController_SetTarget_Ramp(float new_target_rpm);

void SpeedController_Reset();

#endif /* INC_FOC_SPEED_CONTROL_H_ */
