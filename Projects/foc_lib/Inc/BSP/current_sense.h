/*
 * current_sense.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_CURRENT_SENSE_H_
#define INC_CURRENT_SENSE_H_

#include "stm32g4xx_hal.h"

typedef struct {
    float a;
    float b;
    float c;
} abc_current_t;

typedef struct {
    uint16_t a;
    uint16_t b;
    uint16_t c;
} abc_raw_t;

// Inicjalizacja pomiaru prądów
void CurrentSense_Init(ADC_HandleTypeDef *hadc);

// Start pomiaru prądów w trybie przerwań
void CurrentSense_InjectedStart_IT(ADC_HandleTypeDef *hadc);

// Wyliczenie pradów
void CurrentSense_CalculatePhases();

// Realizacja pomiaru - funkcja blokująca
void CurrentSense_Process_ISR();

// Zwraca zmierzone prądy fazowe
void CurrentSense_Read(abc_current_t *currents);

// Zwraca surowe wartości ADC
void CurrentSense_GetRaw(abc_raw_t *raw);

#endif /* INC_CURRENT_SENSE_H_ */
