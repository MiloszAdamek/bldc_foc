/*
 * current_sense.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

#include "BSP/current_sense.h"
#include "FOC/foc_loop.h"
#include "App/config.h"
#include "BSP/powerstage.h"
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

// static const float ADC_TO_CURRENT = ADC_REF_VOLTAGE / (ADC_RESOLUTION * SHUNT_RESISTOR * CURRENT_SENSE_GAIN);

static float s_adc_to_current;

static void CurrentSense_CalibrateOffset(void)
{
    uint32_t sum_a = 0, sum_b = 0, sum_c = 0;
    const int samples = 1000;

    // Make sure to shut down inverter before calibration to avoid current flow

    for (int i = 0; i < samples; ++i)
    {
        HAL_ADCEx_InjectedPollForConversion(s_hadc, HAL_MAX_DELAY);
        sum_a += HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_1);
        sum_b += HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_2);
        #ifdef CURRENT_SENSE_TRIPLE_SHUNT
            sum_c += HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_3);
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

void CurrentSense_Init(ADC_HandleTypeDef *hadc, float vdd_voltage) {
	if (hadc->Instance == ADC1){

	    s_hadc = hadc;

        CurrentSense_UpdateADCCoefficient(vdd_voltage); // Default value, will be updated later

	    HAL_ADCEx_InjectedStop(s_hadc);
	    HAL_ADCEx_InjectedStart(s_hadc);

	    CurrentSense_CalibrateOffset();

	    HAL_ADCEx_InjectedStop(s_hadc);
	}
}

void CurrentSense_UpdateADCCoefficient(float vdd_voltage) {
    s_adc_to_current = vdd_voltage / ((float)ADC_RESOLUTION * SHUNT_RESISTOR * CURRENT_SENSE_GAIN);
}

void CurrentSense_InjectedStart_IT(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1){
        HAL_ADCEx_InjectedStart_IT(s_hadc);
    }
}

void CurrentSense_Process_ISR() {
	if (!is_calibrated) return;

	adc_raw_phase_a = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_1);
	adc_raw_phase_b = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_2);
    #ifdef CURRENT_SENSE_TRIPLE_SHUNT
        adc_raw_phase_c = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_3);
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

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) // 20 kHz
    {
    	if (__HAL_TIM_IS_TIM_COUNTING_DOWN(FOC_GetPwmTimer())) // 10 kHz
		{
			CurrentSense_Process_ISR();
			currents_ready = true;

            // Tylko do debugu, to ma zniknąć stąd
            CurrentSense_CalculatePhases();

            FOC_RunLoop();

			// Sygnalizacja wykonania przerwania - obserwacja oscyloskopem
//			ADC_Conv_Flag_GPIO_Port->BSRR = ADC_Conv_Flag_Pin; // GPIO_PIN_SET
//			ADC_Conv_Flag_GPIO_Port->BSRR = (uint32_t)ADC_Conv_Flag_Pin << 16; // GPIO_PIN_RESET
		}
    }
}




