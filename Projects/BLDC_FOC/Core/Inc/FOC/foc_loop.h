/*
 * foc_loop.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#ifndef INC_FOC_LOOP_H_
#define INC_FOC_LOOP_H_

#include "stm32g4xx_hal.h"
#include <stdio.h>
#include "math.h"
#include "FOC/controller_utils.h"
#include "FOC/svpwm.h"
#include "BSP/current_sense.h"
#include "BSP/as5048a.h"

#define _3PI_2 4.71238898038f

extern TIM_HandleTypeDef* foc_htim;
extern TIM_HandleTypeDef* foc_htim;
extern TIM_HandleTypeDef* enc_htim;
extern ADC_HandleTypeDef* foc_hadc;

typedef struct {
    float d;
    float q;
} dq_ref_t;

// Flags
extern volatile bool ramp_active;
extern volatile bool spi_angle_ready;
extern volatile bool foc_data_ready;
extern volatile bool sensor_aligned;
extern volatile bool new_current_data_ready;
extern volatile bool encoder_prev_ready;

extern volatile float theta_el_last;
extern volatile float theta_mech_last;

// Debug
extern volatile float debug_id;
extern volatile float debug_iq;
extern volatile float debug_id_ref;
extern volatile float debug_iq_ref;
extern volatile float debug_vd;
extern volatile float debug_vq;

void FOC_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim_foc, TIM_HandleTypeDef *htim_enc, SPI_HandleTypeDef *hspi);

void FOC_RunLoop(void); // Główna pętla FOC

void FOC_Update(float theta_el);

void FOC_SetIqTarget(float new_target);

float FOC_GetElecticalAngle(float mech);

void FOC_SetIqTarget_Ramp(float new_target);

void FOC_SetTorqueTarget(float torque_mNm);

bool FOC_AlignSensor(); // Kalibracja enkodera

void FOC_Stop(); // Zatrzymanie PWM, wyłączenie driverów, zatrzymanie ADC

void FOC_Start();

#endif /* INC_FOC_LOOP_H_ */
