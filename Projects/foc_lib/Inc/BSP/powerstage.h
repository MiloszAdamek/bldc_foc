/*
 * powerstage.h
 *
 *  Created on: Jul 18, 2026
 *      Author: Milosz Adamek
 */

#ifndef POWERSTAGE_H
#define POWERSTAGE_H

#include "App/config.h"
#include "BSP/drv8353.h"
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
    #define SHUNT_RESISTOR        0.0005f
    #define CURRENT_SENSE_GAIN    40.0f

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

typedef enum
{
    POWERSTAGE_PWM_MODE_3PWM = DRV8353_PWM_MODE_3PWM,
    POWERSTAGE_PWM_MODE_6PWM = DRV8353_PWM_MODE_6PWM
} PowerStage_PWM_Mode_t;

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
    TIM_HandleTypeDef *htim;      // Timer PWM
    PowerStage_Pins_t pins;       // Piny IN_H / IN_L
    PowerStage_PWM_Mode_t pwm_mode;   // 3PWM / 6PWM
    PowerStage_Status_t status;
    DRV8353_HandleTypeDef drv;    // DRV8353 handle
} PowerStage_HandleTypeDef;

PowerStage_Status_t PowerStage_Init(PowerStage_HandleTypeDef *ps, SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim, const PowerStage_Pins_t *pins);
PowerStage_Status_t PowerStage_On(PowerStage_HandleTypeDef *ps);
PowerStage_Status_t PowerStage_Off(PowerStage_HandleTypeDef *ps);
PowerStage_Status_t PowerStage_Tests(PowerStage_HandleTypeDef *ps);
PowerStage_Status_t PowerStage_CheckFaults(PowerStage_HandleTypeDef *ps);

#endif /* POWERSTAGE_H */