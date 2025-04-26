/*
 * transforms.h
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#ifndef INC_TRANSFORMS_H_
#define INC_TRANSFORMS_H_

#include "stm32g4xx_hal.h"
#include <stdint.h>


// Clarke transform: 3 fazy → αβ (z pomiarów Ia, Ib)
void ClarkeTransform(float ia, float ib, float *ialpha, float *ibeta);

// Park transform: αβ → dq (theta w radianach)
void ParkTransform(float ialpha, float ibeta, float theta, float *id, float *iq);

// Inverse Park transform: dq → αβ (theta w radianach)
void InvParkTransform(float vd, float vq, float theta, float *valpha, float *vbeta);


#endif /* INC_TRANSFORMS_H_ */
