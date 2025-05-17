/**
  ******************************************************************************
  * @file    pwmc_sixstep.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file provides the firmware functions that implement the 
  *          pwmc_sixstep component of the Motor Control SDK.
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
  * @ingroup PWMC_SixStep
  */

/* Includes ------------------------------------------------------------------*/
#include "pwmc_sixstep.h"
#include "mc_type.h"


/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup SixStep
  * @{
  */

/**
 * @defgroup PWMC_SixStep PWM management
 *
 * @brief PWM generation implementation for Six-Step drive
 *
 * This implementation drives the bridges with 3 signals and 
 * their complementary ones or 3 enables. 
 *
 * @todo: Complete documentation.
 * @{
 */

/* Private defines -----------------------------------------------------------*/
#define TIMxCCER_MASK_CH123        ((uint16_t)(LL_TIM_CHANNEL_CH1|LL_TIM_CHANNEL_CH1N|\
                                               LL_TIM_CHANNEL_CH2|LL_TIM_CHANNEL_CH2N|\
                                               LL_TIM_CHANNEL_CH3|LL_TIM_CHANNEL_CH3N))

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  It initializes TIMx and NVIC
  * @param  pHdl: handler of the current instance of the PWM component
  * @retval none
  */
__weak void PWMC_Init( PWMC_Handle_t * pHandle )
{
  /* Clear TIMx break flag. */
  LL_TIM_ClearFlag_BRK( pHandle->pParams_str->TIMx );
  LL_TIM_EnableIT_BRK( pHandle->pParams_str->TIMx );
  LL_TIM_ClearFlag_UPDATE( pHandle->pParams_str->TIMx );
    
  /* disable ADC source trigger */
  LL_TIM_SetTriggerOutput(pHandle->pParams_str->TIMx, LL_TIM_TRGO_RESET);

  /* Enable PWM channel */
  LL_TIM_CC_EnableChannel( pHandle->pParams_str->TIMx, TIMxCCER_MASK_CH123 );

  /* Clear the flags */
  pHandle->OverVoltageFlag = false;
  pHandle->OverCurrentFlag = false;
  pHandle->driverProtectionFlag = false;

  pHandle->hElAngle = 0;
  LL_TIM_EnableCounter( pHandle->pParams_str->TIMx );
  pHandle->pCCMR1_cfg = &(pHandle->TimerCfg->CCMR1_Standard_cfg[0]);
  pHandle->pCCMR2_cfg = &(pHandle->TimerCfg->CCMR2_Standard_cfg[0]);
}

/**
* @brief  It updates the stored duty cycle variable.
* @param  pHandle Pointer on the target component instance.
* @param  new duty cycle value.
* @retval none
*/
__weak void PWMC_SetPhaseVoltage( PWMC_Handle_t * pHandle, uint16_t DutyCycle )
{
  pHandle->CntPh = DutyCycle;
  LL_TIM_WriteReg(TIM1, CCR1, (uint32_t) DutyCycle);
  LL_TIM_WriteReg(TIM1, CCR2, (uint32_t) DutyCycle);
  LL_TIM_WriteReg(TIM1, CCR3, (uint32_t) DutyCycle);  
}

/**
* @brief  It writes the duty cycle into timer shadow registers.
* @param  pHandle Pointer on the target component instance.
* @retval none
*/
__weak void PWMC_LoadNextStep( PWMC_Handle_t * pHandle )
{
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;
  if (pHandle->AlignFlag != 0)
  {
    pHandle->pCCER_cfg = &(pHandle->TimerCfg->CCER_Align_cfg);
    if (pHandle->AlignFlag == -1)
    {
      pHandle->pCCMR1_cfg = &(pHandle->TimerCfg->CCMR1_CCW_Align_cfg[0]);
      pHandle->pCCMR2_cfg = &(pHandle->TimerCfg->CCMR2_CCW_Align_cfg[0]);
    }
    else
    {
      pHandle->pCCMR1_cfg = &(pHandle->TimerCfg->CCMR1_CW_Align_cfg[0]);
      pHandle->pCCMR2_cfg = &(pHandle->TimerCfg->CCMR2_CW_Align_cfg[0]);
    }
    LL_TIM_WriteReg(TIMx, CCER, *(pHandle->pCCER_cfg));
    if ((ES_GPIO == pHandle->LowSideOutputs))
    {
      LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
      LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
      LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
    }
  }
  else
  {
    if (pHandle->QuasiSynchDecay == true)
    {
      pHandle->pCCER_cfg = &(pHandle->TimerCfg->CCER_QuasiSynch_cfg[0]);
      LL_TIM_WriteReg(TIMx, CCER, *(pHandle->pCCER_cfg + pHandle->NextStep));
    }
    else 
    {
      pHandle->pCCER_cfg = &(pHandle->TimerCfg->CCER_cfg[0]);
      if (pHandle->LSModArray[pHandle->NextStep]==1)
      {
        pHandle->pCCMR1_cfg = &(pHandle->TimerCfg->CCMR1_LSMod_cfg[0]);
        pHandle->pCCMR2_cfg = &(pHandle->TimerCfg->CCMR2_LSMod_cfg[0]);
      }
      else
      {
        pHandle->pCCMR1_cfg = &(pHandle->TimerCfg->CCMR1_Standard_cfg[0]);
        pHandle->pCCMR2_cfg = &(pHandle->TimerCfg->CCMR2_Standard_cfg[0]);
      }
    }
    if ((ES_GPIO == pHandle->LowSideOutputs))
    {
      switch ( pHandle->NextStep )
      {
      case STEP_1:
      case STEP_4:
        {
          LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
          LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
          LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        }
        break;
      case STEP_2:
      case STEP_5:
        {
          LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
          LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
          LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
        }
        break;
      case STEP_3:
      case STEP_6:
        {
          LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
          LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
          LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
        }
        break;
      }
    }
    LL_TIM_WriteReg(TIMx, CCER, *(pHandle->pCCER_cfg + pHandle->NextStep));
  }
  LL_TIM_WriteReg(TIMx, CCMR1, *(pHandle->pCCMR1_cfg + pHandle->NextStep));
  LL_TIM_WriteReg(TIMx, CCMR2, *(pHandle->pCCMR2_cfg + pHandle->NextStep));  
  pHandle->Step = pHandle->NextStep;
  LL_TIM_GenerateEvent_COM( TIMx );
  LL_TIM_GenerateEvent_UPDATE( TIMx );
}

/**
  * @brief  It turns on low sides switches. This function is intended to be
  *         used for charging boot capacitors of driving section. It has to be
  *         called each motor start-up when using high voltage drivers
  * @param  pHdl: handler of the current instance of the PWM component
  * @retval none
  */
__weak void PWMC_TurnOnLowSides( PWMC_Handle_t * pHandle, uint32_t ticks)
{
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;

  pHandle->TurnOnLowSidesAction = true;
  
  /* Clear Update Flag */
  LL_TIM_ClearFlag_UPDATE( TIMx );

  /*Turn on the three low side switches */
  LL_TIM_OC_SetCompareCH1(TIMx, 0u);
  LL_TIM_OC_SetCompareCH2(TIMx, 0u);
  LL_TIM_OC_SetCompareCH3(TIMx, 0u);
  LL_TIM_WriteReg(TIMx, CCMR1, 0x4848);
  LL_TIM_WriteReg(TIMx, CCMR2, 0x6848);
  LL_TIM_WriteReg(TIMx, CCER, pHandle->TimerCfg->CCER_Align_cfg);
  LL_TIM_GenerateEvent_UPDATE( TIMx );  
  LL_TIM_GenerateEvent_COM( TIMx );
  /* Main PWM Output Enable */
  LL_TIM_EnableAllOutputs(TIMx);

  if ((ES_GPIO == pHandle->LowSideOutputs))
  {
    LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
    LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
    LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );  
  }
}

/**
  * @brief  This function enables the PWM outputs
  * @param  pHandle: handler of the current instance of the PWM component
  * @retval none
  */
__weak void PWMC_SwitchOnPWM( PWMC_Handle_t * pHandle )
{
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;

  pHandle->TurnOnLowSidesAction = false;
  pHandle->Step = STEP_INIT;
   
  LL_TIM_EnableIT_UPDATE( pHandle->pParams_str->TIMx );
	/* Select the Capture Compare preload feature */
  LL_TIM_CC_EnablePreload( TIMx );
	/* Select the Commutation event source */
  LL_TIM_CC_SetUpdate( TIMx, LL_TIM_CCUPDATESOURCE_COMG_ONLY );
  LL_TIM_OC_SetCompareCH1(TIMx, 0u);
  LL_TIM_OC_SetCompareCH2(TIMx, 0u);
  LL_TIM_OC_SetCompareCH3(TIMx, 0u);

  LL_TIM_GenerateEvent_COM( TIMx );
	/* Main PWM Output Enable */
  TIMx->BDTR |= LL_TIM_OSSI_ENABLE;
  LL_TIM_EnableAllOutputs(TIMx);

  LL_TIM_SetTriggerOutput(TIMx, LL_TIM_TRGO_OC4REF);	 
}

/**
  * @brief  This function sets the capcture compare of the timer channel 
  * used for ADC triggering
  * @param  pHandle: handler of the current instance of the PWM component
  * @param  SamplingPoint: trigger point
  * @retval none
  */
__weak void PWMC_SetADCTriggerChannel( PWMC_Handle_t * pHandle, uint16_t SamplingPoint )
{
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;

  pHandle->ADCTriggerCnt = SamplingPoint;
  LL_TIM_OC_SetCompareCH4(TIMx, pHandle->ADCTriggerCnt);
}

/**
  * @brief  It disables PWM generation on the proper Timer peripheral acting on
  *         MOE bit and reset the TIM status
  * @param  pHandle: handler of the current instance of the PWM component
  * @retval none
  */
__weak void PWMC_SwitchOffPWM( PWMC_Handle_t * pHandle )
{
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;

  pHandle->TurnOnLowSidesAction = false;

  if ((ES_GPIO == pHandle->LowSideOutputs))
  {
    LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
    LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
    LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
  }
  
  /* Main PWM Output Disable */
  LL_TIM_DisableAllOutputs(TIMx);
  LL_TIM_OC_SetCompareCH1(TIMx, 0u);
  LL_TIM_OC_SetCompareCH2(TIMx, 0u);
  LL_TIM_OC_SetCompareCH3(TIMx, 0u);
  pHandle->CntPh = 0;
  LL_TIM_DisableIT_UPDATE( pHandle->pParams_str->TIMx );
  LL_TIM_SetTriggerOutput(pHandle->pParams_str->TIMx, LL_TIM_TRGO_RESET);
  return;
}

/**
 * @brief  It contains the Break event interrupt
 * @param  pHandle: handler of the current instance of the PWM component
 * @retval none
 */
__weak void * PWMC_BRK_IRQHandler( PWMC_Handle_t * pHandle )
{
  pHandle->OverCurrentFlag = true;
  LL_TIM_DisableIT_UPDATE( pHandle->pParams_str->TIMx );
  LL_TIM_ClearFlag_UPDATE( pHandle->pParams_str->TIMx );
  return MC_NULL;
}

/**
  * @brief  This function disable the update of the demagnetization counter in the update event
  * @param  pHandle: handler of the current instance of the PWM component
  * @retval none
  */
__weak void PWMC_DisableHighFreqTask( PWMC_Handle_t * pHandle )
{
  LL_TIM_DisableIT_UPDATE(TIM1);
}

/**
* @brief  It is used to set the align motor flag.
* @param  this related object of class CPWMC
* @param  flag to be applied in uint8_t, 1: motor is in align stage, 2: motor is not in align stage
* @retval none
*/
void PWMC_SetAlignFlag(PWMC_Handle_t *pHandle, int16_t flag)
{
  pHandle->AlignFlag = flag;
}

/**
 * @brief Converts the motor electrical angle to the corresponding step in the six-step sequence
 * @param pHandle pointer on the handle structure of the PWMC instance
 * @retval calculated step
 */
bool  PWMC_ElAngleToStep( PWMC_Handle_t * pHandle )
{
  uint8_t Step;
  if ((pHandle->hElAngle >= (int16_t)( S16_60_PHASE_SHIFT / 2)) && (pHandle->hElAngle < (int16_t)( S16_60_PHASE_SHIFT + S16_60_PHASE_SHIFT / 2))) Step = STEP_1;
  else if ((pHandle->hElAngle >= (int16_t)( S16_60_PHASE_SHIFT + S16_60_PHASE_SHIFT / 2)) && (pHandle->hElAngle < (int16_t)( S16_120_PHASE_SHIFT + S16_60_PHASE_SHIFT / 2))) Step = STEP_2;
  else if ((pHandle->hElAngle >= (int16_t)( S16_120_PHASE_SHIFT + S16_60_PHASE_SHIFT / 2)) || (pHandle->hElAngle < (int16_t)( - S16_120_PHASE_SHIFT - S16_60_PHASE_SHIFT / 2))) Step = STEP_3;
  else if ((pHandle->hElAngle >= (int16_t)( - S16_120_PHASE_SHIFT - S16_60_PHASE_SHIFT / 2)) && (pHandle->hElAngle < (int16_t)( - S16_60_PHASE_SHIFT - S16_60_PHASE_SHIFT / 2))) Step = STEP_4;
  else if ((pHandle->hElAngle >= (int16_t)( - S16_60_PHASE_SHIFT - S16_60_PHASE_SHIFT / 2)) && (pHandle->hElAngle < (int16_t)( - S16_60_PHASE_SHIFT / 2))) Step = STEP_5;
  else if ((pHandle->hElAngle >= (int16_t)( - S16_60_PHASE_SHIFT / 2)) && (pHandle->hElAngle < (int16_t)( S16_60_PHASE_SHIFT / 2))) Step = STEP_6;
  else {}
  pHandle->NextStep = Step;
  if (pHandle->NextStep != pHandle->Step ) return true;
    else return false;
}

/**
* @brief Forces the next step in closed loop operation
* @param  pHandle Pointer on the target component instance.
* @param  Direction motor spinning direction.
* @retval none
 */
void  PWMC_ForceNextStep( PWMC_Handle_t * pHandle, int16_t Direction )
{
  if (Direction > 0)
  {
    pHandle->NextStep ++;
    if (pHandle->NextStep == 6) pHandle->NextStep = 0;
  }
  else
  {
    if (pHandle->NextStep == 0) pHandle->NextStep = 6;
    pHandle->NextStep--;
  }
}
    
/*
  * @brief  Checks if an overcurrent occurred since last call.
  *
  * @param  pHdl: Handler of the current instance of the PWM component.
  * @retval uint16_t Returns #MC_OVER_CURR if an overcurrent has been
  *                  detected since last method call, #MC_NO_FAULTS otherwise.
  */
uint16_t PWMC_IsFaultOccurred(PWMC_Handle_t *pHandle)
{
  uint16_t retVal = MC_NO_FAULTS;

  if (true == pHandle->OverVoltageFlag)
  {
    retVal = MC_OVER_VOLT;
    pHandle->OverVoltageFlag = false;
  }
  else
  {
    /* Nothing to do */
  }

  if (true == pHandle->OverCurrentFlag)
  {
    retVal |= MC_OVER_CURR;
    pHandle->OverCurrentFlag = false;
  }
  else
  {
    /* Nothing to do */
  }

  if (true == pHandle->driverProtectionFlag)
  {
    retVal |= MC_DP_FAULT;
    pHandle->driverProtectionFlag = false;
  }
  else
  {
    /* Nothing to do */
  }

  return (retVal);
}

uint8_t PWMC_GetLSModConfig(PWMC_Handle_t *pHandle)
{
  uint8_t retVal=0;
  for (uint8_t i=0; i<6; i++)
  {
    retVal |= pHandle->LSModArray[i] <<i;
  }
  return (retVal);
}

void PWMC_SetLSModConfig(PWMC_Handle_t *pHandle, uint8_t newConfig)
{
  uint8_t temp=0;  
  for (int8_t i=5; i>=0; i--)
  {
    temp = (uint8_t) ((newConfig >> i) & 0x01);
    if (temp == 1) pHandle->LSModArray[i] = 1;
    else pHandle->LSModArray[i] = 0;
  }
}

/**
  * @brief  It returns the Quasi Synch feature status
  * @param  pHandle: handler of the current instance of the PWM component
  * @retval uint8_t: 0=disabled, 1=enabled
  */
uint8_t PWMC_GetQuasiSynchState(PWMC_Handle_t * pHandle )
{
  return ((pHandle->QuasiSynchDecay) ? ((uint8_t) pHandle->QuasiSynchDecay ) : 0);
}

/**
  * @brief  It enables/disables the Qusi Synch feature at next step change
  * @param  pHandle: handler of the current instance of the PWM component
  * @param  uint8_t: 0=disable, 1=enable
  * @retval none
  */
void PWMC_SetQuasiSynchState(PWMC_Handle_t * pHandle, uint8_t State )
{
  if (State == 1)
  {
    pHandle->QuasiSynchDecay = true;
  }
  else
  {
    pHandle->QuasiSynchDecay = false;
  }    
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
