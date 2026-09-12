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

static volatile float v_bus_meas = 0;
 
void VoltageSense_Init(ADC_HandleTypeDef *hadc) {
    s_hadc = hadc;
    HAL_ADC_Start(hadc);
}

void VoltageSense_UpdateADCCoefficient(float vdd_voltage) {
    s_adc_to_voltage = VOLTAGE_SENSE_DIV_RATIO * vdd_voltage / (float)ADC_RESOLUTION;
}

void VoltageSense_ReadVbus(void) {
    // Sprawdzenie sprzętowej flagi konwersji (End Of Conversion) w rejestrze ISR
    if (s_hadc->Instance->ISR & ADC_ISR_EOC) {
        
        // Odczyt rejestru danych (DR) automatycznie i sprzętowo kasuje flagę EOC
        uint32_t adc_value = s_hadc->Instance->DR;
        v_bus_meas = (float)adc_value * s_adc_to_voltage;
    }
    
    // Wymuszenie startu NOWEJ konwersji poprzez rejestr Control Register (CR).
    // Ustawienie bitu ADSTART omija całą programową maszynę stanów HAL.
    s_hadc->Instance->CR |= ADC_CR_ADSTART;
}

float VoltageSense_GetVbus_ISR(void){
    return v_bus_meas;
}