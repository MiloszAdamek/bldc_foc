/*
 * adc_processing.c
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#include "adc_processing.h"

#define ADC_CENTER 2048.0f

void adc_process(uint16_t *adc_raw, float *Valpha, float *Vbeta) {
    float ia = (float)(adc_raw[0]) - ADC_CENTER;
    float ib = (float)(adc_raw[1]) - ADC_CENTER;

    *Valpha = ia;
    *Vbeta  = (ia + 2.0f * ib) / 1.73205080757f;  // Clarke transform
}

