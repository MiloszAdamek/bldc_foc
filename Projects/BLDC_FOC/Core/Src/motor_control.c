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

// w motor_control.c
#include "motor_control.h"
#include "foc_loop.h"
#include "as5048a.h"

volatile MotorState_t g_motor_state = STATE_IDLE;
static PI_Controller pi_speed = { .kp = PI_KP_V, .ki = PI_KI_V, .limit = PI_LIMIT_V, .integral = 0.0f };
static float target_speed_rpm = 0.0f;
static float actual_speed_rpm = 0.0f;
static float target_torque_iq = 0.0f;

void MotorControl_Run_1ms(void)
{
    // 1. Oblicz aktualną prędkość (przykładowa implementacja)
    //    (Wymaga globalnej zmiennej 'theta_el_last' i czasu próbkowania)
    static float last_angle = 0.0f;
    const float SAMPLING_TIME_SEC = 0.001f;

    // Użyj funkcji z foc_loop.h do przeliczenia kąta mechanicznego
    float current_mech_angle = AS5048_GetMechanicalAngle();

    // Proste filtrowane obliczenie prędkości
    float speed_rad_s = (current_mech_angle - last_angle) / SAMPLING_TIME_SEC;
    last_angle = current_mech_angle;
    actual_speed_rpm = speed_rad_s * (60.0f / M_TWOPI);

    switch (g_motor_state)
    {
        case STATE_IDLE:
            // Czekaj na polecenie startu
            // FOC_Stop() (wyłączenie PWM) powinno być wywołane przy przejściu DO tego stanu
            break;

        case STATE_ALIGNMENT:
            // Ten stan jest specjalny, zazwyczaj jest blokujący
            // lub obsługiwany przy starcie
            break;

        case STATE_TORQUE_CONTROL:
            // W tym trybie wolna pętla nie robi nic
            // FOC_SetIqTarget() jest wywoływane bezpośrednio z zewnątrz
            FOC_SetIqTarget(target_torque_iq);
            break;

        case STATE_SPEED_CONTROL:
            // To jest nasza nowa pętla regulacji prędkości
            float speed_error = target_speed_rpm - actual_speed_rpm;
            float iq_from_speed_pi = pi_control(&pi_speed, speed_error);

            // Wyjście z PI prędkości staje się wejściem do PI prądu
            FOC_SetIqTarget(iq_from_speed_pi);
            break;

        case STATE_FAULT:
            // Nic nie rób, czekaj na reset błędu
            // FOC_Stop() powinno być wywołane przy przejściu DO tego stanu
            break;
    }
}

// --- Publiczne API dla main.c ---

void MotorControl_SetMode_Speed(float rpm) {
    target_speed_rpm = rpm;
    g_motor_state = STATE_SPEED_CONTROL;
}

void MotorControl_SetMode_Torque(float iq) {
    target_torque_iq = iq; // Zapisz cel
    g_motor_state = STATE_TORQUE_CONTROL;
}

//void MotorControl_Start(void) {
//    if (g_motor_state == STATE_IDLE) {
//        g_motor_state = STATE_ALIGNMENT;
//
//        // Uruchom blokującą kalibrację
//        if (FOC_AlignSensor()) { // Zakładamy, że FOC_AlignSensor() zwraca bool
//            FOC_Start(); // Włącz PWM/ADC
//            MotorControl_SetMode_Torque(0.0f); // Przejdź do trybu momentu z zerowym prądem
//        } else {
//            g_motor_state = STATE_FAULT; // Błąd kalibracji
//        }
//    }
//}
