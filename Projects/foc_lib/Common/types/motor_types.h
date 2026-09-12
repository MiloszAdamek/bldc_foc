/*
 * motor_types.h
 *
 *  Created on: Aug 31, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_MOTOR_TYPES_H_
#define INC_MOTOR_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

typedef struct {
    float a;
    float b;
    float c;
} abc_current_t;

typedef struct {
    uint16_t a;
    uint16_t b;
    uint16_t c;
} abc_raw_t;

// Pomiary dostarczane do algorytmu sterowania
typedef struct {
    abc_current_t currents; // Prądy fazowe (A)
    float v_bus;            // Napięcie Vbus (V)
    float theta_el;         // Kąt elektryczny (rad)
    float theta_mech;       // Kąt mechaniczny (rad)
    float omega_mech_rpm;   // Prędkość mechaniczna (rpm)
    float omega_mech_rad_s; // Prędkość mechaniczna (rad/s)
} Motor_Measurements_t;

typedef struct {
    float torque_iq_ref;
    float speed_ref;
    float position_ref;

    bool iq_ramp_enabled; // Flaga włączenia rampy dla prądu Iq (tryb regulacji momentu)
} Motor_References_t;

// Wyjście z algorytmu (wypełnienie PWM)
typedef struct {
    float duty_a; // Zakres 0.0f - 1.0f
    float duty_b;
    float duty_c;
} Motor_Output_t;

typedef struct {
    int   direction;          // 1=CW, -1=CCW
    float zero_electric_angle;
    bool  aligned;
} Motor_Calibration_t;

typedef struct {
    uint32_t loop_ok;
    uint32_t loop_err;
    uint32_t currents_err;
} Motor_Stats_t;

typedef struct {
    ADC_HandleTypeDef *hadc;
    uint32_t rank;            // Ranga w grupie Injected (np. ADC_INJECTED_RANK_1)
} ADC_InjectedChannel_t;

typedef struct {
    float id_meas;
    float id_ref;
    float iq_meas;
    float iq_ref;
    float vd_out;
    float vq_out;
    uint32_t loop_time_us; // Czas wykonania pojedynczego kroku
    uint8_t active_algo;   // 0 = None, 1 = PI_FOC, 2 = CCS_MPC
} Motor_Telemetry_t;

#endif /* INC_MOTOR_TYPES_H_ */