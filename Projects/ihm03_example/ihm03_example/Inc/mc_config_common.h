
/**
  ******************************************************************************
  * @file    mc_config_common.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   Motor Control Subsystem components configuration and handler
  *          structures declarations.
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

#ifndef MC_CONFIG_COMMON_H
#define MC_CONFIG_COMMON_H

#include "pid_regulator.h"
#include "virtual_speed_sensor.h"
#include "ntc_temperature_sensor.h"
#include "mc_interface.h"
#include "mc_configuration_registers.h"
#include "regular_conversion_manager.h"
#include "r_divider_bus_voltage_sensor.h"
#include "virtual_bus_voltage_sensor.h"

extern RegConv_t TempRegConv_M1;
extern NTC_Handle_t TempSensor_M1;
extern VirtualSpeedSensor_Handle_t VirtualSpeedSensorM1;
extern RegConv_t VbusRegConv_M1;
extern RDivider_Handle_t BusVoltageSensor_M1;
extern PWMC_Handle_t *pwmcHandle[NBR_OF_MOTORS];
extern NTC_Handle_t *pTemperatureSensor[NBR_OF_MOTORS];

/* USER CODE BEGIN Additional extern */

/* USER CODE END Additional extern */

#endif /* MC_CONFIG_COMMON_H */
/******************* (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/
