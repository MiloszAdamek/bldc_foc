/**
  ******************************************************************************
  * @file    usart_aspep_driver_no_dma.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file provides firmware functions that implement the uart driver 
  *          for the aspep protocol without using any DMA
  *
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
  */
  
#include <stdint.h>
#include "mc_stm_types.h"
#include "usart_aspep_driver_no_dma.h"
#include "aspep.h"

void UASPEP_TX_INIT(UASPEP_Handle_t *pHandle);
void UASPEP_RX_INIT(UASPEP_Handle_t *pHandle);

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup MCP
  * @{
  */

  
/*
  * @brief  Initialization of the Hardware used for data transmission and reception.
  * 
  * @param  pHandle Handler of the current instance of the UASPEP component
  */
void UASPEP_INIT(void *pASPEPHandle)
{
  UASPEP_Handle_t *pHandle = (UASPEP_Handle_t *)pASPEPHandle; //cstat !MISRAC2012-Rule-11.5
  UASPEP_RX_INIT(pHandle);
  UASPEP_TX_INIT(pHandle);
}


/*
  * @brief  Configures the Hardware used for data transmission to controller.
  * 
  * @param  pHandle Handler of the current instance of the UASPEP component
  */
void UASPEP_TX_INIT(UASPEP_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_USA_ASP_DRV
  if (NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
#endif
    /* Clear UART ISR */
    LL_USART_ClearFlag_TC(pHandle->USARTx);
    LL_USART_ClearFlag_TXFE(pHandle->USARTx);
#ifdef NULL_PTR_CHECK_USA_ASP_DRV
  }
#endif
}

/*
  * @brief  Configures the Hardware used for data reception from controller.
  * 
  * @param  pHandle Handler of the current instance of the UASPEP component
  */
void UASPEP_RX_INIT(UASPEP_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_USA_ASP_DRV
  if (NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
#endif
    /* Enable Error interrupt (EIE) to unmask Overrun interrupt */
    LL_USART_EnableIT_ERROR(pHandle->USARTx);

    LL_USART_EnableIT_RXNE(pHandle->USARTx);
    LL_USART_ReceiveData8(pHandle->USARTx);
#ifdef NULL_PTR_CHECK_USA_ASP_DRV
  }
#endif
}

/*
  * @brief  Configures data transmission.
  * 
  * @param  pDataTxHandle Communication handle
  * @param  data Data to be transmitted to controller
  * @param  length Length of the data to be transmitted
  */
void UASPEP_CFG_TRANSMISSION(void *pDataTxHandle, void *data, uint16_t length)
{
  UASPEP_Handle_t *pASPEPHandle = (UASPEP_Handle_t *)pDataTxHandle; //cstat !MISRAC2012-Rule-11.5

  pASPEPHandle->txBufferIndex = 0;
  pASPEPHandle->txData = data;
  pASPEPHandle->txLength = length;
  LL_USART_EnableIT_TXE_TXFNF(pASPEPHandle->USARTx);
}

/*
  * @brief  Configures data reception.
  * 
  * @param  pDataRxHandle Communication handle
  * @param  buffer Buffer which will receive the communicated data
  * @param  length Length of the received data
  */
void UASPEP_CFG_RECEPTION(void *pDataRxHandle, void* buffer, uint16_t length)
{
  UASPEP_Handle_t *pHandle = (UASPEP_Handle_t *)pDataRxHandle; //cstat !MISRAC2012-Rule-11.5
  
  pHandle->rxBufferIndex = 0;
  pHandle->rxLengthDriver = length;
  pHandle->rxData = buffer;
  LL_USART_EnableIT_RXNE(pHandle->USARTx);
}

/**
  * @brief  Triggers data transmission.
  *
  * Specific to STM32C0 on Single Shunt topology, for which no DMA is used for USART communication.
  * 
  * @param  pASPEPHandle Communication handle
  */
bool UASPEP_TRIG_TRANSMISSION(void *pASPEPHandle)
{
  UASPEP_Handle_t *pHandle = (UASPEP_Handle_t *)pASPEPHandle; //cstat !MISRAC2012-Rule-11.5
  bool result = false;
  
  uint8_t txByte = pHandle->txData[pHandle->txBufferIndex];
  LL_USART_TransmitData8(pHandle->USARTx,txByte);
  pHandle->txBufferIndex += 1U;

  if(pHandle->txBufferIndex == pHandle->txLength){
    LL_USART_DisableIT_TXE_TXFNF(pHandle->USARTx);
    result = true;
  }
  return(result);
}

/**
  * @brief  Triggers data reception.
  *
  * Specific to STM32C0 on Single Shunt topology, for which no DMA is used for USART communication.
  * 
  * @param  pASPEPHandle Communication handle
  */
bool UASPEP_TRIG_RECEPTION(void *pASPEPHandle)
{
  UASPEP_Handle_t *pHandle = (UASPEP_Handle_t *)pASPEPHandle; //cstat !MISRAC2012-Rule-11.5
  bool result = false;
  
  pHandle->rxData[pHandle->rxBufferIndex] = (uint32_t) LL_USART_ReceiveData8(pHandle->USARTx);
  pHandle->rxBufferIndex += 1U;
  if(pHandle->rxBufferIndex == pHandle->rxLengthDriver){
    LL_USART_DisableIT_RXNE_RXFNE(pHandle->USARTx);
    result = true;
  }
  return(result);
}

/*
  * @brief  Sets IDLE state : no transmission on going.
  * 
  * @param  pHandle Handler of the current instance of the UASPEP component
  */
void UASPEP_IDLE_ENABLE(void *pASPEPHandle)
{
  UASPEP_Handle_t *pHandle = (UASPEP_Handle_t *)pASPEPHandle; //cstat !MISRAC2012-Rule-11.5
  LL_USART_ClearFlag_IDLE(pHandle->USARTx);
  LL_USART_EnableIT_IDLE(pHandle->USARTx);
}

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/

