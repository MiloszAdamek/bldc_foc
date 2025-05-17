/**
  ******************************************************************************
  * @file    speed_ctrl.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file contains all definitions and functions prototypes for the
  *          Speed & Torque Control component of the Motor Control SDK.
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
  * @ingroup SpeednTorqCtrl
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPEEDCTRLCLASS_H
#define __SPEEDCTRLCLASS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "mc_type.h"
#include "pid_regulator.h"
#include "speed_pos_fdbk.h"

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup SpeedCtrl
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  Speed Control parameters definition
  */
typedef struct
{
  MC_ControlMode_t Mode;   /*!< Modality of STC. It can be one of these two
                               settings: STC_TORQUE_MODE to enable the
                               Torque mode or STC_SPEED_MODE to enable the
                               Speed mode.*/
  int16_t TargetFinal;  /*!< Backup of hTargetFinal to be applied in the
                             last step.*/
  int32_t SpeedRefUnitExt; /*!< Current mechanical rotor speed reference
                                     expressed in SPEED_UNIT multiplied by
                                     65536.*/
  uint32_t DutyCycleRef;     /*!< Current pulse reference.*/
  uint32_t RampRemainingStep;/*!< Number of steps remaining to complete the
                                     ramp.*/
  PID_Handle_t * PISpeed;   /*!< The regulator used to perform the speed
                                     control loop.*/
  SpeednPosFdbk_Handle_t * SPD;/*!< The speed sensor used to perform the speed
                                     regulation.*/
  int32_t IncDecAmount; /*!< Increment/decrement amount to be applied to
                                     the reference value at each
                                     CalcSpeedReference.*/

  uint16_t STCFrequencyHz;             /*!< Frequency on which the user updates
                                             the torque reference calling
                                             STC_CalcTorqueReference method
                                             expressed in Hz */
  uint16_t MaxAppPositiveMecSpeedUnit; /*!< Application maximum positive value
                                             of the rotor mechanical speed. Expressed in
                                             the unit defined by #SPEED_UNIT.*/
  uint16_t MinAppPositiveMecSpeedUnit; /*!< Application minimum positive value
                                             of the rotor mechanical speed. Expressed in
                                             the unit defined by #SPEED_UNIT.*/
  int16_t MaxAppNegativeMecSpeedUnit;  /*!< Application maximum negative value
                                             of the rotor mechanical speed. Expressed in
                                             the unit defined by #SPEED_UNIT.*/
  int16_t MinAppNegativeMecSpeedUnit;  /*!< Application minimum negative value
                                             of the rotor mechanical speed. Expressed in
                                             the unit defined by #SPEED_UNIT.*/
  uint16_t MaxPositiveDutyCycle;          /*!< Maximum positive value of motor
                                             torque. This value represents
                                             actually the maximum Iq current
                                             expressed in digit.*/
  MC_ControlMode_t ModeDefault;          /*!< Default STC modality.*/
  int16_t MecSpeedRefUnitDefault;      /*!< Default mechanical rotor speed
                                             reference expressed in the unit
                                             defined by #SPEED_UNIT.*/
  uint16_t DutyCycleRefDefault;            /*!< Default pulse reference.*/
} SpeednTorqCtrl_Handle_t;



/* It initializes all the object variables */
void STC_Init( SpeednTorqCtrl_Handle_t * pHandle, PID_Handle_t * oPI, SpeednPosFdbk_Handle_t * oSPD );

/* It resets the integral term of speed regulator */
void STC_Clear( SpeednTorqCtrl_Handle_t * pHandle );

/* Get the current mechanical rotor speed reference. Expressed in the unit defined by SPEED_UNIT.*/
int16_t STC_GetMecSpeedRefUnit( SpeednTorqCtrl_Handle_t * pHandle );

/*  Get the current motor duty cycle reference. */
uint16_t STC_GetDutyCycleRef(SpeednTorqCtrl_Handle_t *pHandle);

/* Set the mode of the speed controller (Torque mode or Speed mode)*/
void STC_SetControlMode( SpeednTorqCtrl_Handle_t * pHandle, MC_ControlMode_t bMode );

/* Starts the execution of a ramp using new target and duration. */
bool STC_ExecRamp( SpeednTorqCtrl_Handle_t * pHandle, int16_t hTargetFinal, uint32_t hDurationms );

/* It computes the new value of motor speed reference */
uint16_t STC_CalcSpeedReference(SpeednTorqCtrl_Handle_t *pHandle);

/* Get the Default mechanical rotor speed reference. Expressed in the unit defined by SPEED_UNIT.*/
int16_t STC_GetMecSpeedRefUnitDefault( SpeednTorqCtrl_Handle_t * pHandle );

/* Returns the Application maximum positive rotor mechanical speed. Expressed in the unit defined by SPEED_UNIT.*/
uint16_t STC_GetMaxAppPositiveMecSpeedUnit( SpeednTorqCtrl_Handle_t * pHandle );

/* Returns the Application minimum negative rotor mechanical speed. Expressed in the unit defined by SPEED_UNIT.*/
int16_t STC_GetMinAppNegativeMecSpeedUnit( SpeednTorqCtrl_Handle_t * pHandle );

/* Stop the execution of speed ramp. */
bool STC_StopSpeedRamp( SpeednTorqCtrl_Handle_t * pHandle );

/* It sets in real time the speed sensor utilized by the 6step. */
void STC_SetSpeedSensor( SpeednTorqCtrl_Handle_t * pHandle, SpeednPosFdbk_Handle_t * oSPD );

/* Force the speed reference to the current speed */
void STC_ForceSpeedReferenceToCurrentSpeed( SpeednTorqCtrl_Handle_t * pHandle );

/**
  * @brief  Get the modality of the speed controller.
  * @param  pHandle: handler of the current instance of the SpeednTorqCtrl component
  * @retval MC_ControlMode_t It returns the modality of STC. It can be one of
  *         these two values: MCM_TORQUE_MODE or MCM_SPEED_MODE.
  */
static inline MC_ControlMode_t STC_GetControlMode( SpeednTorqCtrl_Handle_t * pHandle )
{
  return ((MC_NULL == pHandle) ? MCM_TORQUE_MODE : pHandle->Mode);
}

/**
  * @brief  This command interrupts the execution of any previous ramp command.
  *         The last value of mechanical rotor speed reference is maintained.
  * @param  pHandle: handler of the current instance of the SpeednTorqCtrl component
  * @retval none
  */
static inline void STC_StopRamp(SpeednTorqCtrl_Handle_t *pHandle)
{
  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
    pHandle->RampRemainingStep = 0U;
    pHandle->IncDecAmount = 0;
  }
}

/**
  * @brief  Check if the settled speed ramp has been completed.
  * @param  pHandle: handler of the current instance of the SpeednTorqCtrl component
  * @retval bool It returns true if the ramp is completed, false otherwise.
  */
static inline bool STC_RampCompleted(SpeednTorqCtrl_Handle_t *pHandle)
{
  bool retVal = false;

  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
    if (0U == pHandle->RampRemainingStep)
    {
      retVal = true;
    }
  }
  return (retVal);
}

/**
  * @brief It returns the speed sensor utilized by the FOC.
  * @param  pHandle: handler of the current instance of the SpeednTorqCtrl component
  * @retval SpeednPosFdbk_Handle_t speed sensor utilized by the FOC.
  */
static inline SpeednPosFdbk_Handle_t * STC_GetSpeedSensor( SpeednTorqCtrl_Handle_t * pHandle )
{
  return ((MC_NULL ==  pHandle) ? MC_NULL : pHandle->SPD);
}

/**
  * @}
  */

/**
  * @}
  */
#ifdef __cplusplus
}
#endif /* __cpluplus */

#endif /* __SPEEDCTRLCLASS_H */

/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/

