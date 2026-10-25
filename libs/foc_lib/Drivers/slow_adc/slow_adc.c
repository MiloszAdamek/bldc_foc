/*
 * slow_adc.c
 *
 *  Created on: Sep 28, 2026
 *      Author: Milosz Adamek
 */

#include "board.h"
#include "slow_adc.h"
#include "math.h"

static ADC_HandleTypeDef *s_hadc;
static float s_adc_to_voltage;
static volatile SlowLoopMeasurements_t slow_loop_measurements = {0};
static uint16_t adc_raw_buff[3];
volatile bool slow_adc_ready = false;

/* Współczynniki filtru IIR dla fs = 10 kHz (dt = 100 us): */
#define VBUS_LPF_ALPHA  0.239f   /* fc ≈ 500 Hz (tau ≈ 318 us) - szybka reakcja choppera */
#define TEMP_LPF_ALPHA  0.00125f /* fc ≈ 2 Hz   (tau ≈ 80 ms)  - silne tłumienie szumu NTC */

/* Stany filtrów na surowych wartościach ADC */
static float s_vbus_adc_filt       = 0.0f;
static float s_mosfet_ntc_adc_filt = 0.0f;
static float s_motor_ntc_adc_filt  = 0.0f;
static bool  s_filter_initialized  = false;

static float s_vdd_voltage = 3.3f;

static TemperatureSensor_t motor_temp_sensor = 
{
    .R25            = 10000.0f,
    .B              = 3375.0f,
    .R_series       = 10000.0f,
    .V_supply       = 3.3f,
    .V_ref          = 3.3f,
    .ADC_resolution = 4095.0f,
};

static TemperatureSensor_t mosfet_temp_sensor = 
{
    .R25            = 10000.0f,
    .B              = 3350.0f,
    .R_series       = 10000.0f,
    .V_supply       = 3.3f,
    .V_ref          = 3.3f,
    .ADC_resolution = 4095.0f,
};

void SlowADC_Init(ADC_HandleTypeDef *hadc, float vdd_voltage) 
{
    s_hadc = hadc;
    s_vdd_voltage = vdd_voltage;
    s_filter_initialized = false;
    SlowADC_UpdateADCCoefficient();
    HAL_ADC_Start_DMA(hadc, (uint32_t*)adc_raw_buff, 3);
}

void SlowADC_Trigger(void) 
{
    if (!(s_hadc->Instance->CR & ADC_CR_ADSTART)) {
        s_hadc->Instance->CR |= ADC_CR_ADSTART;
    }
}

void SlowADC_UpdateADCCoefficient(void) 
{
    s_adc_to_voltage = VOLTAGE_SENSE_DIV_RATIO * s_vdd_voltage / (float)ADC_RESOLUTION;
}

float NTC_CalculateTemperature_VCC(TemperatureSensor_t *sensor, float adc_value) 
{
    if (adc_value < 10.0f) return -40.0f; // Rozwarcie
    if (adc_value >= (sensor->ADC_resolution - 10.0f)) return 150.0f; // Zwarcie

    float r_ntc = sensor->R_series * ((sensor->ADC_resolution - adc_value) / adc_value);
    float inv_T = (1.0f / 298.15f) + (1.0f / sensor->B) * logf(r_ntc / sensor->R25);
    return (1.0f / inv_T) - 273.15f;
}

float NTC_CalculateTemperature_GND(TemperatureSensor_t *sensor, float adc_value)
{
    if (adc_value < 10.0f) return 150.0f; // Zwarcie
    if (adc_value >= (sensor->ADC_resolution - 10.0f)) return -40.0f; // Rozwarcie

    float r_ntc = sensor->R_series * (adc_value / (sensor->ADC_resolution - adc_value));
    float inv_T = (1.0f / 298.15f) + (1.0f / sensor->B) * logf(r_ntc / sensor->R25);
    return (1.0f / inv_T) - 273.15f;
}

void SlowADC_CalculateMeasurements(void) 
{
    float raw_vbus   = (float)adc_raw_buff[0];
    float raw_mosfet = (float)adc_raw_buff[1];
    float raw_motor  = (float)adc_raw_buff[2];

    if (!s_filter_initialized) {
        s_vbus_adc_filt       = raw_vbus;
        s_mosfet_ntc_adc_filt = raw_mosfet;
        s_motor_ntc_adc_filt  = raw_motor;
        s_filter_initialized  = true;

        slow_loop_measurements.v_bus       = s_vbus_adc_filt * s_adc_to_voltage;
        slow_loop_measurements.mosfet_temp = NTC_CalculateTemperature_VCC(&mosfet_temp_sensor, s_mosfet_ntc_adc_filt);
        slow_loop_measurements.motor_temp  = NTC_CalculateTemperature_GND(&motor_temp_sensor, s_motor_ntc_adc_filt);
        return;
    }

    /* 1. Aktualizacja filtrów IIR przy pełnym 10 kHz (tylko 3 mnożenia!) */
    s_vbus_adc_filt       += VBUS_LPF_ALPHA * (raw_vbus   - s_vbus_adc_filt);
    s_mosfet_ntc_adc_filt += TEMP_LPF_ALPHA * (raw_mosfet - s_mosfet_ntc_adc_filt);
    s_motor_ntc_adc_filt  += TEMP_LPF_ALPHA * (raw_motor  - s_motor_ntc_adc_filt);

    /* 2. Vbus przeliczamy zawsze przy 10 kHz (pod Braking Chopper i SVPWM) */
    slow_loop_measurements.v_bus = s_vbus_adc_filt * s_adc_to_voltage;

    /* 3. Decymacja logf(): temperatury przeliczamy z częstotliwością 100 Hz,
     * rozdzielając oba logarytmy na różne cykle (brak piku obciążenia CPU) */
    static uint8_t temp_div = 0;
    temp_div++;
    if (temp_div == 50) {
        slow_loop_measurements.mosfet_temp = NTC_CalculateTemperature_VCC(&mosfet_temp_sensor, s_mosfet_ntc_adc_filt);
    } else if (temp_div >= 100) {
        slow_loop_measurements.motor_temp  = NTC_CalculateTemperature_GND(&motor_temp_sensor, s_motor_ntc_adc_filt);
        temp_div = 0;
    }
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
        // slow_adc_ready = true;
        SlowADC_CalculateMeasurements();
    }
}