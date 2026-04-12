/*
 * current_sense.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

//#include "App/config.h"
#include "BSP/current_sense.h"
#include "FOC/foc_loop.h"
#include "App/config.h"
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

static void CurrentSense_CalibrateOffset(void)
{
    uint32_t sum_a = 0, sum_b = 0, sum_c = 0;
    const int samples = 1000;

#ifdef MODE_3PWM
    HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_RESET);
#endif

    for (int i = 0; i < samples; ++i)
    {
        HAL_ADCEx_InjectedPollForConversion(s_hadc, HAL_MAX_DELAY);

        sum_a += HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_1);
        sum_b += HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_2);
        sum_c += HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_3);
    }

    offset_a = sum_a / samples;
    offset_b = sum_b / samples;
    offset_c = sum_c / samples;

    is_calibrated = true;

    printf("Offset A: %u, B: %u, C: %u \r\n", offset_a, offset_b, offset_c);
}

void CurrentSense_Init(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == ADC1){

	    s_hadc = hadc;

	    HAL_ADCEx_InjectedStop(s_hadc);
	    HAL_ADCEx_InjectedStart(s_hadc);

	    CurrentSense_CalibrateOffset();

	    HAL_ADCEx_InjectedStop(s_hadc);
	}
}

void CurrentSense_Process_ISR() {
    if (is_calibrated) {
        adc_raw_phase_a = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_1);
        adc_raw_phase_b = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_2);
        adc_raw_phase_c = HAL_ADCEx_InjectedGetValue(s_hadc, ADC_INJECTED_RANK_3);
    }
}

void CurrentSense_CalculatePhases(){

    if (is_calibrated)
    {
        // ADC -> napięcie
        float voltage_a = ((float)(adc_raw_phase_a - offset_a) * ADC_REF_VOLTAGE / ADC_RESOLUTION);
        float voltage_b = ((float)(adc_raw_phase_b - offset_b) * ADC_REF_VOLTAGE / ADC_RESOLUTION);
        float voltage_c = ((float)(adc_raw_phase_c - offset_c) * ADC_REF_VOLTAGE / ADC_RESOLUTION);

        // Napięcie -> prąd
        current_a = - voltage_a / (SHUNT_RESISTOR * CURRENT_SENSE_GAIN);
        current_b = - voltage_b / (SHUNT_RESISTOR * CURRENT_SENSE_GAIN);
        current_c = - voltage_c / (SHUNT_RESISTOR * CURRENT_SENSE_GAIN);
    }
}

void CurrentSense_Read(abc_current_t *currents)
{
    currents->a = current_a;
    currents->b = current_b;
    currents->c = current_c;
}

void CurrentSense_GetRaw(abc_raw_t *raw)
{
    raw->a = adc_raw_phase_a;
    raw->b = adc_raw_phase_b;
    raw->c = adc_raw_phase_c;
    raw->c = 0;
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) // 20 kHz
    {
    	if (__HAL_TIM_IS_TIM_COUNTING_DOWN(FOC_GetPwmTimer())) // 10 kHz
		{
			CurrentSense_Process_ISR();
			currents_ready = true;

			// Sygnalizacja wykonania przerwania - obserwacja oscyloskopem
//			ADC_Conv_Flag_GPIO_Port->BSRR = ADC_Conv_Flag_Pin; // GPIO_PIN_SET
//			ADC_Conv_Flag_GPIO_Port->BSRR = (uint32_t)ADC_Conv_Flag_Pin << 16; // GPIO_PIN_RESET
		}
    }
}




