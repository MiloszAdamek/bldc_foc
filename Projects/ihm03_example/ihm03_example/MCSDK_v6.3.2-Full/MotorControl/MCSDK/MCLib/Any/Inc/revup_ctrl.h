/**
  ******************************************************************************
  * @file    revup_ctrl.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file contains all definitions and functions prototypes for the
  *          RevUpCtrl component of the Motor Control SDK.
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
  * @ingroup RevUpCtrlFOC
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef REVUP_CTRL_H
#define REVUP_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "mc_type.h"
#include "speed_torq_ctrl.h"
#include "virtual_speed_sensor.h"
#include "sto_speed_pos_fdbk.h"
#include "pwm_curr_fdbk.h"

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup RevUpCtrl
  * @{
  */

/** @addtogroup RevUpCtrlFOC
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/**
  * @brief Maximum number of phases allowed for RevUp process.
  *
  */
#define RUC_MAX_PHASE_NUMBER 5u

/**
  * @brief RevUpCtrl_PhaseParams_t structure used for phases definition
  *
  */
typedef struct
{
  uint16_t hDurationms;       /**< @brief Duration of the RevUp phase. This parameter is expressed in millisecond.*/
  int16_t hFinalMecSpeedUnit; /**< @brief Mechanical speed assumed by VSS at the end of the RevUp phase. Expressed in
                                   the unit defined by #SPEED_UNIT */
  int16_t hFinalTorque;       /**< @brief Motor torque reference imposed by STC at the end of RevUp phase. This value
                                   represents actually the Iq current expressed in digit.*/
  void *pNext;                /**< @brief Pointer on the next phase section to proceed
                                   This parameter is NULL for the last element. */
} RevUpCtrl_PhaseParams_t;

/**
  * @brief  Handle structure of the RevUpCtrl.
  *
  */

typedef struct
{
  uint16_t hRUCFrequencyHz;                                 /**< @brief Frequency call to main RevUp procedure RUC_Exec.
                                                                 This parameter is equal to speed loop frequency. */

  int16_t hStartingMecAngle;                                /**< @brief Starting angle of programmed RevUp.*/

  uint16_t hPhaseRemainingTicks;                            /**< @brief Number of clock events remaining to complete the
                                                                 phase. */

  int16_t hDirection;                                       /**< @brief Motor direction.
                                                                 This parameter can be any value -1 or +1 */

  RevUpCtrl_PhaseParams_t *pCurrentPhaseParams;             /**< @brief Pointer on the current RevUp phase processed. */

  RevUpCtrl_PhaseParams_t ParamsData[RUC_MAX_PHASE_NUMBER]; /**< @brief Start up Phases sequences used by RevUp
                                                                 controller. Up to five phases can be used for the start
                                                                  up. */

  uint8_t bPhaseNbr;                                        /**< @brief Number of phases relative to the programmed
                                                                 RevUp sequence. This parameter can be any value from 1
                                                                 to 5 */

  uint8_t bFirstAccelerationStage;                          /**< @brief Indicates the phase to start the final
                                                                 acceleration.
                                                                 At start of this stage sensor-less algorithm cleared.*/
  uint16_t hMinStartUpValidSpeed;                           /**< @brief Minimum rotor speed required to validate the
                                                                 startup.
                                                                 This parameter is expressed in SPPED_UNIT */
  uint16_t hMinStartUpFlySpeed;                             /**< @brief Minimum rotor speed required to validate the on
                                                                 the fly. This parameter is expressed in the unit
                                                                 defined by #SPEED_UNIT */
  int16_t hOTFFinalRevUpCurrent;                            /**< @brief Final targetted torque for OTF phase. */

  uint16_t hOTFSection1Duration;                            /**< @brief On-the-fly phase duration, millisecond.
                                                                 This parameter is expressed in millisecond.*/
  bool OTFStartupEnabled;                                   /**< @brief Flag for OTF feature activation.
                                                                 Feature disabled when set to false */
  uint8_t bOTFRelCounter;                                   /**< @brief Counts the number of reliability of state
                                                                 observer */

  bool OTFSCLowside;                                        /**< @brief Flag to indicate status of low side switchs.
                                                                 This parameter can be true when Low Sides switch is ON
                                                                 otherwise set to false. */
  bool EnteredZone1;                                        /**< @brief Flag to indicate that the minimum rotor speed
                                                                 has been reached. */

  uint8_t bResetPLLTh;                                      /**< @brief Threshold to reset PLL during OTF */

  uint8_t bResetPLLCnt;                                     /**< @brief Counter to reset PLL during OTF when the
                                                                 threshold is reached. */

  uint8_t bStageCnt;                                        /**< @brief Counter of executed phases.
                                                                 This parameter can be any value from 0 to 5 */

  RevUpCtrl_PhaseParams_t OTFPhaseParams;                   /**< @brief RevUp phase parameter of OTF feature.*/

  SpeednTorqCtrl_Handle_t *pSTC;                            /**< @brief Speed and torque controller object used by
                                                                 RevUpCtrl.*/

  VirtualSpeedSensor_Handle_t *pVSS;                        /**< @brief Virtual speed sensor object used by RevUpCtrl.*/

  STO_Handle_t *pSNSL;                                      /**< @brief STO sensor object used by OTF startup.*/

  PWMC_Handle_t *pPWM;                                      /**< @brief PWM object used by OTF startup.*/

} RevUpCtrl_Handle_t;


/* Exported functions ------------------------------------------------------- */

/*  Initializes and configures the RevUpCtrl Component */
void RUC_Init(RevUpCtrl_Handle_t *pHandle, SpeednTorqCtrl_Handle_t *pSTC, VirtualSpeedSensor_Handle_t *pVSS,
              STO_Handle_t *pSNSL, PWMC_Handle_t *pPWM);

/* Initializes the internal RevUp controller state */
void RUC_Clear(RevUpCtrl_Handle_t *pHandle, int16_t hMotorDirection);

/* Main Rev-Up controller procedure executing overall programmed phases */
bool RUC_Exec(RevUpCtrl_Handle_t *pHandle);

/* Main Rev-Up controller procedure that executes overall programmed phases and on-the-fly startup handling */
bool RUC_OTF_Exec(RevUpCtrl_Handle_t *pHandle);

/* Checks that alignment and first acceleration stage are completed */
bool RUC_FirstAccelerationStageReached(RevUpCtrl_Handle_t *pHandle);

/* Allows to modify duration (ms unit) of a selected phase */
void RUC_SetPhaseDurationms(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase, uint16_t hDurationms);

/* Allows to modify targeted mechanical speed of a selected phase */
void RUC_SetPhaseFinalMecSpeedUnit(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase, int16_t hFinalMecSpeedUnit);

/* Allows to modify targeted the motor torque of a selected phase */
void RUC_SetPhaseFinalTorque(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase, int16_t hFinalTorque);

/* Allows to read duration set in selected phase */
uint16_t RUC_GetPhaseDurationms(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase);

/* Allows to read targeted Rotor speed set in selected phase */
int16_t RUC_GetPhaseFinalMecSpeedUnit(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase);

/* Allows to read targeted motor torque set in selected phase */
int16_t RUC_GetPhaseFinalTorque(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase);

/* @brief  It checks if this stage is used for align motor. */
uint8_t RUC_IsAlignStageNow(RevUpCtrl_Handle_t *pHandle);

/**
  * @brief  Allow to exit from Rev-Up process at the current rotor speed.
  * @param  pHandle: Pointer on Handle structure of RevUp controller.
  */
static inline void RUC_Stop(RevUpCtrl_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
#endif
    VirtualSpeedSensor_Handle_t *pVSS = pHandle->pVSS;
    pHandle->pCurrentPhaseParams = MC_NULL;
    pHandle->hPhaseRemainingTicks = 0U;
    VSS_SetMecAcceleration(pVSS, SPD_GetAvrgMecSpeedUnit(&pVSS->_Super), 0U);
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  }
#endif
}

/**
  * @brief  Provide current state of Rev-Up controller procedure.
  * @param  pHandle: Pointer on Handle structure of RevUp controller.
  *  @retval Boolean set to true when entire Rev-Up phases have been completed.
  */
static inline bool RUC_Completed(RevUpCtrl_Handle_t *pHandle)
{
  bool retVal = false;
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
#endif
    if (MC_NULL == pHandle->pCurrentPhaseParams)
    {
      retVal = true;
    }
    else
    {
      /* Nothing to do */
    }
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  }
#endif
  return (retVal);
}

/**
  * @brief  Allow to configure a Rev-Up phase.
  * @param  pHandle: Pointer on Handle structure of Rev-Up controller.
  * @param  phaseNumber : number of phases relative to the programmed Rev-Up
  * @param  phaseData:
  *       - RevUp phase where new motor torque shall be modified.
  *              This parameter must be a number between 0 and 6.
  *            - RevUp phase where new mechanical speed shall be modified.
  *             This parameter must be a number between 0 and 6.
  *            - New duration value required for associated phase in ms unit.
  *  @retval Boolean set to true
  */
static inline bool RUC_SetPhase(RevUpCtrl_Handle_t *pHandle, uint8_t phaseNumber, RevUpCtrl_PhaseParams_t *phaseData)
{
  bool retValue = true;
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  if ((MC_NULL == pHandle) || (MC_NULL == phaseData))
  {
    retValue = false;
  }
  else
  {
#endif
    pHandle->ParamsData[phaseNumber].hFinalTorque = phaseData->hFinalTorque;
    pHandle->ParamsData[phaseNumber].hFinalMecSpeedUnit = phaseData->hFinalMecSpeedUnit;
    pHandle->ParamsData[phaseNumber].hDurationms = phaseData->hDurationms;
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  }
#endif
  return (retValue);
}

/**
  * @brief  Allow to read total number of programmed phases.
  * @param  pHandle: Pointer on Handle structure of RevUp controller.
  *  @retval Returns number of phases relative to the programmed Rev-Up.
  *
  */

static inline uint8_t RUC_GetNumberOfPhases(RevUpCtrl_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  return ((MC_NULL == pHandle) ? 0U : (uint8_t)pHandle->bPhaseNbr);
#else
  return ((uint8_t)pHandle->bPhaseNbr);
#endif
}

/**
  * @brief  Allow to read a programmed phase.
  * @param  pHandle: Pointer on Handle structure of RevUp controller.
  * @param  phaseNumber : number of phases relative to the programmed Rev-Up
  * @param  phaseData:
  *       - RevUp phase from where targeted rotor speed is read.
  *              This parameter must be a number between 0 and 6.
  *            - RevUp phase from where targeted motor torque is read.
  *             This parameter must be a number between 0 and 6.
  *            - Duration set in selected phase in ms unit.
  *  @retval Returns Boolean set to true value.
  */
static inline bool RUC_GetPhase(RevUpCtrl_Handle_t *pHandle, uint8_t phaseNumber, RevUpCtrl_PhaseParams_t *phaseData)
{
  bool retValue = true;
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  if ((MC_NULL == pHandle) || (MC_NULL == phaseData))
  {
    retValue = false;
  }
  else
  {
#endif
    phaseData->hFinalTorque = (int16_t)pHandle->ParamsData[phaseNumber].hFinalTorque;
    phaseData->hFinalMecSpeedUnit = (int16_t)pHandle->ParamsData[phaseNumber].hFinalMecSpeedUnit;
    phaseData->hDurationms = (uint16_t)pHandle->ParamsData[phaseNumber].hDurationms;
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  }
#endif
  return (retValue);
}

/**
  * @brief  Allow to read status of On The Fly (OTF) feature.
  * @param  pHandle: Pointer on Handle structure of RevUp controller.
  *  @retval Boolean set to true at the end of OTF processing.
  */
static inline bool RUC_Get_SCLowsideOTF_Status(RevUpCtrl_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_REV_UP_CTL
  return ((MC_NULL == pHandle) ? false : pHandle->OTFSCLowside);
#else
  return (pHandle->OTFSCLowside);
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

#endif /* REVUP_CTRL_H */

/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/
