/*
 * delay_us.h
 *
 *  Created on: Apr 21, 2025
 *      Author: Miloush
 */

#ifndef INC_DELAY_US_H_
#define INC_DELAY_US_H_

#include "stm32f4xx_hal.h"

void DWT_Init(void);
void delay_us(uint32_t us);

#endif /* INC_DELAY_US_H_ */
