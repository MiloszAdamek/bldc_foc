/**
  ******************************************************************************
  * @file    open_loop_sixstep.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file contains all definitions and functions prototypes for the
  *          openloop sixstep Control component of the Motor Control SDK.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  * @ingroup OpenLoop_SixStep
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OPENLOOPSIXSTEP_H
#define __OPENLOOPSIXSTEP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "mc_type.h"
#include "pid_regulator.h"
#include "speed_pos_fdbk.h"
#include "drive_parameters.h"
#include "regular_conversion_manager.h"

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup SixStep
  * @{
  */

/** @addtogroup OpenLoop_SixStep
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  Speed Control parameters definition
  */
typedef struct
{
  uint32_t DutyCycleRefTable[M1_COMMUTATION_STEP_TIME_BUF_SIZE];         /*!< DutyCycleRef table for open loop to compute mean of dutyCycleRef. */
  uint32_t DutyCycleRefMean;                                             /*!< Computed DutyCycleRef mean value. */
  uint16_t CurrentFactor;                                                /*!< percentage Range of duty cycle for current mode (percnetage of max period). */
  uint8_t DutyCycleRefCounter;                                           /*!< DutyCycleRef counter to start mean computation when sufficient number value are into the buffer. */
  uint8_t DutyCycleRef;                                                  /*!< Duty cycle =  (DutyCycleRef) value to set into timers. */
  uint8_t DutyCycleRefTableIndex;                                        /*!< DutyCycleRef table current pointer into the DutyCycleRefTable. */
  uint8_t DutyCycleRefTableSize;                                         /*!< DutyCycleRef table size 1 to 12. */
  uint8_t VoltageFactor;                                                 /*!< Range of duty cycle for voltage mode (percnetage of max period). */
  bool Openloop;                                                         /*!< OpenLopp flag to enable disable the feature, true = enabled, false = disabled. */
  bool RevUp;                                                            /*!< RevUp flag to enable disable the startup feature, true = enabled, false = disabled. */
  bool OnSensing;                                                        /*!< On sensing flag to enable disable the PWM ON Sensing feature, true = enabled, false = disabled. */

} OpenLoopSixstepCtrl_Handle_t;

/* Initializes all the object variables, usually it has to be called once right after object creation. */
void OLS_Init(OpenLoopSixstepCtrl_Handle_t *pHandle);

/* Returns the openloop status flag. */
bool OLS_GetOpenLoopFlag(OpenLoopSixstepCtrl_Handle_t *pHandle);

/*  It sets the openloop status flag in real time. */
void OLS_SetOpenLoopFlag(OpenLoopSixstepCtrl_Handle_t *pHandle, bool Flag);

/* It sets the Duty cycle reference table size in real time. */
void OLS_SetDutyCycleRefTableSize(OpenLoopSixstepCtrl_Handle_t *pHandle, uint8_t DutyCycleRefTableSize);

/* Returns the Duty cycle reference table size to compute a mean of the requested speed. */
uint8_t OLS_GetDutyCycleRefTableSize(OpenLoopSixstepCtrl_Handle_t *pHandle);

/* It sets the Duty cycle reference. */
void OLS_SetDutyCycleRef(OpenLoopSixstepCtrl_Handle_t *pHandle, uint8_t DutyCycleRef);

/* Returns the Duty cycle reference value. */
uint8_t OLS_GetDutyCycleRef(OpenLoopSixstepCtrl_Handle_t *pHandle);

/* It sets the Rev-up phase enable flag. */
void OLS_SetRevUpFlag(OpenLoopSixstepCtrl_Handle_t *pHandle, bool Flag);

/* Returns the Rev-up phase enable flag. */
bool OLS_GetRevUpFlag(OpenLoopSixstepCtrl_Handle_t *pHandle);

/* It is used to set the new value of duty cycle reference coming from the potentiometer (ADC). */
void OLS_Potentiometer_Run(OpenLoopSixstepCtrl_Handle_t *pHandle, uint16_t RawValue);

/* is used to compute the new value of motor PWM duty Cycle reference in openLoop Voltage mode. */
uint16_t OLS_CalcOpenLoopDutyCycleVM(uint16_t PWMperiod, OpenLoopSixstepCtrl_Handle_t *pHandle, MC_ControlMode_t Mode);

/* It is used to compute the new value of motor current duty Cycle reference in current mode. */
uint16_t OLS_CalcOpenLoopDutyCycleCM(uint16_t PWMperiod, OpenLoopSixstepCtrl_Handle_t *pHandle, MC_ControlMode_t Mode);

/* It sets Current factor in percentage unit. */
void OLS_SetCurrentFactor(OpenLoopSixstepCtrl_Handle_t *pHandle, uint16_t CurrentFactor);

/* Returns Current factor in percentage unit. */
uint16_t OLS_GetCurrentFactor(OpenLoopSixstepCtrl_Handle_t *pHandle);

/* It sets Voltage factor in percentage unit.. */
void OLS_SetVoltageFactor(OpenLoopSixstepCtrl_Handle_t *pHandle, uint8_t VoltageFactor);

/* Returns Voltage factor in percentage unit. */
uint8_t OLS_GetVoltageFactor(OpenLoopSixstepCtrl_Handle_t *pHandle);

/* It sets openloop BEMF Sensing ON command. */
void OLS_SetOnSensing(OpenLoopSixstepCtrl_Handle_t *pHandle, bool OnSensing);

/* Returns the openloop BEMF Sensing ON command. */
bool OLS_GetOnSensing(OpenLoopSixstepCtrl_Handle_t *pHandle);

/**
  * @}
  */

/**
  * @}
  */
/**
  * @}
  */

#ifdef __cplusplus
}
#endif /* __cpluplus */

#endif /* _OPENLOOPSIXSTEP_H */

/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/

