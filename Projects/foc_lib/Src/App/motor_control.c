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
#include "FOC/speed_estimator.h"
#include "FOC/position_control.h"
#include "BSP/as5048a.h"
#include "gpio.h"

static TIM_HandleTypeDef* speed_ctrl_htim;
static TIM_HandleTypeDef* position_ctrl_htim;
static TIM_HandleTypeDef* cmd_htim;

volatile uint32_t spi_ready_err = 0;
volatile uint32_t spi_ready_ok = 0;

volatile MotorState_t g_motor_state = STATE_IDLE;
static float target_position = 0.0f;
static float target_speed_rpm = 0.0f;
static float target_torque_iq = 0.0f;

volatile bool speed_loop_enabled = false;
volatile bool position_loop_enabled = false;

void MotorControl_Init(TIM_HandleTypeDef* speed_control_htim, TIM_HandleTypeDef* position_control_htim, TIM_HandleTypeDef* commander_htim)
{
	speed_ctrl_htim = speed_control_htim;
	position_ctrl_htim = position_control_htim;
	cmd_htim = commander_htim;
	HAL_TIM_Base_Start_IT(speed_ctrl_htim);
	HAL_TIM_Base_Start_IT(position_ctrl_htim);
	HAL_TIM_Base_Start_IT(cmd_htim);
	PositionController_Init(POSITION_UNIT_RAD); // Wybór jednostki w regulatorze pozycji
	SpeedEstimator_Init(SPEED_PERIOD_SEC);
	MotorControl_Start();
}

void MotorControl_Start(void)
{
    if (g_motor_state == STATE_IDLE) {
        g_motor_state = STATE_ALIGNMENT;

        // Uruchom kalibrację

//        if (!FOC_IsSensorAligned()) {
//            if (!FOC_AlignSensor()) {
//                FOC_Stop();
//                g_motor_state = STATE_FAULT;
//                return;
//            }
//        }

        FOC_Start();
//        MotorControl_SetTorque(0.0f);

        // old
//        if(sensor_aligned){
//            if (FOC_AlignSensor())
//            {
//                FOC_Start(); // Włącz PWM/ADC
//                MotorControl_SetTorque(0.0f); // Przejdź do trybu momentu z zerowym prądem
//            } else {
//            	FOC_Stop();
//                g_motor_state = STATE_FAULT; // Błąd kalibracji
//            }
//        }
//        else{
//        	FOC_Start();
//        	MotorControl_SetTorque(0.0f); // Przejdź do trybu momentu z zerowym prądem
//        }
    }
}

void MotorControl_Stop(void)
{
    FOC_Stop();
    g_motor_state = STATE_IDLE;
}

void MotorControl_SetPosition(float position)
{
	if (g_motor_state == STATE_IDLE) {
	    FOC_Start();
	}
    speed_loop_enabled = true;
    position_loop_enabled = true;

    target_position = position;
    PositionController_Reset();
    PositionController_SetTarget(target_position); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń
    g_motor_state = STATE_RUN;
}

void MotorControl_SetSpeed(float rpm)
{
	if (g_motor_state == STATE_IDLE) {
	    FOC_Start();
	}
    speed_loop_enabled = true;
    position_loop_enabled = false;

    target_speed_rpm = rpm;
    SpeedController_Reset();
    SpeedController_SetTarget(rpm);
//    SpeedController_SetTarget_Ramp(rpm); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń
    g_motor_state = STATE_RUN;
}

void MotorControl_SetTorque(float iq)
{
	if (g_motor_state == STATE_IDLE) {
	    FOC_Start();
	}
    speed_loop_enabled = false;
    position_loop_enabled = false;

    target_torque_iq = iq;
    FOC_SetIqTarget_Ramp(iq); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń
    g_motor_state = STATE_RUN;
}

void MotorControl_Reboot()
{
	HAL_NVIC_SystemReset();
}

void MotorControl_SetState(MotorState_t new_state){
	g_motor_state = new_state;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* FOC 10 kHz - priority 0 */
	if (htim->Instance == FOC_GetPwmTimer()->Instance) // 40 kHz
	{
		static bool foc_toggle = false;
		if (!__HAL_TIM_IS_TIM_COUNTING_DOWN(htim)) // 20kHz
		{
			foc_toggle = !foc_toggle;
			if (foc_toggle){ // Loop FOC 10 kHz
				// FOC_Flag_GPIO_Port->BSRR = FOC_Flag_Pin; // GPIO_PIN_SET
				FOC_RunLoop();
				// FOC_Flag_GPIO_Port->BSRR = (uint32_t)FOC_Flag_Pin << 16; // GPIO_PIN_RESET
			}
		}
		return;
	}

	/* Encoder SPI 10 kHz - priority 1 */
	else if (htim->Instance == FOC_GetEncTimer()->Instance)
	{
        if (spi_ready) {
            // SPI_Flag_GPIO_Port->BSRR = SPI_Flag_Pin; // GPIO_PIN_SET
            AS5048_ReadAngleDMA();
            /* CS_HIGH and spi_ready=true in DMA callback */
        }
        return;
	}

	/* Speed 1 kHz - priority 2 */
	else if (htim->Instance == speed_ctrl_htim->Instance)
	{
        SpeedEstimator_Update();          /* Prediction or corection */
        if (speed_loop_enabled){
        	SpeedController_Update();
        }
        return;
	}

	/* Position 200 Hz - priority 3 */
	else if (htim->Instance == position_ctrl_htim->Instance)
	{
		if (position_loop_enabled){
			PositionController_Update();
		}
		return;
	}

	/* Commander CLI 100 Hz - priority 4 */
	else if (htim->Instance == cmd_htim->Instance)
	{
		Commander_Process();
		return;
	}
}
