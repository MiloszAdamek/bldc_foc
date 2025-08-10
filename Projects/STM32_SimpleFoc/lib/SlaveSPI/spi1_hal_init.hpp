#pragma once
#include "stm32g4xx_hal.h"

// Globalny handler SPI2
extern SPI_HandleTypeDef hspi1;

// Master init
extern "C" void MX_SPI1_Init_Master(void);

// Slave init
extern "C" void MX_SPI1_Init_Slave(void);