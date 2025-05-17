/**
  ******************************************************************************
  * @file    bus_voltage_sensor.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file contains all definitions and functions prototypes for the
  *          BusVoltageSensor component of the Motor Control SDK.
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
  * @ingroup BusVoltageSensor
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BUSVOLTAGESENSOR_H
#define BUSVOLTAGESENSOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "mc_type.h"

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup BusVoltageSensor
  * @{
  */

/**
  * @brief  BusVoltageSensor handle definition
  */
typedef struct
{
  SensorType_t SensorType;    /*!< It contains the information about the type
                                   of instanced bus voltage sensor object.
                                   It can be equal to REAL_SENSOR or
                                   VIRTUAL_SENSOR */
  uint16_t ConversionFactor;  /*!< It is used to convert bus voltage from
                                   u16Volts into real Volts (V).
                                   1 u16Volt = 65536/ConversionFactor Volts
                                   For real sensors ConversionFactor it is
                                   equal to the product between the expected MCU
                                   voltage and the voltage sensing network
                                   attenuation. For virtual sensors it must
                                   be equal to 500 */

  uint16_t LatestConv;        /*!< It contains latest Vbus converted value
                                   expressed in u16Volt format */
  uint16_t AvBusVoltage_d;    /*!< It contains latest available average Vbus
                                   expressed in u16Volt format */
  uint16_t FaultState;        /*!< It contains latest Fault code (#MC_NO_ERROR,
                                   #MC_OVER_VOLT or #MC_UNDER_VOLT) */
} BusVoltageSensor_Handle_t;


/* Exported functions ------------------------------------------------------- */
uint16_t VBS_GetBusVoltage_d(const BusVoltageSensor_Handle_t *pHandle);
uint16_t VBS_GetAvBusVoltage_V(const BusVoltageSensor_Handle_t *pHandle);

/**
  * @brief  It return latest averaged Vbus measurement expressed in u16Volt format
  * @param  pHandle related Handle of BusVoltageSensor_Handle_t
  * @retval uint16_t Latest averaged Vbus measurement in u16Volt format
  */
static inline uint16_t VBS_GetAvBusVoltage_d(const BusVoltageSensor_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_BUS_VOLT
  uint16_t temp_avBusVoltage_d;

  if (MC_NULL == pHandle)
  {
    temp_avBusVoltage_d = 0U;
  }
  else
  {
    temp_avBusVoltage_d = pHandle->AvBusVoltage_d;
  }
  return (temp_avBusVoltage_d);
#else
  return (pHandle->AvBusVoltage_d);
#endif
}

/**
  * @brief  It returns #MC_OVER_VOLT, #MC_UNDER_VOLT or #MC_NO_ERROR depending on
  *         bus voltage and protection threshold values
  * @param  pHandle related Handle of BusVoltageSensor_Handle_t
  * @retval uint16_t Fault code error
  */
static inline uint16_t VBS_CheckVbus(const BusVoltageSensor_Handle_t *pHandle)
{
#ifdef NULL_PTR_CHECK_BUS_VOLT
  uint16_t temp_faultState;

  if (MC_NULL ==  pHandle)
  {
    temp_faultState = 0U;
  }
  else
  {
    temp_faultState = pHandle->FaultState;
  }
  return (temp_faultState);
#else
  return (pHandle->FaultState);
#endif
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

#endif /* BUSVOLTAGESENSOR_H */

/************************ (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/
