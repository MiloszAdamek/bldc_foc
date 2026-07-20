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

    typedef enum
    {
        POWERSTATE_ALL_LOW = 0,   // LS ON
        POWERSTATE_ALL_HIGH       // HS ON
    } PowerTestState_t;

#endif

typedef enum
{
    POWERSTAGE_OK = 0,
    POWERSTAGE_ERROR
} PowerStage_Status_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint32_t      alternate;   // AF dla PWM
} PowerPin_t;

typedef struct
{
    PowerPin_t IN_H_A;
    PowerPin_t IN_H_B;
    PowerPin_t IN_H_C;

    PowerPin_t IN_L_A;
    PowerPin_t IN_L_B;
    PowerPin_t IN_L_C;

} PowerStage_Pins_t;

typedef struct
{
    PowerStage_Pins_t pins;
    TIM_HandleTypeDef *htim;  // Timer do generowania PWM
    PowerStage_Status_t status;
    PowerStage_PWM_Mode_t pwm_mode;
} PowerStage_HandleTypeDef;

typedef enum
{
    POWERSTAGE_PWM_MODE_3PWM = 0,
    POWERSTAGE_PWM_MODE_6PWM
} PowerStage_PWM_Mode_t;

PowerStage_Status_t PowerStage_Init(void);
PowerStage_Status_t PowerStage_On(void);
PowerStage_Status_t PowerStage_Off(void);
PowerStage_Status_t PowerStage_StartPWM(TIM_HandleTypeDef *htim);
PowerStage_Status_t PowerStage_StopPWM(TIM_HandleTypeDef *htim);
PowerStage_Status_t PowerStage_Tests(void);
PowerStage_Status_t PowerStage_CheckFaults(void);
PowerStage_Status_t PowerStage_SetPWMMode(PowerStage_PWM_Mode_t pwm_mode);
PowerStage_Status_t Force_AllHalfBridges(PowerTestState_t state);

#endif /* POWERSTAGE_H */