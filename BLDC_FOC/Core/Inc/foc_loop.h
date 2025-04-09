/*
 * foc_loop.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#ifndef INC_FOC_LOOP_H_
#define INC_FOC_LOOP_H_

#include "stm32g4xx_hal.h"
#include "current_sense.h"

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

void FOC_Init(ADC_HandleTypeDef *hadc);

// Główna pętla FOC
void FOC_Update(const abc_current_t *currents, float theta_el, const dq_ref_t *i_ref);

#endif /* INC_FOC_LOOP_H_ */
