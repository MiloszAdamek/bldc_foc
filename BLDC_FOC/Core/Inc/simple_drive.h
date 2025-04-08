/*
 * simple_drive.h
 *
 *  Created on: Apr 5, 2025
 *      Author: Miloush
 */

#ifndef INC_SIMPLE_DRIVE_H_
#define INC_SIMPLE_DRIVE_H_

#include "stm32g4xx.h"

void SimpleDrive_Init(float freq_hz);
void SimpleDrive_Run(TIM_HandleTypeDef *htim);

#endif /* INC_SIMPLE_DRIVE_H_ */
