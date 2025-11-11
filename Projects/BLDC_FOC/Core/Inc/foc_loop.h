/*
 * foc_loop.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#ifndef INC_FOC_LOOP_H_
#define INC_FOC_LOOP_H_

#include <controller_utils.h>
#include "stm32g4xx_hal.h"
#include "svpwm.h"
#include "current_sense.h"
#include "as5048a.h"
#include <stdio.h>
#include "math.h"

#define _PI_2 1.57079632679f
#define _3PI_2 4.71238898038f
#define _SQRT3_2 0.86602540378f

typedef struct {
    float d;
    float q;
} dq_ref_t;

// Flags
extern volatile bool ramp_active;
extern volatile bool spi_angle_ready;
extern volatile bool foc_data_ready;

extern volatile float theta_el_last;
extern float FOC_GetElecticalAngle(float mech);

// Debug
extern volatile float debug_id;
extern volatile float debug_iq;
extern volatile float debug_id_ref;
extern volatile float debug_iq_ref;
extern volatile float debug_vd;
extern volatile float debug_vq;

void FOC_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim, SPI_HandleTypeDef *hspi);

// Główna pętla FOC
void FOC_Update(float theta_el);

void FOC_SetIqTarget(float new_target);

void FOC_SetTorqueTarget(float torque_mNm);

// Kalibracja enkodera
void FOC_AlignSensor();

void FOC_SetPhaseVoltage(float Uq, float Ud, float angle_el);

#endif /* INC_FOC_LOOP_H_ */
