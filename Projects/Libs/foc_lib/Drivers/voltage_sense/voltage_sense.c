/*
 * voltage_sense.c
 *
 *  Created on: Aug 19, 2026
 *      Author: Milosz Adamek
 */

#include "voltage_sense.h"
#include "board.h"

// Obsługa pomiaru injected

static ADC_InjectedChannel_t *s_hadc;

static float s_adc_to_voltage;
static float adc_raw_voltage = 0;

static volatile float v_bus_meas = 0;
 
void VoltageSense_Init(ADC_InjectedChannel_t *hadc_inj, float vdd_voltage) 
{
    s_hadc = hadc_inj;
    VoltageSense_UpdateADCCoefficient(vdd_voltage);
}

void VoltageSense_UpdateADCCoefficient(float vdd_voltage) 
{
    s_adc_to_voltage = VOLTAGE_SENSE_DIV_RATIO * vdd_voltage / (float)ADC_RESOLUTION;
}

void VoltageSense_CalculateVoltage() 
{
    v_bus_meas = (float)adc_raw_voltage * s_adc_to_voltage;
}

void VoltageSense_Read(float *vbus)
{
    *vbus = v_bus_meas;
}

void VoltageSense_Process_ISR() 
{
        adc_raw_voltage = *s_hadc->jdr;
        VoltageSense_CalculateVoltage();
}