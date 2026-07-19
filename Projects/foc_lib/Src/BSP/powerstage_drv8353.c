/*
 * powerstage_drv8353.c
 *
 *  Created on: Jul 18, 2026
 *      Author: Milosz Adamek
 */
    
#include "App/config.h"

#ifdef DRV8353

#include "BSP/powerstage.h"
#include "BSP/drv8353.h"
#include "spi.h"
#include "tim.h"

static DRV8353_HandleTypeDef g_drv;

void PowerStage_Init(void)
{
    DRV8353_Init(&g_drv, &hspi2, &htim1);
    PowerStage_Off();
}

void PowerStage_Off(void)
{
    DRV8353_SetOutputState(&g_drv, DRV_OUTPUT_COAST);
    DRV8353_PWMDisable(&g_drv);
}

void PowerStage_On(void)
{

}

void PowerStage_StartPWM(TIM_HandleTypeDef *htim)
{
    // start timera
}

void PowerStage_StopPWM(TIM_HandleTypeDef *htim)
{
    DRV8353_PWMDisable(&g_drv);
}

#endif
