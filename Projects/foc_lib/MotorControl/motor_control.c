/*
 * motor_control.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Milosz Adamek
 */

#include "motor_control.h"
#include "motor_algorithm.h"
#include "motor_alignment.h"
#include "motor_types.h"
#include "config.h"
#include "board.h"
#include "foc_loop.h"
#include "speed_control.h"
#include "speed_estimator.h"
#include "position_control.h"
#include "as5048a.h"
#include "encoder_hub.h"
#include "svpwm.h"
#include "current_sense.h"
#include "voltage_sense.h"

#define ENCODER_TIMEOUT_LIMIT 100

volatile bool g_cmd_flag = false; // Flaga ustawiona w przerwaniu TIM, komenda przetwarzana w pętli while()

extern BoardHandleTypeDef board;

volatile Motor_References_t g_ref; // Volatile, bo może być modyfikowane w ISR Commander_Process() i w ISR MotorControl_SlowLoopMeasurementsISR()
volatile Motor_Telemetry_t g_telem; // Zapisywane w ISR MotorControl_OnCurrentSampleISR() i odczytywane w ISR MotorControl_SlowLoopMeasurementsISR()

Motor_Calibration_t g_calibration; // Ustawiane tylko raz na starcie, potem odczyt

volatile Motor_Stats_t g_stats; // Volatile, bo może być modyfikowane w ISR

static ControlAlgorithm_t active_algorithm;

MotorState_t g_motor_state = STATE_IDLE;

volatile uint32_t spi_ready_err = 0;
volatile uint32_t spi_ready_ok = 0;

volatile bool speed_loop_enabled = false;
volatile bool position_loop_enabled = false;

// Debug - cubemonitor
volatile MonitorData_t monitor_data __attribute__((section(".fixed_logs_section")));

static inline void MotorControl_LogCubeMonitor(const Motor_Measurements_t *meas);
static inline bool MotorControl_BuildMeasurements(Motor_Measurements_t *meas);
static inline void MotorControl_BuildReferences(Motor_References_t *ref);

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

static inline void MotorControl_LogCubeMonitor(const Motor_Measurements_t *meas)
{
    monitor_data.current_a  = meas->currents.a;
    monitor_data.current_b  = meas->currents.b;
    monitor_data.current_c  = meas->currents.c;
    monitor_data.theta_el   = meas->theta_el;
    monitor_data.theta_mech = meas->theta_mech;
    monitor_data.speed      = meas->omega_mech_rpm;

    monitor_data.id         = g_telem.id_meas;
    monitor_data.iq         = g_telem.iq_meas;
    monitor_data.iq_ref     = g_telem.iq_ref;
    monitor_data.id_ref     = g_telem.id_ref;
    monitor_data.vd_out     = g_telem.vd_out;
    monitor_data.vq_out     = g_telem.vq_out;

    // Błędy nadrzędnych regulatorów logowane tak jak poprzednio
    // monitor_data.position_err = position_err;
    // monitor_data.position_ref = position_ref;
    // monitor_data.position_reg_out = position_reg_out;
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
	MotorControl_SetTorque_Iq(0.15f);
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

	// WYŁĄCZ rampę momentu (pracuje regulator pozycji i prędkości)
    g_ref.iq_ramp_enabled = false;

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

	// WYŁĄCZ rampę momentu (pracuje tylko rampa regulatora prędkości)
    g_ref.iq_ramp_enabled = false;

	g_ref.speed_ref = rpm;
    SpeedController_Reset();
    SpeedController_SetTarget(rpm);
//    SpeedController_SetTarget_Ramp(rpm); // Aktywacja rampy tylko przy zmianie wartości zadanej w wierszu poleceń
    g_motor_state = STATE_RUN;
}

void MotorControl_SetTorque_Iq(float iq)
{
	if (g_motor_state == STATE_IDLE) {
	    if (active_algorithm.Start) {
	        active_algorithm.Start(active_algorithm.ctx);
	    }
	}
    speed_loop_enabled = false;
    position_loop_enabled = false;

	// WŁĄCZ rampę momentu (sterujemy prądem bezpośrednio)
    g_ref.iq_ramp_enabled = true;

	g_ref.torque_iq_ref = iq; // Algorytm sam zajmie się rampą wewnątrz swojej funkcji Update()
    g_motor_state = STATE_RUN;
}

void MotorControl_SetTorque_mNm(float torque_mNm)
{
    // Iq = (T_mNm / 1000) / Kt
    float target_iq = (torque_mNm / 1000.0f) / MOTOR_TORQUE_CONSTANT;
	MotorControl_SetTorque_Iq(target_iq);
}

void MotorControl_Reboot(void)
{
	HAL_NVIC_SystemReset();
}

void MotorControl_SetState(MotorState_t new_state)
{
	g_motor_state = new_state;
}

void MotorControl_OnCurrentSampleISR(void)
{	
    Motor_Measurements_t meas;
    Motor_References_t ref;
    Motor_Output_t out;

	if (!MotorControl_BuildMeasurements(&meas))
	{
		return;
	}

	MotorControl_BuildReferences(&ref);

	// === Pętla wybranego algorytmu sterowania ===
    if (active_algorithm.Update)
    {
        active_algorithm.Update(
            active_algorithm.ctx,
            &meas,
            &ref,
            &out
        );
    }

	// === Wysterowanie wyjść PWM ===

	// === Aktualizacja telemetrii ====

    if (active_algorithm.GetTelemetry)
    {
        active_algorithm.GetTelemetry(
            active_algorithm.ctx,
            &g_telem
        );
    }

	MotorControl_LogCubeMonitor(&meas);

	// === Sygnalizacja wykonania przerwania - obserwacja oscyloskopem ===

	// ADC_Conv_Flag_GPIO_Port->BSRR = ADC_Conv_Flag_Pin; // GPIO_PIN_SET
	// ADC_Conv_Flag_GPIO_Port->BSRR = (uint32_t)ADC_Conv_Flag_Pin << 16; // GPIO_PIN_RESET
}

static inline bool MotorControl_BuildMeasurements(Motor_Measurements_t *meas)
{
	// ==== Odczyt i przetwarzanie próbek prądów z ADC ===
	CurrentSense_Process_ISR();
	CurrentSense_CalculatePhases();
	CurrentSense_Read(&meas->currents);

	// === Odczyt i przetwarzanie próbek kąta z enkodera ===
	EncoderSample_t enc;

    static uint32_t encoder_timeout = 0;
    static float last_theta_mech = 0.0f;
    static float last_theta_el = 0.0f;

    if (EncoderHub_ConsumeSample(&enc))
    {
        last_theta_mech = enc.theta_mech;
        last_theta_el = MotorAlignment_GetElectricalAngle(last_theta_mech);

        encoder_timeout = 0;

        EncoderHub_PublishAngle(
            last_theta_mech,
            last_theta_el
        );
    }
    else
    {
        encoder_timeout++;

        if (encoder_timeout > ENCODER_TIMEOUT_LIMIT)
        {
            MotorControl_Stop();
            g_motor_state = STATE_FAULT;
            return false;
        }
    }
    
    // === Zapis ostatnich wartości kątów, wersja bez ekstrapolacji ===
    // meas->theta_mech = last_theta_mech;
    // meas->theta_el   = last_theta_el;

    // === Odczyt prędkości mechanicznej ===
    meas->omega_mech_rpm = SpeedEstimator_GetOmegaRPM_ISR();
    meas->omega_mech_rad_s = SpeedEstimator_GetOmegaRad_s_ISR();


    // === Ekstrapolacja kąta w przód o T_DELAY ===
    const float T_DELAY = 125e-6f; // 125 us - na próbę (zmieniaj 100..150us)
    meas->theta_mech = normalize_angle(last_theta_mech + meas->omega_mech_rad_s * T_DELAY);
    meas->theta_el = MotorAlignment_GetElectricalAngle(meas->theta_mech);

	// === Odczyt napięcia Vbus ===
    meas->v_bus = VoltageSense_GetVbus_ISR();

	return true;
}

static inline void MotorControl_BuildReferences(Motor_References_t *ref)
{
    ref->torque_iq_ref   = g_ref.torque_iq_ref;
    ref->speed_ref       = g_ref.speed_ref;
    ref->position_ref    = g_ref.position_ref;
    ref->iq_ramp_enabled = g_ref.iq_ramp_enabled;
}

void MotorControl_OnEncoderSampleISR(void)
{
	if (spi_ready) {
		// SPI_Flag_GPIO_Port->BSRR = SPI_Flag_Pin; // GPIO_PIN_SET
		AS5048_ReadAngleDMA();
		/* CS_HIGH and spi_ready=true in DMA callback */
	}
}

void MotorControl_OnSpeedISR(void)
{
	SpeedEstimator_Update();
	if (speed_loop_enabled){
		SpeedController_Update(&g_ref);
	}
}

void MotorControl_OnPositionISR(void)
{
	if (position_loop_enabled){
		PositionController_Update();
	}
}

void MotorControl_OnCommandISR(void)
{
    g_cmd_flag = true;
    // W przerwaniu tylko ustawienie flagi,
    // a przetwarzanie komend w while(1)

	// Commander_Process();
}

void MotorControl_SlowLoopMeasurementsISR(void)
{
	VoltageSense_ReadVbus();

	// Obsługa telemetrii i heatbeat dla CAN
}
