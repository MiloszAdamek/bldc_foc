/*
 * foc_loop.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#ifndef INC_FOC_LOOP_H_
#define INC_FOC_LOOP_H_

#include "stm32g4xx_hal.h"

// Ustawienia regulatorów PI
typedef struct {
    float kp;
    float ki;
    float integral;
    float limit;
} PI_Controller;

void FOC_Init(void);

// Główna pętla FOC
void FOC_Update(float ia, float ib, float theta_el_rad);

#endif /* INC_FOC_LOOP_H_ */
