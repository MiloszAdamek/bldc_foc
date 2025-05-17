/**
  ******************************************************************************
  * @file    open_loop_sixstep.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file provides firmware functions that implement the
  *  openloop sixstep Control component of the Motor Control SDK.
  *
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

/* Includes ------------------------------------------------------------------*/
#include "pwmc_sixstep.h"
#include "open_loop_sixstep.h"
#include "regular_conversion_manager.h"
#include "mc_config_common.h"
#include "mc_type.h"

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup SixStep
  * @{
  */


/**
 * @defgroup OpenLoop_SixStep OpenLoop management
 * @brief OpenLoop component of the 6-step Motor Control SDK.
 *
 * The OpenLoop feature of the 6-step MCSDK is a debug feature that allows the user to run the algorithm without a speed
 * control usually performed in close loop with a PI control.
 *
 * In case of voltage driving mode, the duty cycle of the PWM Timer that controls the  3 phases, is managed directly by
 * the user interface.
 *
 * In case of current mode, this is duty cycle of the current reference used to control the speed that is managed directly.
 *
 * A mean value of the duty cycle is computed on a buffer containing a maximum of 12 stored values.
 *
 * The periodicity of the duty cycle computation is the same than the speed control in close loop (medium frequency task).
 *
 * The OpenLoop feature can start with or without a RevUp sequence.
 *
 * The OpenLoop feature can also run only with the BEMF zero crossing detection set in PWM ON sensing.
 *
 *
 * @{
 */

/* Local Functions */
/* It is used to compute the mean value of duty Cycle reference in Current or Voltage mode. */
static uint16_t OLS_CalcOpenLoopDutyCycle(uint32_t HTargetCntPh, OpenLoopSixstepCtrl_Handle_t *pHandle,  MC_ControlMode_t Mode);

/**
 * @brief  Initializes all the object variables, usually it has to be called
 *         once right after object creation.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval none.
 */

__weak void OLS_Init(OpenLoopSixstepCtrl_Handle_t *pHandle)
{
  pHandle->DutyCycleRefMean = 0U;
  pHandle->DutyCycleRefCounter = pHandle->DutyCycleRefTableSize;
  uint8_t i;
  for (i = 0U; i < pHandle->DutyCycleRefTableSize; i++)
  {
    pHandle->DutyCycleRefTable[i] = 0U;
  }
}

/**
 * @brief  Returns the openloop status flag.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval bool: openloop status flag to set, true = enable  or false=disable.
 */
__weak bool OLS_GetOpenLoopFlag(OpenLoopSixstepCtrl_Handle_t *pHandle)
{
  return ((MC_NULL == pHandle) ? 0U : pHandle->Openloop);
}

/**
 * @brief  It sets the openloop status flag in real time.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @param  flag: openloop status flag to set, true = enable  or false=disable.
 * @retval none.
 */
__weak void OLS_SetOpenLoopFlag(OpenLoopSixstepCtrl_Handle_t *pHandle, bool Flag)
{
  if ( MC_NULL == pHandle )
  {
    /* Nothing to do */
  }
  else
  {
    pHandle->Openloop = Flag;
  }
}

/**
 * @brief  It sets the Duty cycle reference table size in real time.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @param  DutyCycleRefTableSize: openloop status flag to set, true = enabled  or false = disabled.
 * @retval none.
 */

__weak void OLS_SetDutyCycleRefTableSize(OpenLoopSixstepCtrl_Handle_t *pHandle, uint8_t DutyCycleRefTableSize)
{
  if ( MC_NULL == pHandle )
  {
    /* Nothing to do */
  }
  else
  {
    pHandle->DutyCycleRefTableSize = DutyCycleRefTableSize;
  }
}

/**
 * @brief  Returns the Duty cycle reference table size to compute a mean of the requested speed.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component
 * @retval uint8_t: openloop Duty cycle reference table size [1..12].
 */
__weak uint8_t OLS_GetDutyCycleRefTableSize(OpenLoopSixstepCtrl_Handle_t *pHandle)
{
  return ((MC_NULL == pHandle) ? 0U : pHandle->DutyCycleRefTableSize);
}

/**
 * @brief  It sets the Duty cycle reference.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval none.
 */
__weak void OLS_SetDutyCycleRef(OpenLoopSixstepCtrl_Handle_t *pHandle, uint8_t DutyCycleRef)
{
  if ( MC_NULL == pHandle )
  {
  }
  else
  {
    pHandle->DutyCycleRef = DutyCycleRef;
  }
}

/**
 * @brief  Returns the Duty cycle reference value.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval uint8_t: DutyCycleRef Duty cycle reference value in percentage unit.
 */
__weak uint8_t OLS_GetDutyCycleRef(OpenLoopSixstepCtrl_Handle_t *pHandle)
{
  return ((MC_NULL == pHandle) ? 0U : pHandle->DutyCycleRef);
}

/**
 * @brief  It sets the Rev-up phase enable flag.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval none.
 */
__weak void OLS_SetRevUpFlag(OpenLoopSixstepCtrl_Handle_t *pHandle, bool Flag)
{
  if ( MC_NULL == pHandle )
  {
    /* Nothing to do */
  }
  else
  {
    pHandle->RevUp = Flag;
  }
}

/**
 * @brief  Returns the Rev-up phase enable flag.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval bool: RevUp enable flag true = enabled, false = disabled.
 */
__weak bool OLS_GetRevUpFlag(OpenLoopSixstepCtrl_Handle_t *pHandle)
{
  return ((MC_NULL == pHandle) ? 0U : pHandle->RevUp);
}

/**
 * @brief  It is used to set the new value of duty cycle reference coming from the potentiometer (ADC).
 *         It must be called at fixed time equal to hSTCFrequencyHz. It is called
 *         passing as parameter the aw value comming from ADC IP..
 * @param  pHandle: handler of the current instance of the SpeednTorqCtrl and component.
 * @param  RawValue: ADC value coming from the potentiometer.
 * @retval none.
 */
void OLS_Potentiometer_Run(OpenLoopSixstepCtrl_Handle_t *pHandle, uint16_t RawValue)
{
  uint32_t dutyCycleRef;
  dutyCycleRef = (100U * RawValue) / 65535U; /* Convert from 16bits to percentage value */
  dutyCycleRef = ((dutyCycleRef < 2U) ? 2U : dutyCycleRef); /* Lower value limitation */
  dutyCycleRef = ((dutyCycleRef > 98U) ? 98U : dutyCycleRef); /* higher value limitation */
  OLS_SetDutyCycleRef(pHandle, (uint8_t)dutyCycleRef); /* Set the dutyCycleRef */
}

/**
 * @brief  It is used to compute the new value of motor PWM duty Cycle reference in openLoop Voltage mode.
 *         It must be called at fixed time equal to hSTCFrequencyHz. It is called
 *         passing as parameter the PWM period, open loop parameters used to set up the duty cycle and the
 *         MCM_SPEED_MODE.
 * @param  PWMperiod: PWM Period counter value.
 * @param  pHandle: handler of the current instance of the SpeednTorqCtrl and component.
 * @param  Mode: MCM_SPEED_MODE required for Openloop feature.
 * @retval int16_t: Motor duty cycle reference. This value represents actually the
 *         dutycycle expressed in digit for PWM counter.
 */
__weak uint16_t OLS_CalcOpenLoopDutyCycleVM(uint16_t PWMperiod, OpenLoopSixstepCtrl_Handle_t *pHandle, MC_ControlMode_t Mode)
{
  uint32_t hTargetCntPh = 0U;

  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
    if (MCM_SPEED_MODE == Mode)
    {
      uint32_t highLimit = (pHandle->VoltageFactor * (uint32_t)PWMperiod) / 100U; /* Compute high limitation in PWM counter value */
      hTargetCntPh = ((uint32_t)pHandle->DutyCycleRef * (uint32_t)highLimit) / 100U; /* Convert from percentage to PWM counter value. */
      hTargetCntPh = OLS_CalcOpenLoopDutyCycle(hTargetCntPh, pHandle, Mode); /* Compute mean duty cyle value */
    }
    else
    {
      /* Nothing to do */
    }
  }
  return ((uint16_t)hTargetCntPh);
}


/**
 * @brief  It is used to compute the new value of PWM current duty Cycle reference in current mode.
 *         It must be called at fixed time equal to hSTCFrequencyHz. It is called
 *         passing as parameter the PWM period, the open loop parameters used to set up the duty cycle and the
 *         MCM_SPEED_MODE.
 * @param  PWMperiod: PWM Period counter value of current refernce.
 * @param  pHandle: handler of the current instance of the SpeednTorqCtrl and component.
 * @param  Mode: MCM_SPEED_MODE required for Openloop feature.
 * @retval int16_t:  PWM curretn dutycycle reference. This value represents actually the
 *         dutycycle expressed in digit for PWM counter.
 */
__weak uint16_t OLS_CalcOpenLoopDutyCycleCM(uint16_t PWMperiod, OpenLoopSixstepCtrl_Handle_t *pHandle, MC_ControlMode_t Mode)
{
  uint32_t hTargetCntPh = 0U;

  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
    if (MCM_SPEED_MODE == Mode)
    {
      uint32_t highLimit = (pHandle->CurrentFactor * (uint32_t)PWMperiod) / 100U; /* Compute high limitation in PWM counter value */
      hTargetCntPh = ((uint32_t)pHandle->DutyCycleRef * (uint32_t)highLimit) / 100U;  /* Convert from percentage to CM counter value. */
      hTargetCntPh = OLS_CalcOpenLoopDutyCycle(hTargetCntPh, pHandle, Mode); /* Compute mean duty cyle value */
    }
    else
    {
      /* Nothing to do */
    }
  }
  return ((uint16_t)hTargetCntPh);
}

/**
 * @brief  It is used to compute the mean value of duty Cycle reference in Current or Voltage mode.
 *         It must be called at fixed time equal to hSTCFrequencyHz. It is called
 *         passing as parameter the current HTargetCntPh, the open loop parameters and the
 *         MCM_SPEED_MODE.
 * @param  HTargetCntPh: PWM Period counter value of current refernce.
 * @param  pHandle: handler of the current instance of the SpeednTorqCtrl and component.
 * @param  Mode: MCM_SPEED_MODE required for Openloop feature.
 * @retval int16_t motor dutycycle reference. This value represents actually the
 *         mean dutycycle expressed in digit for PWM counter.
 */
uint16_t OLS_CalcOpenLoopDutyCycle(uint32_t HTargetCntPh, OpenLoopSixstepCtrl_Handle_t *pHandle,  MC_ControlMode_t Mode)
{
  uint32_t hTargetCntPh = HTargetCntPh;

  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
    if (MCM_SPEED_MODE == Mode)
    {
      pHandle->DutyCycleRefTable[pHandle->DutyCycleRefTableIndex] = hTargetCntPh; /* Fill the table */
      pHandle->DutyCycleRefTableIndex++; /* Increment table pointer */
      /* Wrap table index. */
      pHandle->DutyCycleRefTableIndex =
              ((pHandle->DutyCycleRefTableSize == pHandle->DutyCycleRefTableIndex) ?
                      0U : pHandle->DutyCycleRefTableIndex);
      pHandle->DutyCycleRefMean = 0U; /* Clear accumulator mean value */

      if (1U < pHandle->DutyCycleRefCounter) /* Number of samples in the table is OK. */
      {
        pHandle->DutyCycleRefCounter--;
      }
      else
      {
        /* Wait the good number of samples into the table.*/
      }

      uint8_t i;
      for (i = 0U; i < pHandle->DutyCycleRefTableSize; i++)
      {
        pHandle->DutyCycleRefMean += pHandle->DutyCycleRefTable[i];
      }

      if (1U == pHandle->DutyCycleRefCounter) /* Perform mean computation of the table. */
      {
        pHandle->DutyCycleRefMean = (pHandle->DutyCycleRefMean / (uint32_t)pHandle->DutyCycleRefTableSize);
        hTargetCntPh = pHandle->DutyCycleRefMean;
      }
      else
      {
        /* Wait the good number of samples into the table.*/
      }
    }
    else
    {
      /* Nothing to do */
    }
  }
  return ((uint16_t)hTargetCntPh);
}

/**
 * @brief  It sets Current factor in percentage unit.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @param  uint16_t: Current factor in percentage unit.
 * @retval none.
 */
__weak void OLS_SetCurrentFactor(OpenLoopSixstepCtrl_Handle_t *pHandle, uint16_t CurrentFactor)
{
  if ( MC_NULL == pHandle )
  {
  }
  else
  {
    pHandle->CurrentFactor = CurrentFactor;
  }
}

/**
 * @brief  Returns Current factor in percentage unit.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval int16_t: Current factor in percentage unit.
 */
__weak uint16_t OLS_GetCurrentFactor(OpenLoopSixstepCtrl_Handle_t *pHandle)
{
  return ((MC_NULL == pHandle) ? 0U : pHandle->CurrentFactor);
}

/**
 * @brief  It sets Voltage factor in percentage unit.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @param int16_t: Voltage factor in percentage unit.
 * @retval none.
 */
__weak void OLS_SetVoltageFactor(OpenLoopSixstepCtrl_Handle_t *pHandle, uint8_t VoltageFactor)
{
  if ( MC_NULL == pHandle )
  {
  }
  else
  {
    pHandle->VoltageFactor = VoltageFactor;
  }
}

/**
 * @brief  Returns Voltage Factor value for max PWM duty cycle.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval uint8_t: Voltage factor in percentage unit.
 */
__weak uint8_t OLS_GetVoltageFactor(OpenLoopSixstepCtrl_Handle_t *pHandle)
{
  return ((MC_NULL == pHandle) ? 0U : pHandle->VoltageFactor);
}


/**
 * @brief  It sets openloop BEMF Sensing ON command.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @param  bool: BEMF Sensing ON, true = enabled , false = disabled.
 * @retval none.
 */
__weak void OLS_SetOnSensing(OpenLoopSixstepCtrl_Handle_t *pHandle, bool OnSensing)
{
  if ( MC_NULL == pHandle )
  {
  }
  else
  {
    pHandle->OnSensing = OnSensing;
  }
}

/**
 * @brief  Returns the openloop BEMF Sensing ON command.
 * @param  pHandle: handler of the current instance of the OpenloopSixstepCtrl component.
 * @retval bool: BEMF Sensing ON, true = enabled , false = disabled.
 */
__weak bool OLS_GetOnSensing(OpenLoopSixstepCtrl_Handle_t *pHandle)
{
  return ((MC_NULL == pHandle) ? 0U : pHandle->OnSensing);
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

/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/
