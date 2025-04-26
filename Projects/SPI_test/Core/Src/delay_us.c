/*
 * delay_us.c
 *
 *  Created on: Apr 21, 2025
 *      Author: Miloush
 */

#include "delay_us.h"

void DWT_Init(void)
{
    // Włącz dostęp do DWT (Debug Watch Trace)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // Reset licznika cykli
    DWT->CYCCNT = 0;

    // Włącz licznik
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = (HAL_RCC_GetHCLKFreq() / 1000000L) * us; // przeliczenie µs na cykle CPU
    while ((DWT->CYCCNT - start) < ticks);
}

