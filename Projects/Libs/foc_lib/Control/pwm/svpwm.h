/*
 * svpwm.h
 *
 *  Created on: Mar 30, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_SVPWM_H_
#define INC_SVPWM_H_

#include "tim.h"

void SVPWM_GetDutyCycles(float Ualpha, float Ubeta, float *dc_a, float *dc_b, float *dc_c);

void SVPWM_Update(float Ualpha, float Ubeta);

void SVPWM_Init(TIM_HandleTypeDef *htim);

//void SVPWM_Test_Run(float freq);

#endif /* INC_SVPWM_H_ */
