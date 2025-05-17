/*
 * svpwm.h
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#ifndef INC_SVPWM_H_
#define INC_SVPWM_H_

#include "main.h"

extern volatile uint16_t debug_Ta;
extern volatile uint16_t debug_Tb;
extern volatile uint16_t debug_Tc;

void SVPWM_Update(float Valpha, float Vbeta);

#endif /* INC_SVPWM_H_ */
