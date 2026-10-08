/*
 * ntc_sense.c
 *
 *  Created on: Oct 8, 2026
 *      Author: Milosz Adamek
 */

#include "ntc_sense.h"
#include "math_consts.h"

#define KELVIN_OFFSET 273.15f

float NTC_CalculateTemperature_VCC(TemperatureSensor_t *sensor, float adc_value) 
{
    // Zabezpieczenie przed zwarciem
    if (adc_value < 1.0f) {
        return 125.0f; // max temperatura
    }
    // Zabezpieczenie przed rozwarciem
    if (adc_value >= sensor->ADC_resolution) {
        return -40.0f;
    }

    float r_ntc = sensor->R_series * ((sensor->ADC_resolution - adc_value) / adc_value);
    float temp_kelvin = 1.0f / ((1.0f / (25.0f + KELVIN_OFFSET)) + (1.0f / sensor->B) * logf(r_ntc / sensor->R25));
    float temp_celsius = temp_kelvin - KELVIN_OFFSET;
    return temp_celsius;
}

float NTC_CalculateTemperature_GND(TemperatureSensor_t *sensor, float adc_value)
{
    // Zabezpieczenie przed zwarciem
    if (adc_value < 1.0f) {
        return 125.0f; // max temperatura
    }
    // Zabezpieczenie przed rozwarciem
    if (adc_value >= sensor->ADC_resolution) {
        return -40.0f;
    }

    float r_ntc = sensor->R_series * (adc_value / (sensor->ADC_resolution - adc_value));
    float temp_kelvin = 1.0f / ((1.0f / (25.0f + KELVIN_OFFSET)) + (1.0f / sensor->B) * logf(r_ntc / sensor->R25));
    float temp_celsius = temp_kelvin - KELVIN_OFFSET;
    return temp_celsius;
}