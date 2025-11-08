/*
 * foc_loop.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#ifndef INC_FOC_LOOP_H_
#define INC_FOC_LOOP_H_

#include "stm32g4xx_hal.h"
#include "transforms.h"
#include "svpwm.h"
#include "current_sense.h"
#include "as5048a.h"
#include <stdio.h>
#include "math.h"

#define _PI_2 1.57079632679f
#define _3PI_2 4.71238898038f
#define _SQRT3_2 0.86602540378f

// Ustawienia regulatorów PI
typedef struct {
    float kp;
    float ki;
    float integral;
    float limit;
} PI_Controller;

typedef struct {
    float d;
    float q;
} dq_ref_t;

extern volatile float theta_el;
extern volatile AS5048_ReadResult raw;
extern volatile float encoder_offset;
extern volatile int encoder_direction;

// Debug global variables
extern volatile float debug_angle_deg;
extern volatile float debug_theta_el;
extern volatile float debug_ia;
extern volatile float debug_ib;
extern volatile float debug_ic;
extern volatile uint16_t raw_copy;

// Flags
extern volatile bool currents_ready;
extern volatile bool encoder_ready;
extern volatile bool encoder_trigger;
extern volatile bool encoder_calibrated;
extern volatile bool foc_update_ready;
extern volatile bool ramp_active;
extern volatile bool svpwm_test_active;

void FOC_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim, SPI_HandleTypeDef *hspi);

// Główna pętla FOC
void FOC_Update();

void FOC_SetIqTarget(float new_target);

// Kalibracja enkodera
void FOC_AlignSensor();

void FOC_SetPhaseVoltage(float Uq, float Ud, float angle_el);

#endif /* INC_FOC_LOOP_H_ */
