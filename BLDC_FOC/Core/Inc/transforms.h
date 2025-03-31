/*
 * transforms.h
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#ifndef INC_TRANSFORMS_H_
#define INC_TRANSFORMS_H_

#include "stm32g4xx_hal.h"

void clarke_transform(float Ia, float Ib, float *Valpha, float *Vbeta);
void park_transform(float Valpha, float Vbeta, float theta, float *Vd, float *Vq);

#endif /* INC_TRANSFORMS_H_ */
