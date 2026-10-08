/*
 * ntc_sense.h
 *
 *  Created on: Oct 8, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_NTC_SENSE_H_
#define INC_NTC_SENSE_H_

typedef struct {
    float R25;       // Resistance at 25°C
    float B;         // Beta value
    float R_series;  // Series resistor value
    float V_supply;  // Supply voltage
    float V_ref;     // ADC reference voltage
    float ADC_resolution; // ADC resolution
} TemperatureSensor_t;

float NTC_CalculateTemperature_VCC(TemperatureSensor_t *sensor, float adc_value);

float NTC_CalculateTemperature_GND(TemperatureSensor_t *sensor, float adc_value);

#endif /* INC_NTC_SENSE_H_ */