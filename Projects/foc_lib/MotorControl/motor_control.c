/*
 * motor_control.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Milosz Adamek
 */

#include "motor_control.h"
#include "motor_algorithm.h"
#include "motor_alignment.h"
#include "config.h"
#include "commander.h"
#include "board.h"
#include "foc_loop.h"
#include "motor_types.h"
#include "speed_control.h"
#include "speed_estimator.h"
#include "position_control.h"
#include "as5048a.h"
#include "encoder_hub.h"
#include "gpio.h"
#include "svpwm.h"

#define ENCODER_TIMEOUT_LIMIT 100

extern BoardHandleTypeDef board;

Motor_Measurements_t g_meas;
Motor_References_t g_ref;
Motor_Output_t g_out;
Motor_Telemetry_t g_telem;

static ControlAlgorithm_t active_algorithm;

volatile MotorState_t g_motor_state = STATE_IDLE;

volatile uint32_t spi_ready_err = 0;
volatile uint32_t spi_ready_ok = 0;

// static float target_position = 0.0f;
// static float target_speed_rpm = 0.0f;
// static float target_torque_iq = 0.0f;

volatile bool speed_loop_enabled = false;
volatile bool position_loop_enabled = false;

// Debug - cubemonitor
volatile MonitorData_t monitor_data __attribute__((section(".fixed_logs_section")));

void MotorControl_Init(BoardHandleTypeDef* p_board)
{
	// === Wybór algorytmu sterowania ===
	active_algorithm = PI_FOC_Create();
	if(active_algorithm.Init) {
        active_algorithm.Init(active_algorithm.ctx);
    }

	HAL_TIM_Base_Start_IT(p_board->htim_speed);
    HAL_TIM_Base_Start_IT(p_board->htim_pos);
    HAL_TIM_Base_Start_IT(p_board->htim_cmd);
	PositionController_Init(POSITION_UNIT_RAD); // Wybór jednostki w regulatorze pozycji
	SpeedEstimator_Init(SPEED_PERIOD_SEC);
	Board_Init(p_board);
	SVPWM_Init(p_board->htim_pwm);

	MotorControl_Start();
}

static void Log_To_CubeMonitor(void)
{
    monitor_data.current_a = g_meas.currents.a;
    monitor_data.current_b = g_meas.currents.b;
    monitor_data.current_c = g_meas.currents.c;

    // Korzystamy ze znormalizowanej struktury telemetrycznej algorytmu (FOC/MPC)
    monitor_data.id = g_telem.id_meas;
    monitor_data.iq = g_telem.iq_meas;
    // monitor_data.iq_ref = g_telem.iq_ref;
    
    monitor_data.theta_el = g_meas.theta_el;
    monitor_data.theta_mech = g_meas.theta_mech;

    monitor_data.speed = g_meas.omega_mech;

    // Błędy nadrzędnych regulatorów logowane tak jak poprzednio
    monitor_data.position_err = position_err;
    monitor_data.position_ref = position_ref;
    monitor_data.position_reg_out = position_reg_out;
}

void MotorControl_Start(void)
{
	// Jeśli sensor nie jest skalibrowany, to uruchom procedurę kalibracji
	if(!MotorAlignment_IsAligned()){
		if (g_motor_state == STATE_IDLE) {
			g_motor_state = STATE_ALIGNMENT;
			if (!MotorAlignment_AlignSensor()) {
				MotorControl_Stop();
				g_motor_state = STATE_FAULT;
				return;
			}
		}
	}
	Board_StartMotor(&board);
	Board_StartPeripherals(&board);
	if (active_algorithm.Start) {
        active_algorithm.Start(active_algorithm.ctx);
    }
	MotorControl_SetTorque(0.15f);
}

void MotorControl_Stop(void)
{
	Board_StopMotor(&board);
	Board_StopPeripherals(&board);
    if (active_algorithm.Stop) {
        active_algorithm.Stop(active_algorithm.ctx);
    }
    g_motor_state = STATE_IDLE;
}

void MotorControl_SetPosition(float position)
{
	if (g_motor_state == STATE_IDLE) {
	    if (active_algorithm.Start) {
        active_algorithm.Start(active_algorithm.ctx);
    }
	}
    speed_loop_enabled = true;
    position_loop_enabled = true;

    // target_position = position;

	g_ref.position_ref = position;
    PositionController_Reset();
    PositionController_SetTarget(position); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń
    g_motor_state = STATE_RUN;
}

void MotorControl_SetSpeed(float rpm)
{
	if (g_motor_state == STATE_IDLE) {
	    if (active_algorithm.Start) {
	        active_algorithm.Start(active_algorithm.ctx);
	    }
	}
    speed_loop_enabled = true;
    position_loop_enabled = false;

    // target_speed_rpm = rpm;

	g_ref.speed_ref = rpm;
    SpeedController_Reset();
    SpeedController_SetTarget(rpm);
//    SpeedController_SetTarget_Ramp(rpm); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń
    g_motor_state = STATE_RUN;
}

void MotorControl_SetTorque(float iq)
{
	if (g_motor_state == STATE_IDLE) {
	    if (active_algorithm.Start) {
	        active_algorithm.Start(active_algorithm.ctx);
	    }
	}
    speed_loop_enabled = false;
    position_loop_enabled = false;

    // target_torque_iq = iq;
	// FOC_SetIqTarget_Ramp(iq); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń

	g_ref.torque_iq_ref = iq; // Algorytm sam zajmie się rampą wewnątrz swojej funkcji Update()

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

	// ==== Odczyt i przetwarzanie próbek prądów z ADC ===

	CurrentSense_Process_ISR();
	CurrentSense_CalculatePhases();
	CurrentSense_Read(&g_meas.currents);

	// === Odczyt i przetwarzanie próbek kąta z enkodera ===

	EncoderSample_t enc;
    static uint32_t encoder_timeout = 0;
    
    if(EncoderHub_ConsumeSample(&enc)){
        g_meas.theta_mech = enc.theta_mech;
        g_meas.theta_el = MotorAlignment_GetElectricalAngle(g_meas.theta_mech);
        
        encoder_timeout = 0;
        
        EncoderHub_PublishAngle(g_meas.theta_mech, g_meas.theta_el);
    } else {
        encoder_timeout++;
        
        if (encoder_timeout > ENCODER_TIMEOUT_LIMIT){
            MotorControl_Stop(); // Awaryjne zatrzymanie całego sterowania
            g_motor_state = STATE_FAULT;
            return;
        }
    }

	// === Pętla wybranego algorytmu sterowania ===

	if (active_algorithm.Update) {
        active_algorithm.Update(active_algorithm.ctx, &g_meas, &g_ref, &g_out);
    }

	// === Aktualizacja telemetrii ====

	if (active_algorithm.GetTelemetry) {
        active_algorithm.GetTelemetry(active_algorithm.ctx, &g_telem);
    }

	Log_To_CubeMonitor();

	// === Sygnalizacja wykonania przerwania - obserwacja oscyloskopem ===

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

