/*
 * current_sense.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

#include "current_sense.h"
#include "foc_loop.h"
#include "config.h"
#include "powerstage.h"
#include "stdbool.h"

static ADC_HandleTypeDef *s_hadc;

static volatile uint16_t adc_raw_phase_a = 0;
static volatile uint16_t adc_raw_phase_b = 0;
static volatile uint16_t adc_raw_phase_c = 0;

static float current_a = 0.0f;
static float current_b = 0.0f;
static float current_c = 0.0f;

static uint16_t offset_a = 0;
static uint16_t offset_b = 0;
static uint16_t offset_c = 0;

volatile bool is_calibrated = false;

static float s_adc_to_current;

static void CurrentSense_CalibrateOffset(ADC_InjectedChannel_t *hadc_inj)
{
    uint32_t sum_a = 0, sum_b = 0, sum_c = 0;
    const int samples = 1000;

    // Make sure to shut down inverter before calibration to avoid current flow

    for (int i = 0; i < samples; ++i)
    {
        HAL_ADCEx_InjectedPollForConversion(hadc_inj->hadc, HAL_MAX_DELAY);
        sum_a += HAL_ADCEx_InjectedGetValue(hadc_inj->hadc, hadc_inj->rank);
        sum_b += HAL_ADCEx_InjectedGetValue(hadc_inj->hadc, ADC_INJECTED_RANK_2);
        #ifdef CURRENT_SENSE_TRIPLE_SHUNT
            sum_c += HAL_ADCEx_InjectedGetValue(hadc_inj->hadc, ADC_INJECTED_RANK_3);
        #endif
    }

    offset_a = sum_a / samples;
    offset_b = sum_b / samples;
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        offset_c = sum_c / samples;
    #endif

    is_calibrated = true;
    printf("Offset A: %u, B: %u, C: %u \r\n", offset_a, offset_b, offset_c);
}

void CurrentSense_Init(ADC_InjectedChannel_t *hadc_inj, float vdd_voltage) {
    s_hadc = hadc_inj->hadc;

    CurrentSense_UpdateADCCoefficient(vdd_voltage);

    HAL_ADCEx_InjectedStop(s_hadc);
    HAL_ADCEx_InjectedStart(s_hadc);

    CurrentSense_CalibrateOffset(hadc_inj);

    HAL_ADCEx_InjectedStop(s_hadc);
}

void CurrentSense_UpdateADCCoefficient(float vdd_voltage) {
    s_adc_to_current = vdd_voltage / ((float)ADC_RESOLUTION * SHUNT_RESISTOR * CURRENT_SENSE_GAIN);
}

void CurrentSense_InjectedStart_IT(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1){
        HAL_ADCEx_InjectedStart_IT(s_hadc);
    }
}

void CurrentSense_InjectedStop_IT(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1){
        HAL_ADCEx_InjectedStop_IT(s_hadc);
    }
}

void CurrentSense_Process_ISR() {
	if (!is_calibrated) return;

	// adc_raw_phase_a = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_1);
    adc_raw_phase_a = s_hadc->Instance->JDR1;
	// adc_raw_phase_b = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_2);
    adc_raw_phase_b = s_hadc->Instance->JDR2;
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        // adc_raw_phase_c = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_3);
        adc_raw_phase_c = s_hadc->Instance->JDR3;
    #endif
}

#ifdef IHM03
void CurrentSense_CalculatePhases(){
    if (!is_calibrated) return;
    
    int32_t diff_a = (int32_t)adc_raw_phase_a - (int32_t)offset_a;
    int32_t diff_b = (int32_t)adc_raw_phase_b - (int32_t)offset_b;
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        int32_t diff_c = (int32_t)adc_raw_phase_c - (int32_t)offset_c;
    #endif

    current_a = -(float)diff_a * s_adc_to_current;
    current_b = -(float)diff_b * s_adc_to_current;
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        current_c = -(float)diff_c * s_adc_to_current;
    #else
        current_c = -(current_a + current_b);
    #endif
}
#endif

#ifdef DRV8353
void CurrentSense_CalculatePhases(){
    if (!is_calibrated) return;

    int32_t diff_a = (int32_t)adc_raw_phase_a - (int32_t)offset_a;
    int32_t diff_b = (int32_t)adc_raw_phase_b - (int32_t)offset_b;
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        int32_t diff_c = (int32_t)adc_raw_phase_c - (int32_t)offset_c;
    #endif

    current_a = (float)diff_a * s_adc_to_current;
    current_b = (float)diff_b * s_adc_to_current;
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        current_c = (float)diff_c * s_adc_to_current;
    #else
        current_c = -(current_a + current_b);
    #endif
}
#endif

void CurrentSense_Read(abc_current_t *currents)
{
    currents->a = current_a;
    currents->b = current_b;
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        currents->c = current_c;
    #endif
}

void CurrentSense_GetRaw(abc_raw_t *raw)
{
    raw->a = adc_raw_phase_a;
    raw->b = adc_raw_phase_b;
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        raw->c = adc_raw_phase_c;
    #endif
}
