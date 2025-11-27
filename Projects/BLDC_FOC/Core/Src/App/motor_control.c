/*
 * motor_control.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Miloush
 */

#include "App/motor_control.h"
#include "App/config.h"
#include "App/commander.h"
#include "FOC/foc_loop.h"
#include "FOC/speed_control.h"
#include "BSP/as5048a.h"
#include "gpio.h"

static TIM_HandleTypeDef* ctrl_htim;
static TIM_HandleTypeDef* cmd_htim;

volatile uint32_t spi_ready_err = 0;
volatile uint32_t spi_ready_ok = 0;

volatile MotorState_t g_motor_state = STATE_IDLE;
static float target_speed_rpm = 0.0f;
static float target_torque_iq = 0.0f;

void MotorControl_Init(TIM_HandleTypeDef* control_htim, TIM_HandleTypeDef* commander_htim){
	ctrl_htim = control_htim;
	cmd_htim = commander_htim;
	HAL_TIM_Base_Start_IT(ctrl_htim);
	HAL_TIM_Base_Start_IT(cmd_htim);
	MotorControl_Start();
}

void MotorControl_Run(void)
{
    switch (g_motor_state)
    {
        case STATE_IDLE:
            break;

        case STATE_ALIGNMENT:
            break;

        case STATE_TORQUE_CONTROL:
            break;

        case STATE_SPEED_CONTROL:
        	float iq_cmd = SpeedController_Update(target_speed_rpm);
        	FOC_SetIqTarget(iq_cmd);
            break;

        case STATE_FAULT:
            // FOC_Stop() powinno być wywołane przy przejściu do tego stanu
            break;
    }
}

void MotorControl_Start(void) {
    if (g_motor_state == STATE_IDLE) {
        g_motor_state = STATE_ALIGNMENT;

        // Uruchom kalibrację
        if (FOC_AlignSensor())
        {
            FOC_Start(); // Włącz PWM/ADC
            MotorControl_SetTorque(0.0f); // Przejdź do trybu momentu z zerowym prądem
        } else {
            g_motor_state = STATE_FAULT; // Błąd kalibracji
        }
    }
}

void MotorControl_Stop(void) {
    FOC_Stop();
    g_motor_state = STATE_IDLE;
}

void MotorControl_SetSpeed(float rpm) {
    target_speed_rpm = rpm;
    SpeedController_Reset();
    g_motor_state = STATE_SPEED_CONTROL;
}

void MotorControl_SetTorque(float iq) {
    target_torque_iq = iq; // Zapisz cel
    g_motor_state = STATE_TORQUE_CONTROL;

    FOC_SetIqTarget_Ramp(iq); // Aktywacja rampy tylko raz
}

void MotorControl_Reboot(){
	HAL_NVIC_SystemReset();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == foc_htim->Instance) // 40 kHz update
	{
		static bool foc_toggle = false;

		// CNT = ARR → counting DOWN (20 kHz)
		if (!__HAL_TIM_IS_TIM_COUNTING_DOWN(htim))
		{
			foc_toggle = !foc_toggle;      // dzieli CNT=0 na pół → 10 kHz

			if (foc_toggle)                // FOC = 10 kHz
			{
				HAL_GPIO_WritePin(TIM1_Update_Flag_GPIO_Port, TIM1_Update_Flag_Pin, GPIO_PIN_SET);
				FOC_RunLoop();
				HAL_GPIO_WritePin(TIM1_Update_Flag_GPIO_Port, TIM1_Update_Flag_Pin, GPIO_PIN_RESET);
			}
		}
		else
		{
//			spi_toggle = !spi_toggle;
//			if(spi_toggle){
//		        if (spi_ready) {
//		            spi_ready = false;
//		            GPIOC->BSRR = (1U << 9);
//		            AS5048_ReadAngleDMA();
//		        }
//			}
		}
	}
	if (htim->Instance == enc_htim->Instance) // pętla 10 kHz
	{
        if (spi_ready) {
            spi_ready = false;
            GPIOC->BSRR = (1U << 9); // PC9
            AS5048_ReadAngleDMA();
        }
	}
	if (htim->Instance == ctrl_htim->Instance) // pętla 1 kHz
	{
		MotorControl_Run();
	}
	if (htim->Instance == cmd_htim->Instance) // pętla 100 Hz
	{
		Commander_Process();
	}
}
