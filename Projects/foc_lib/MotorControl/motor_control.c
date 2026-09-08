/*
 * motor_control.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Milosz Adamek
 */

#include "motor_control.h"
#include "config.h"
#include "commander.h"
#include "board.h"
#include "foc_loop.h"
#include "speed_control.h"
#include "speed_estimator.h"
#include "position_control.h"
#include "as5048a.h"
#include "gpio.h"

extern BoardHandleTypeDef board;

volatile MotorState_t g_motor_state = STATE_IDLE;

volatile uint32_t spi_ready_err = 0;
volatile uint32_t spi_ready_ok = 0;

static float target_position = 0.0f;
static float target_speed_rpm = 0.0f;
static float target_torque_iq = 0.0f;

volatile bool speed_loop_enabled = false;
volatile bool position_loop_enabled = false;

void MotorControl_Init(BoardHandleTypeDef* p_board)
{
	HAL_TIM_Base_Start_IT(p_board->htim_speed);
    HAL_TIM_Base_Start_IT(p_board->htim_pos);
    HAL_TIM_Base_Start_IT(p_board->htim_cmd);
	PositionController_Init(POSITION_UNIT_RAD); // Wybór jednostki w regulatorze pozycji
	SpeedEstimator_Init(SPEED_PERIOD_SEC);
	Board_Init(p_board);
	MotorControl_Start();
}

void MotorControl_Start(void)
{
	// Jeśli sensor nie jest skalibrowany, to uruchom procedurę kalibracji
	if(!FOC_IsSensorAligned()){
		if (g_motor_state == STATE_IDLE) {
			g_motor_state = STATE_ALIGNMENT;
			// Uruchom kalibrację
			if (!FOC_IsSensorAligned()) {
				if (!FOC_AlignSensor()) {
					// Błąd alignmentu FOC
					MotorControl_Stop();
					g_motor_state = STATE_FAULT;
					return;
				}
			}
		}	
	}
	Board_StartMotor(&board);
	Board_StartPeripherals(&board);
	FOC_Start();
	MotorControl_SetTorque(0.15f);
}

void MotorControl_Stop(void)
{
	Board_StopMotor(&board);
	Board_StopPeripherals(&board);
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

void MotorControl_SetState(MotorState_t new_state)
{
	g_motor_state = new_state;
}

void MotorControl_OnCurrentSampleISR()
{
	CurrentSense_Process_ISR();
	currents_ready = true;

	CurrentSense_CalculatePhases();

	// Tu powinien być odczyt kąta, ewentualnie w przerwaniu od TIM_ENC

	FOC_RunLoop();

	// Sygnalizacja wykonania przerwania - obserwacja oscyloskopem
	// ADC_Conv_Flag_GPIO_Port->BSRR = ADC_Conv_Flag_Pin; // GPIO_PIN_SET
	// ADC_Conv_Flag_GPIO_Port->BSRR = (uint32_t)ADC_Conv_Flag_Pin << 16; // GPIO_PIN_RESET
}

void MotorControl_OnEncoderSampleISR()
{
	if (spi_ready) {
		// SPI_Flag_GPIO_Port->BSRR = SPI_Flag_Pin; // GPIO_PIN_SET
		AS5048_ReadAngleDMA();
		/* CS_HIGH and spi_ready=true in DMA callback */
	}
}

void MotorControl_OnSpeedISR()
{
	SpeedEstimator_Update();          /* Prediction or corection */
	if (speed_loop_enabled){
		SpeedController_Update();
	}
}

void MotorControl_OnPositionISR()
{
	if (position_loop_enabled){
		PositionController_Update();
	}
}

void MotorControl_OnCommandISR()
{
	Commander_Process();
}

