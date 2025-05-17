/**
  ******************************************************************************
  * @file    speed_torq_ctrl.h
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
  * @ingroup SpeednTorqCtrlClassic
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SPEEDNTORQCTRLCLASS_H
#define SPEEDNTORQCTRLCLASS_H

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

/** @addtogroup SpeednTorqCtrl
  * @{
  */

/** @addtogroup SpeednTorqCtrlClassic
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  Speed & Torque Control parameters definition
  */
typedef struct
{
  MC_ControlMode_t Mode;               /*!< Modality of STC. It can be one of these two settings:
                                           MCM_TORQUE_MODE to enable the Torque mode or
                                           MCM_SPEED_MODE to enable the Speed mode. */
  int16_t TargetFinal;                 /*!< Backup of #hTargetFinal to be applied in the last step. */
  int32_t SpeedRefUnitExt;             /*!< Current mechanical rotor speed reference expressed in
                                            [SPEED_UNIT](measurement_units.md) multiplied by 65536.*/
  int32_t TorqueRef;                   /*!< Current motor torque reference. This value represents actually
                                            the Iq current expressed in digit multiplied by 65536. */
  uint32_t RampRemainingStep;          /*!< Number of steps remaining to complete the ramp. */
  PID_Handle_t *PISpeed;               /*!< The regulator used to perform the speed control loop. */
  SpeednPosFdbk_Handle_t *SPD;         /*!< The speed sensor used to perform the speed regulation. */
  int32_t IncDecAmount;                /*!< Increment/decrement amount to be applied to the reference value at each
                                            #CalcTorqueReference. */
  uint16_t STCFrequencyHz;             /*!< Frequency on which the user updates the torque reference calling
                                            #STC_CalcTorqueReference method expressed in Hz */
  uint16_t MaxAppPositiveMecSpeedUnit; /*!< Application maximum positive value of the rotor mechanical speed.
                                            Expressed in the unit defined by [SPEED_UNIT](measurement_units.md). */
  uint16_t MinAppPositiveMecSpeedUnit; /*!< Application minimum positive value of the rotor mechanical speed.
                                            Expressed in the unit defined by [SPEED_UNIT](measurement_units.md). */
  int16_t MaxAppNegativeMecSpeedUnit;  /*!< Application maximum negative value of the rotor mechanical speed.
                                            Expressed in the unit defined by [SPEED_UNIT](measurement_units.md). */
  int16_t MinAppNegativeMecSpeedUnit;  /*!< Application minimum negative value of the rotor mechanical speed.
                                            Expressed in the unit defined by [SPEED_UNIT](measurement_units.md). */
  uint16_t MaxPositiveTorque;          /*!< Maximum positive value of motor torque. This value represents actually
                                            the maximum Iq current expressed in digit. */
  int16_t MinNegativeTorque;           /*!< Minimum negative value of motor torque. This value represents actually
                                            the maximum Iq current expressed in digit. */
  MC_ControlMode_t ModeDefault;        /*!< Default STC modality. */
  int16_t MecSpeedRefUnitDefault;      /*!< Default mechanical rotor speed reference expressed in the unit defined by
                                            [SPEED_UNIT](measurement_units.md). */
  int16_t TorqueRefDefault;            /*!< Default motor torque reference. This value represents actually the Iq
                                            current reference expressed in digit. */
  int16_t IdrefDefault;                /*!< Default Id current reference expressed in digit. */
} SpeednTorqCtrl_Handle_t;



/* Initializes all the object variables */
void STC_Init(SpeednTorqCtrl_Handle_t *pHandle, PID_Handle_t *pPI, SpeednPosFdbk_Handle_t *SPD_Handle);

/* Resets the integral term of speed regulator */
void STC_Clear(SpeednTorqCtrl_Handle_t *pHandle);

/* Gets the current mechanical rotor speed reference */
int16_t STC_GetMecSpeedRefUnit(SpeednTorqCtrl_Handle_t *pHandle);

/* Sets the mode of the speed and torque controller (Torque mode or Speed mode) */
void STC_SetControlMode(SpeednTorqCtrl_Handle_t *pHandle, MC_ControlMode_t bMode);

/* Starts the execution of a ramp using new target and duration */
bool STC_ExecRamp(SpeednTorqCtrl_Handle_t *pHandle, int16_t hTargetFinal, uint32_t hDurationms);

/* Computes the new value of motor torque reference */
int16_t STC_CalcTorqueReference(SpeednTorqCtrl_Handle_t *pHandle);

/* Gets the Default mechanical rotor speed reference */
int16_t STC_GetMecSpeedRefUnitDefault(SpeednTorqCtrl_Handle_t *pHandle);

/* Returns the Application maximum positive rotor mechanical speed */
uint16_t STC_GetMaxAppPositiveMecSpeedUnit(SpeednTorqCtrl_Handle_t *pHandle);

/* Returns the Application minimum negative rotor mechanical speed */
int16_t STC_GetMinAppNegativeMecSpeedUnit(SpeednTorqCtrl_Handle_t *pHandle);

/* Stops the execution of speed ramp */
bool STC_StopSpeedRamp(SpeednTorqCtrl_Handle_t *pHandle);

/* Sets in real time the speed sensor utilized by the FOC */
void STC_SetSpeedSensor(SpeednTorqCtrl_Handle_t *pHandle, SpeednPosFdbk_Handle_t *SPD_Handle);

/* It returns the default values of Iqdref */
qd_t STC_GetDefaultIqdref(SpeednTorqCtrl_Handle_t *pHandle);

/* Forces the speed reference to the current speed */
void STC_ForceSpeedReferenceToCurrentSpeed(SpeednTorqCtrl_Handle_t *pHandle);

/**
  * @brief  Gets the modality of the speed and torque controller
  *           @ref SpeednTorqCtrl_Handle_t::Mode "Mode".
  * @param  pHandle: handler of the current instance of the SpeednTorqCtrl component.
  * @retval MC_ControlMode_t  modality of STC. It can be one of
  *         these two values: MCM_TORQUE_MODE or MCM_SPEED_MODE.
  *
  * - Called by @ref SpeedRegulatorPotentiometer Speed potentiometer component to manage new speed reference.
  */
static inline MC_ControlMode_t STC_GetControlMode(SpeednTorqCtrl_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_SPD_TRQ_CTL
  return ((MC_NULL == pHandle) ? MCM_TORQUE_MODE : pHandle->Mode);
#else
  return (pHandle->Mode);
#endif
}

/**
  * @brief  Interrupts the execution of any previous ramp command in particular by clearing
  *         the number of steps remaining to complete the ramp
  *         @ref SpeednTorqCtrl_Handle_t::RampRemainingStep "RampRemainingStep".
  * @param  pHandle: handler of the current instance of the SpeednTorqCtrl component.
  * @retval none
  *
  * - If STC has been set in Torque mode the last value of Iq is maintained.\n
  * - If STC has been set in Speed mode the last value of mechanical
  * rotor speed reference is maintained.
  * - Called by MCI_StopSpeedRamp execution command.
  */
static inline void STC_StopRamp(SpeednTorqCtrl_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_SPD_TRQ_CTL
  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
#endif
    pHandle->RampRemainingStep = 0U;
    pHandle->IncDecAmount = 0;
#ifdef NULL_PTR_CHECK_SPD_TRQ_CTL
  }
#endif
}

/**
  * @brief  Checks if the settled speed or torque ramp has been completed by checking zero value of
  *           @ref SpeednTorqCtrl_Handle_t::RampRemainingStep "RampRemainingStep".
  * @param  pHandle: handler of the current instance of the SpeednTorqCtrl component.
  * @retval bool returning true if the ramp is completed, false otherwise.
  *
  * - Called during motor profiler tuning of HALL sensor.
  */
static inline bool STC_RampCompleted(SpeednTorqCtrl_Handle_t *pHandle)
{
  bool retVal = false;
#ifdef NULL_PTR_CHECK_SPD_TRQ_CTL
  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
#endif
    if (0U == pHandle->RampRemainingStep)
    {
      retVal = true;
    }
#ifdef NULL_PTR_CHECK_SPD_TRQ_CTL
  }
#endif
  return (retVal);
}

/**
  * @brief  Changes the nominal current by setting new values of
  *         @ref SpeednTorqCtrl_Handle_t::MaxPositiveTorque "MaxPositiveTorque" and
  *         @ref SpeednTorqCtrl_Handle_t::MinNegativeTorque "MinNegativeTorque".
  * @param  pHandle: handler of the current instance of the SpeednTorqCtrl component.
  * @param  hNominalCurrent: represents actually the maximum Iq current expressed in digit.
  * @retval none
  *
  * - Not used into current implementation.
  */
static inline void STC_SetNominalCurrent(SpeednTorqCtrl_Handle_t *pHandle, uint16_t hNominalCurrent)
{
#ifdef NULL_PTR_CHECK_SPD_TRQ_CTL
  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
#endif
    pHandle->MaxPositiveTorque = hNominalCurrent;
    pHandle->MinNegativeTorque = -(int16_t)hNominalCurrent;
#ifdef NULL_PTR_CHECK_SPD_TRQ_CTL
  }
#endif
}

/**
  * @brief Returns the speed sensor utilized by the FOC.
  * @param pHandle: handler of the current instance of the SpeednTorqCtrl component.
  * @retval SpeednPosFdbk_Handle_t speed sensor utilized by the FOC.
  *
  * - Called as soon as component parameters are required by MC FW.
  */
static inline SpeednPosFdbk_Handle_t *STC_GetSpeedSensor(SpeednTorqCtrl_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_SPD_TRQ_CTL
  return ((MC_NULL ==  pHandle) ? MC_NULL : pHandle->SPD);
#else
  return (pHandle->SPD);
#endif
}

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

#endif /* SPEEDNTORQCTRLCLASS_H */

/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/

