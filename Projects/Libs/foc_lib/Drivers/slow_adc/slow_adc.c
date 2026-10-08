/*
 * slow_adc.c
 *
 *  Created on: Sep 28, 2026
 *      Author: Milosz Adamek
 */

#include "board.h"
#include "ntc_sense.h"
#include "slow_adc.h"

#define NUM_SLOW_ADC_CHANNELS 2

static ADC_HandleTypeDef *s_hadc;
static float s_adc_to_voltage;
static volatile SlowLoopMeasurements_t slow_loop_measurements = {0};

static uint16_t adc_raw_buff[NUM_SLOW_ADC_CHANNELS];
volatile bool slow_adc_ready = false;

static float s_vdd_voltage = 3.3f;

static TemperatureSensor_t motor_temp_sensor = 
{
    .R25      = 10000.0f, // 10k ohm at 25°C
    .B        = 3375.0f,  // Beta value
    .R_series = 10000.0f, // Series resistor value
    .V_supply = 3.3f,     // Supply voltage
    .V_ref    = 3.3f,     // ADC reference voltage
    .ADC_resolution = 4095.0f, // 12-bit ADC resolution
};

static TemperatureSensor_t mosfet_temp_sensor = 
{
    .R25      = 10000.0f, // 10k ohm at 25°C
    .B        = 3350.0f,  // Beta value
    .R_series = 10000.0f, // Series resistor value
    .V_supply = 3.3f,     // Supply voltage
    .V_ref    = 3.3f,     // ADC reference voltage
    .ADC_resolution = 4095.0f, // 12-bit ADC resolution
};

void SlowADC_Init(ADC_HandleTypeDef *hadc, float vdd_voltage) 
{
    s_hadc = hadc;
    s_vdd_voltage = vdd_voltage;
    SlowADC_UpdateADCCoefficient();
    HAL_ADC_Start_DMA(hadc, (uint32_t*)&adc_raw_buff, NUM_SLOW_ADC_CHANNELS);
}

void SlowADC_Trigger(void) 
{
    // Wymuszenie startu sekwencji 3 kanałów regular przez software
    if (!(s_hadc->Instance->CR & ADC_CR_ADSTART)) {
        s_hadc->Instance->CR |= ADC_CR_ADSTART;
    }
}

void SlowADC_UpdateADCCoefficient() 
{
    s_adc_to_voltage = VOLTAGE_SENSE_DIV_RATIO * s_vdd_voltage / (float)ADC_RESOLUTION;
}

void SlowADC_CalculateMeasurements(void) 
{

    // slow_loop_measurements.v_bus = (float)adc_raw_buff[0] * s_adc_to_voltage;
    slow_loop_measurements.mosfet_temp = NTC_CalculateTemperature_VCC(&mosfet_temp_sensor, (float)adc_raw_buff[0]);
    slow_loop_measurements.motor_temp = NTC_CalculateTemperature_GND(&motor_temp_sensor, (float)adc_raw_buff[1]);
}

float SlowADC_GetMotorTemperature_ISR() 
{
    return slow_loop_measurements.motor_temp;
}

float SlowADC_GetMosfetTemperature_ISR() 
{
    return slow_loop_measurements.mosfet_temp;
}

float SlowADC_GetVBusVoltage_ISR() 
{
    return slow_loop_measurements.v_bus;
}

SlowLoopMeasurements_t SlowADC_GetData_ISR(void)
{
    return slow_loop_measurements;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if(hadc == s_hadc)
    {
        slow_adc_ready = true;
    }
}