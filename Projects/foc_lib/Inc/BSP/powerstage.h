/*
 * powerstage.h
 *
 *  Created on: Jul 18, 2026
 *      Author: Milosz Adamek
 */

#ifndef POWERSTAGE_H
#define POWERSTAGE_H

#include "App/config.h"
#include "tim.h"

#if !defined(DRV8353) && !defined(IHM03)
#error "No power stage defined!"
#endif

// typedef enum
// {
//     POWERSTAGE_OK = 0,
//     POWERSTAGE_ERROR
// } PowerStage_Status_t;

#ifdef G431_ESC
    #define SHUNT_RESISTOR        0.003f
    #define CURRENT_SENSE_GAIN    16.0f
#endif

#ifdef IHM03
    #define SHUNT_RESISTOR        0.33f
    #define CURRENT_SENSE_GAIN    1.528f
#endif

#ifdef DRV8353
    #define SHUNT_RESISTOR        0.005f
    #define CURRENT_SENSE_GAIN    20.0f
#endif

void PowerStage_Init(void);
void PowerStage_On(void);
void PowerStage_Off(void);
void PowerStage_StartPWM(TIM_HandleTypeDef *htim);
void PowerStage_StopPWM(TIM_HandleTypeDef *htim);

#endif /* POWERSTAGE_H */