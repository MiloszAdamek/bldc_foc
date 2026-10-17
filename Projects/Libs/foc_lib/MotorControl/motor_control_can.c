/*
 * motor_control_can.c
 *
 *  Created on: Sep 13, 2025
 *      Author: Milosz Adamek
 */


#include "motor_control.h"
#include "math_consts.h"
#include "motor_control_faults.h"

extern volatile Motor_Telemetry_t g_telem;

extern volatile MotorFaults_t g_motor_faults;

extern volatile MotorState_t g_motor_state;

extern volatile uint32_t g_heartbeat_period_ticks;
extern volatile uint32_t g_telemetry_period_ticks;
extern volatile bool g_can_heartbeat_flag;
extern volatile bool g_can_telemetry_flag;

extern volatile uint32_t g_comm_timeout_cnt;
extern uint32_t watchdog_timeout_ms;
extern volatile bool g_watchdog_enabled;

void MotorControl_OnCANISR(void)
{
    static uint16_t heartbeat_counter = 0;
    static uint16_t telemetry_counter = 0;

    // Heartbeat
    if (g_heartbeat_period_ticks > 0)
        {
            heartbeat_counter++;
            if (heartbeat_counter >= g_heartbeat_period_ticks)
            {
                heartbeat_counter = 0;
                g_can_heartbeat_flag = true;
            }
        }
    // Telemetria
    if (g_telemetry_period_ticks > 0)
    {
        telemetry_counter++;
        if (telemetry_counter >= g_telemetry_period_ticks)
        {
            telemetry_counter = 0;
            g_can_telemetry_flag = true;
        }
    }
}

// Powinno być przekazywane przez wartość, a nie przez wskaźnik - do 4 bajtów jest szybciej, przez rejestry procesoras
void MotorControl_GetCANTelemetry(float *pos_rev, float *vel_rpm)
{
    float temp_pos_rad;
    float temp_vel_rpm;

    // Wejście w sekcję krytyczną - blokada przerwań na czas kopiowania
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    temp_pos_rad = g_telem.theta_mech;
    temp_vel_rpm = g_telem.omega_mech_rpm;

    // Wyjście z sekcji krytycznej
    __set_PRIMASK(primask);

    // Konwersja z radianów na obroty (0.0 - 1.0)
    *pos_rev = temp_pos_rad / TWO_PI; 
    *vel_rpm = temp_vel_rpm;
}

void MotorControl_GetCANHeartbeat(uint16_t *state, uint16_t *faults)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    *state = g_telem.motor_state;
    // Jeśli masz mechanizm błędów, dodaj odczyt faults
    *faults = g_motor_faults; 
    __set_PRIMASK(primask);
}

void MotorControl_SetWatchdog(uint16_t timeout_ms)
{
    if (timeout_ms == 0) {
        // Przesłanie 0 ms wyłącza watchdoga
        g_watchdog_enabled = false;
        g_comm_timeout_cnt = 0;
    } else {
        // Najpierw zerujemy licznik i ustawiamy próg, potem uzbrajamy 
        g_comm_timeout_cnt  = 0;
        watchdog_timeout_ms = (uint32_t)timeout_ms;
        __DMB();
        g_watchdog_enabled  = true;
    }
}

void MotorControl_WatchdogCAN(void)
{
    if (!g_watchdog_enabled) {
        return;
    }

    if (g_motor_state == STATE_RUN) {
        g_comm_timeout_cnt++;
        if (g_comm_timeout_cnt > watchdog_timeout_ms) {
            g_motor_state = STATE_FAULT;
            MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_COMM_TIMEOUT);
            MotorControl_Stop();

        }
    } else {
        g_comm_timeout_cnt = 0;
    }
}