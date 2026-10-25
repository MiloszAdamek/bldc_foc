/*
 * slow_adc.h
 *
 *  Created on: Sep 28, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_SLOW_ADC_H_
#define INC_SLOW_ADC_H_

#include "adc.h"

typedef struct {
    float v_bus;
    float motor_temp;
    float mosfet_temp;
} SlowLoopMeasurements_t;

typedef struct {
    float R25;       // Resistance at 25°C
    float B;         // Beta value
    float R_series;  // Series resistor value
    float V_supply;  // Supply voltage
    float V_ref;     // ADC reference voltage
    float ADC_resolution; // ADC resolution
} TemperatureSensor_t;

void SlowADC_Init(ADC_HandleTypeDef *hadc, float vdd_voltage);
void SlowADC_Trigger(void);
void SlowADC_UpdateADCCoefficient();
void SlowADC_CalculateMeasurements(void);

float SlowADC_GetVBusVoltage_ISR(void);
float SlowADC_GetMotorTemperature_ISR(void);
float SlowADC_GetMosfetTemperature_ISR(void);
SlowLoopMeasurements_t SlowADC_GetData_ISR(void);

#endif /* INC_SLOW_ADC_H_ */