/**
  ******************************************************************************
  * @file    usart_aspep_driver.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file contains all definitions and functions prototypes for the
  *           uart driver for the aspep protocol.
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
#ifndef usart_aspep_driver_h
#define usart_aspep_driver_h

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
  USART_TypeDef *USARTx;  /*!< USART used for MCP communication */
  DMA_TypeDef *rxDMA;     /*!< DMA used for MCP data reception */
  DMA_TypeDef *txDMA;     /*!< DMA used for MCP data transmission */
  uint32_t rxChannel;     /*!< DMA channel for MCP data reception */
  uint32_t txChannel;     /*!< DMA channel for MCP data transmission */
} UASPEP_Handle_t;

void UASPEP_CFG_TRANSMISSION(void *pHandle, void *data, uint16_t length);
void UASPEP_CFG_RECEPTION(void *pHandle, void *buffer, uint16_t length);
void UASPEP_INIT(void *pHandle);
void UASPEP_IDLE_ENABLE(void *pHandle);

#endif
/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/
