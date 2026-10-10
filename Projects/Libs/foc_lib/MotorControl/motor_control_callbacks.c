/*
 * motor_control_callbacks.c
 *
 *  Created on: Sep 1, 2026
 *      Author: Milosz Adamek
 */

#include "motor_control.h"
#include "board.h"

extern BoardHandleTypeDef board;
extern MotorState_t g_motor_state;

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

	#if defined(USE_AS5048A_ENCODER)
	
		/* Encoder SPI 10 kHz - priority 1 */
		else if (htim->Instance == board.htim_enc->Instance)
		{
			MotorControl_OnEncoderSampleISR();
		}
		
	#endif

	/* Speed & DMA ADC regular conversions 1 kHz - priority 2 */
	else if (htim->Instance == board.htim_speed->Instance)
	{
        MotorControl_OnSpeedISR();
		// MotorControl_SlowLoopMeasurementsISR(); 
	}

	/* Position 200 Hz - priority 3 */
	else if (htim->Instance == board.htim_pos->Instance)
	{
        MotorControl_OnPositionISR();
	}
	
	/* CLI 100 Hz - priority 4 */
	else if (htim->Instance == board.htim_slow_loop->Instance)
	{
		#if defined(USE_CMD_INTERFACE)
        	MotorControl_OnCommandISR();
		#endif
	}
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == board.hadc_currA.hadc->Instance) // 20 kHz - Center aligned Mode 3, 10 kHz - Center aligned Mode 1
    {
		MotorControl_OnCurrentSampleISR();

		#if defined(USE_CAN_INTERFACE)
			MotorControl_OnCANISR();
		#endif

		MotorControl_SlowLoopMeasurementsISR();
		
        // if (__HAL_TIM_IS_TIM_COUNTING_DOWN(board.htim_pwm)){ // 10 kHz - gdy center aligned mode 3
        //     MotorControl_OnCurrentSampleISR();
        // }
    }
}

void HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == board.htim_pwm->Instance)
	{
		// Obsługa przerwania Break - np. wyłączenie PWM, ustawienie flagi FAULT
		Board_StopMotor(&board);
		g_motor_state = STATE_FAULT;
		MotorControl_GetFaults();
	}
}
