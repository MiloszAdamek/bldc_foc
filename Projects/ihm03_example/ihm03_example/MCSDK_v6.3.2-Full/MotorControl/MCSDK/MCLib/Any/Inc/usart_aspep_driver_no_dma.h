/**
  ******************************************************************************
  * @file    usart_aspep_driver_no_dma.h
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
#ifndef usart_aspep_driver_no_dma_h
#define usart_aspep_driver_no_dma_h

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
  USART_TypeDef *USARTx;                   /* USART used for data transmission and reception.                                                         */
  uint16_t txLength;                       /*!< Length of the transmitted data packet : payload and header. Specific to C0 Single Shunt.              */
  uint16_t rxLengthDriver;                 /*!< Length of the received data packet used by driver : payload and header. Specific to C0 Single Shunt.  */
  uint8_t rxBufferIndex;                   /*!< Index of the received data packet : goes from 0 to rxLength. Specific to C0 Single Shunt.             */
  uint16_t txBufferIndex;                  /*!< Index of the transmitted data packet : goes from 0 to txLength. Specific to C0 Single Shunt.          */
  uint8_t *rxData;                         /*!< Pointer to the actual ASPEP received data : either rxBuffer or rxHeader. Specific to C0 Single Shunt. */
  uint8_t *txData;                         /*!< Pointer to the transmitted data byte per byte. Specific to C0 Single Shunt.                           */
} UASPEP_Handle_t;

void UASPEP_CFG_TRANSMISSION(void *pHandle, void *data, uint16_t length);
void UASPEP_CFG_RECEPTION(void *pHandle, void *buffer, uint16_t length);
bool UASPEP_TRIG_TRANSMISSION(void *pHandle);
bool UASPEP_TRIG_RECEPTION(void *pHandle);
void UASPEP_INIT(void *pHandle);
void UASPEP_IDLE_ENABLE(void *pHandle);

#endif
/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/
