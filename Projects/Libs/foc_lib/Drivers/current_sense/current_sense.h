/*
 * current_sense.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_CURRENT_SENSE_H_
#define INC_CURRENT_SENSE_H_

#include "stm32g4xx_hal.h"
#include "motor_types.h"

// Inicjalizacja pomiaru prądów
void CurrentSense_Init(ADC_InjectedChannel_t *hadc_inj, float vdd_voltage);

// Start pomiaru prądów w trybie przerwań
void CurrentSense_InjectedStart_IT(ADC_HandleTypeDef *hadc);

// Stop pomiaru prądów w trybie przerwań
void CurrentSense_InjectedStop_IT(ADC_HandleTypeDef *hadc);

// Wyliczenie pradów
void CurrentSense_CalculatePhases();

// Realizacja pomiaru - funkcja blokująca
void CurrentSense_Process_ISR();

// Zwraca zmierzone prądy fazowe
void CurrentSense_Read(abc_current_t *currents);

// Zwraca surowe wartości ADC
void CurrentSense_GetRaw(abc_raw_t *raw);

// Aktualizacja współczynnika przeliczeniowego ADC na prąd w zależności od napięcia zasilania
void CurrentSense_UpdateADCCoefficient(float vdd_voltage);

#endif /* INC_CURRENT_SENSE_H_ */
