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

static TIM_HandleTypeDef* ctrl_htim;
static TIM_HandleTypeDef* cmd_htim;

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
            // Nic nie rób, czekaj na reset błędu
            // FOC_Stop() powinno być wywołane przy przejściu DO tego stanu
            break;
    }
}

// --- Publiczne API dla main.c ---

void MotorControl_Start(void) {
    if (g_motor_state == STATE_IDLE) {
        g_motor_state = STATE_ALIGNMENT;

        // Uruchom blokującą kalibrację
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
	if (htim->Instance == foc_htim->Instance) // pętla 20 kHz
	{
		// Pipeline FOC
		if (!__HAL_TIM_IS_TIM_COUNTING_DOWN(foc_htim)){	// Update na początku cyklu PWM, licznik osiągnął 0
			if (spi_ready) {
			AS5048_ReadAngleDMA();
			}
		}
		else{	// Update w środku cyklu PWM, licznik osiągnął ARR
			FOC_RunLoop();
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
