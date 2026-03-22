/*
 * position_control.h
 *
 *  Created on: Mar 22, 2026
 *      Author: Miloush
 */

#ifndef INC_FOC_POSITION_CONTROL_H_
#define INC_FOC_POSITION_CONTROL_H_

#include <stdint.h>

typedef enum
{
    POSITION_UNIT_RAD = 0,
    POSITION_UNIT_DEG
} PositionUnit_t;


void PositionController_Init(PositionUnit_t unit);

// Aktualizacja regulatora pozycji (wywoływane w pętli 1 kHz)
void PositionController_Update();

void PositionController_SetTarget(float new_target_position);

void PositionController_Reset();

#endif /* INC_FOC_POSITION_CONTROL_H_ */
