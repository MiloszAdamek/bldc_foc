/*
 * current_sense.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#include "App/config.h"
#include "BSP/current_sense.h"
#include "FOC/foc_loop.h"
#include <stdbool.h>
#include <stdio.h>
#include "main.h"

#define ADC_REF_VOLTAGE   3.3f       // Vref zasilania ADC
#define ADC_RESOLUTION    4096.0f    // 12-bit ADC

static ADC_HandleTypeDef *hadc_local;

static uint16_t adc_raw_phase_a = 0;
static uint16_t adc_raw_phase_b = 0;
//static uint16_t adc_raw_phase_c = 0;

static float current_a = 0.0f;
static float current_b = 0.0f;
static float current_c = 0.0f;

static uint16_t offset_a = 0;
static uint16_t offset_b = 0;
//static uint16_t offset_c = 0;

volatile bool is_calibrated = false;

volatile uint32_t adc_inj_irq_cnt = 0;

static void CurrentSense_CalibrateOffset(void)
{
    uint32_t sum_a = 0, sum_b = 0;
    const int samples = 1000;

    HAL_GPIO_WritePin(PWM_EN_FAULT_GPIO_Port, PWM_EN_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_W_GPIO_Port, PWM_EN_W_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_V_GPIO_Port, PWM_EN_V_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWM_EN_U_GPIO_Port, PWM_EN_U_Pin, GPIO_PIN_RESET);

    for (int i = 0; i < samples; ++i)
    {
        HAL_ADCEx_InjectedPollForConversion(hadc_local, HAL_MAX_DELAY);

        sum_a += HAL_ADCEx_InjectedGetValue(hadc_local, ADC_INJECTED_RANK_1);
        sum_b += HAL_ADCEx_InjectedGetValue(hadc_local, ADC_INJECTED_RANK_2);
//        sum_c += HAL_ADCEx_InjectedGetValue(hadc_local, ADC_INJECTED_RANK_3);
    }

    offset_a = sum_a / samples;
    offset_b = sum_b / samples;
//    offset_c = sum_c / samples;

    is_calibrated = true;

    printf("Offset A: %u, B: %u \r\n", offset_a, offset_b);
}

void CurrentSense_Init(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == ADC1){

	    hadc_local = hadc;

	    HAL_ADCEx_InjectedStop(hadc_local);
	    HAL_ADCEx_InjectedStart(hadc_local);

	    CurrentSense_CalibrateOffset();

	    HAL_ADCEx_InjectedStop(hadc_local);
	}
}

void CurrentSense_Process_ISR() {
    if (is_calibrated) {
        adc_raw_phase_a = HAL_ADCEx_InjectedGetValue(hadc_local, ADC_INJECTED_RANK_1);
        adc_raw_phase_b = HAL_ADCEx_InjectedGetValue(hadc_local, ADC_INJECTED_RANK_2);
//        adc_raw_phase_c = HAL_ADCEx_InjectedGetValue(hadc_local, ADC_INJECTED_RANK_3);
    }
}

void CurrentSense_CalculatePhases(){

    if (is_calibrated)
    {
        // ADC -> napięcie
        float voltage_a = ((float)(adc_raw_phase_a - offset_a) * ADC_REF_VOLTAGE / ADC_RESOLUTION);
        float voltage_b = ((float)(adc_raw_phase_b - offset_b) * ADC_REF_VOLTAGE / ADC_RESOLUTION);
//        float voltage_c = ((float)(adc_raw_phase_c - offset_c) * ADC_REF_VOLTAGE / ADC_RESOLUTION);

        // Napięcie -> prąd
        current_a = - voltage_a / (SHUNT_RESISTOR * CURRENT_SENSE_GAIN);
        current_b = - voltage_b / (SHUNT_RESISTOR * CURRENT_SENSE_GAIN);
//        current_c = - voltage_c / (SHUNT_RESISTOR * CURRENT_SENSE_GAIN);
        current_c = - (current_a + current_b);
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
//    raw->c = adc_raw_phase_c;
    raw->c = 0;
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) // Przerwanie 20kHz
    {
    	if (__HAL_TIM_IS_TIM_COUNTING_DOWN(foc_htim)) // 10 kHz
		{
//			HAL_GPIO_WritePin(ADC_Conv_Flag_GPIO_Port, ADC_Conv_Flag_Pin, GPIO_PIN_SET);

			CurrentSense_Process_ISR();
			new_current_data_ready = true;
			adc_inj_irq_cnt++;

//			HAL_GPIO_WritePin(ADC_Conv_Flag_GPIO_Port, ADC_Conv_Flag_Pin, GPIO_PIN_RESET);

			ADC_Conv_Flag_GPIO_Port->BSRR = ADC_Conv_Flag_Pin; // GPIO_PIN_SET
			ADC_Conv_Flag_GPIO_Port->BSRR = (uint32_t)ADC_Conv_Flag_Pin << 16; // GPIO_PIN_RESET
		}
    }
}




