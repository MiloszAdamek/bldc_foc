/*
 * motor_control.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Miloush
 */

#include "motor_control.h"
#include "foc_loop.h"
#include "as5048a.h"
#include "config.h"
#include "commander.h"

static TIM_HandleTypeDef* ctrl_htim;
static TIM_HandleTypeDef* cmd_htim;

volatile MotorState_t g_motor_state = STATE_IDLE;
static PI_Controller pi_speed = { .kp = PI_KP_V, .ki = PI_KI_V, .limit = PI_LIMIT_V, .integral = 0.0f };
static float target_speed_rpm = 0.0f;
static float actual_speed_rpm = 0.0f;
static float target_torque_iq = 0.0f;

void MotorControl_Init(TIM_HandleTypeDef* control_htim, TIM_HandleTypeDef* commander_htim){
	ctrl_htim = control_htim;
	cmd_htim = commander_htim;
	HAL_TIM_Base_Start_IT(ctrl_htim);
	HAL_TIM_Base_Start_IT(cmd_htim);
	MotorControl_Start();
}

void MotorControl_SpeedController(float speed_error){
    float iq_from_speed_pi = pi_control(&pi_speed, speed_error, FSM_PERIOD_SEC);
    // Wyjście z PI prędkości staje się wejściem do PI prądu
    FOC_SetIqTarget(iq_from_speed_pi);
}

void MotorControl_Run(void)
{
	if (g_motor_state >= STATE_TORQUE_CONTROL) {
		actual_speed_rpm = MotorControl_GetActualSpeed();
	} else {
		actual_speed_rpm = 0.0f;
	}

    switch (g_motor_state)
    {
        case STATE_IDLE:
            break;

        case STATE_ALIGNMENT:
            break;

        case STATE_TORQUE_CONTROL:
            break;

        case STATE_SPEED_CONTROL:
            // Pętla regulacji prędkości
            float speed_error = target_speed_rpm - actual_speed_rpm;
            MotorControl_SpeedController(speed_error);
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
    pi_speed.integral = 0.0f; // Zerowanie przy przejsciu w tryb predkosci, by uniknac nagłego skoku
    g_motor_state = STATE_SPEED_CONTROL;
}

void MotorControl_SetTorque(float iq) {
    target_torque_iq = iq; // Zapisz cel
    g_motor_state = STATE_TORQUE_CONTROL;

    FOC_SetIqTarget_Ramp(iq); // Aktywacja rampy tylko raz
}

float MotorControl_GetActualSpeed(){
	static float last_mech_angle = 0.0f;
	static float speed_lpf = 0.0f; // Filtrowana prędkość (LPF)

	float current_mech_angle = AS5048_GetMechanicalAngle();

	float delta_angle = wrap_pi(current_mech_angle - last_mech_angle);

	float speed_rad_s_raw = delta_angle / FSM_PERIOD_SEC;

	// Zastosuj filtr dolnoprzepustowy (LPF)
	// Współczynnik 0.1f możesz dostroić; mniejszy = bardziej gładko, wolniej
	speed_lpf = speed_lpf * 0.9f + speed_rad_s_raw * 0.1f;

	last_mech_angle = current_mech_angle;

	actual_speed_rpm = speed_lpf * (60.0f / M_TWOPI);

	return actual_speed_rpm;
}

void MotorControl_Reboot(){
	HAL_NVIC_SystemReset();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == ctrl_htim->Instance) // pętla 1 kHz
	{
		MotorControl_Run();
	}
	if (htim->Instance == cmd_htim->Instance) // pętla 100 Hz
	{
		Commander_Process();
	}
}
