/*
 * motor_control_callbacks.c
 *
 *  Created on: Sep 1, 2026
 *      Author: Milosz Adamek
 */

#include "motor_control.h"
#include "board.h"

extern BoardHandleTypeDef board;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* FOC 10 kHz - priority 0 */
	if (htim->Instance == board.htim_pwm->Instance) // 40 kHz
	{
		// static bool foc_toggle = false;
		// if (!__HAL_TIM_IS_TIM_COUNTING_DOWN(htim)) // 20kHz
		// {
		// 	foc_toggle = !foc_toggle;
		// 	if (foc_toggle){ // Loop FOC 10 kHz
		// 		// FOC_Flag_GPIO_Port->BSRR = FOC_Flag_Pin; // GPIO_PIN_SET
		// 		FOC_RunLoop();
		// 		// FOC_Flag_GPIO_Port->BSRR = (uint32_t)FOC_Flag_Pin << 16; // GPIO_PIN_RESET
		// 	}
		// }
		return;
	}

	/* Encoder SPI 10 kHz - priority 1 */
	else if (htim->Instance == board.htim_enc->Instance)
	{
        MotorControl_OnEncoderSampleISR();
	}

	/* Speed 1 kHz - priority 2 */
	else if (htim->Instance == board.htim_speed->Instance)
	{
        MotorControl_OnSpeedISR();
	}

	/* Position 200 Hz - priority 3 */
	else if (htim->Instance == board.htim_pos->Instance)
	{
        MotorControl_OnPositionISR();
	}

	/* Commander CLI 100 Hz - priority 4 */
	else if (htim->Instance == board.htim_cmd->Instance)
	{
        MotorControl_OnCommandISR();
	}
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == board.hadc_currA.hadc->Instance) // 20 kHz
    {
        if (__HAL_TIM_IS_TIM_COUNTING_DOWN(board.htim_pwm)){ // 10 kHz
            MotorControl_OnCurrentSampleISR();
        }
    }
}
