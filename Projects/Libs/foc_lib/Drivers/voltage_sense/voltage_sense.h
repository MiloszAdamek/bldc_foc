/*
 * voltage_sense.h
 *
 *  Created on: Aug 19, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_VOLTAGE_SENSE_H_
#define INC_VOLTAGE_SENSE_H_

#include "adc.h"

void VoltageSense_Init(ADC_HandleTypeDef *hadc);

void VoltageSense_UpdateADCCoefficient(float vdd_voltage);

void VoltageSense_ReadVbus(void);

float VoltageSense_GetVbus_ISR(void);

#endif /* INC_VOLTAGE_SENSE_H_ */