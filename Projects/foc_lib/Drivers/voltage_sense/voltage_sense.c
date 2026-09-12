/*
 * voltage_sense.c
 *
 *  Created on: Aug 19, 2026
 *      Author: Milosz Adamek
 */

#include "voltage_sense.h"
#include "board.h"

static ADC_HandleTypeDef *s_hadc;

static float s_adc_to_voltage;
 
void VoltageSense_Init(ADC_HandleTypeDef *hadc) {
    s_hadc = hadc;
}

void VoltageSense_UpdateADCCoefficient(float vdd_voltage) {
    s_adc_to_voltage = VOLTAGE_SENSE_DIV_RATIO * vdd_voltage / (float)ADC_RESOLUTION;
}

void VoltageSense_ReadVDC(volatile float *voltage) {
    uint32_t adc_value = 0;

    HAL_ADC_Start(s_hadc);
    if (HAL_ADC_PollForConversion(s_hadc, 10) == HAL_OK) {
        adc_value = HAL_ADC_GetValue(s_hadc);
    }
    HAL_ADC_Stop(s_hadc);

    *voltage = (float)adc_value * s_adc_to_voltage;
}