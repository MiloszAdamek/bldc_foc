/*
 * voltage_sense.h
 *
 *  Created on: Aug 19, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_VOLTAGE_SENSE_H_
#define INC_VOLTAGE_SENSE_H_

#include "adc.h"
#include "board.h"

void VoltageSense_Init(ADC_InjectedChannel_t *hadc_inj, float vdd_voltage);

void VoltageSense_UpdateADCCoefficient(float vdd_voltage);

void VoltageSense_ReadVbus(void);

void VoltageSense_Read(float *vbus);

void VoltageSense_Process_ISR(void);

#endif /* INC_VOLTAGE_SENSE_H_ */