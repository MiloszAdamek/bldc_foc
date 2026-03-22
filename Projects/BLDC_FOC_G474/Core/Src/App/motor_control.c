/*
 * motor_control.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Milosz Adamek
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

void MotorControl_Init(TIM_HandleTypeDef* control_htim, TIM_HandleTypeDef* commander_htim)
{
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
        	SpeedController_Update();
            break;

        case STATE_FAULT:
            break;
    }
}

void MotorControl_Start(void)
{
    if (g_motor_state == STATE_IDLE) {
        g_motor_state = STATE_ALIGNMENT;

        // Uruchom kalibrację
        if(sensor_aligned){
            if (FOC_AlignSensor())
            {
                FOC_Start(); // Włącz PWM/ADC
                MotorControl_SetTorque(0.0f); // Przejdź do trybu momentu z zerowym prądem
            } else {
            	FOC_Stop();
                g_motor_state = STATE_FAULT; // Błąd kalibracji
            }
        }
        else{
        	FOC_Start();
        	MotorControl_SetTorque(0.0f); // Przejdź do trybu momentu z zerowym prądem
        }
    }
}

void MotorControl_Stop(void)
{
    FOC_Stop();
    g_motor_state = STATE_IDLE;
}

void MotorControl_SetSpeed(float rpm)
{
	if (g_motor_state == STATE_IDLE) {
	    FOC_Start();
	}
    target_speed_rpm = rpm;
    SpeedController_Reset();
    SpeedController_SetTarget_Ramp(rpm); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń
    g_motor_state = STATE_SPEED_CONTROL;
}

void MotorControl_SetTorque(float iq)
{
	if (g_motor_state == STATE_IDLE) {
	    FOC_Start();
	}
    target_torque_iq = iq;
    FOC_SetIqTarget_Ramp(iq); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń
    g_motor_state = STATE_TORQUE_CONTROL;
}

void MotorControl_Reboot()
{
	HAL_NVIC_SystemReset();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == foc_htim->Instance) // 40 kHz update
	{
		static bool foc_toggle = false;

		if (!__HAL_TIM_IS_TIM_COUNTING_DOWN(htim)) // 20kHz
		{
			foc_toggle = !foc_toggle;

			if (foc_toggle)                // Pętla FOC 10 kHz
			{
				// Sygnalizacja wykonania przerwania - obserwacja oscyloskopem
				FOC_Flag_GPIO_Port->BSRR = FOC_Flag_Pin; // GPIO_PIN_SET
				FOC_RunLoop();
				FOC_Flag_GPIO_Port->BSRR = (uint32_t)FOC_Flag_Pin << 16; // GPIO_PIN_RESET
			}
		}
	}
	if (htim->Instance == enc_htim->Instance) // Odczyt z enkodera 10 kHz
	{
        if (spi_ready) {
            spi_ready = false;
            // Sygnalizacja wykonania przerwania - obserwacja oscyloskopem
            SPI_Flag_GPIO_Port->BSRR = SPI_Flag_Pin; // GPIO_PIN_SET
            AS5048_ReadAngleDMA();
        }
	}
	if (htim->Instance == ctrl_htim->Instance) // Pętla regulacji prędkości 1 kHz
	{
		MotorControl_Run();
	}
	if (htim->Instance == cmd_htim->Instance) // Pętla obsługi wiersza poleceń 100 Hz
	{
		Commander_Process();
	}
}
