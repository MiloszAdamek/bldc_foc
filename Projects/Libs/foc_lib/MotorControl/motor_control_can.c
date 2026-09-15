/*
 * motor_control_can.c
 *
 *  Created on: Sep 13, 2025
 *      Author: Milosz Adamek
 */


#include "motor_control.h"
#include "math_consts.h"

extern volatile Motor_Telemetry_t g_telem;

extern volatile uint32_t g_heartbeat_period_ticks;
extern volatile uint32_t g_telemetry_period_ticks;
extern volatile bool g_can_heartbeat_flag;
extern volatile bool g_can_telemetry_flag;

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
    *faults = 0; 
    __set_PRIMASK(primask);
}