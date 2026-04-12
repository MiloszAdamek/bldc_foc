/*
 * svpwm.h
 *
 *  Created on: Mar 30, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_SVPWM_H_
#define INC_SVPWM_H_

#include "main.h"
#include "stm32g4xx.h"

void SVPWM_Update(float Valpha, float Vbeta);

void SVPWM_Init();

void SVPWM_Test_Run(float freq);

#endif /* INC_SVPWM_H_ */
