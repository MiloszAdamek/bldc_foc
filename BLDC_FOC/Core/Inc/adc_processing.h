/*
 * adc_processing.h
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#ifndef INC_ADC_PROCESSING_H_
#define INC_ADC_PROCESSING_H_

#include <stdint.h>

void adc_process(uint16_t *adc_raw, float *Valpha, float *Vbeta);

#endif /* INC_ADC_PROCESSING_H_ */
