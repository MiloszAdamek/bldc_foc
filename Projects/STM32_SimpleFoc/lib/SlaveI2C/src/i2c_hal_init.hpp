#pragma once
#include "stm32g4xx_hal.h"

// Globalny handler I2C1
extern I2C_HandleTypeDef hi2c1;

// Funkcja do ręcznej inicjalizacji I2C1 w trybie slave
extern "C" void MX_I2C1_Init(void);

// Funkcja do ręcznej inicjalizacji I2C1 w trybie master
extern "C" void MX_I2C1_Init_Master(void);