/*
 * motor_types.h
 *
 *  Created on: Aug 31, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_MOTOR_TYPES_H_
#define INC_MOTOR_TYPES_H_

typedef struct {
    float ia, ib, ic;
    float vdc;
    float theta_mech;
    float theta_el;
    float speed_rads;
} Motor_Measurements_t;

typedef struct {
    float torque_ref;
    float speed_ref;
    float position_ref;
} Motor_References_t;

// Wyjście z algorytmu (wypełnienie PWM)
typedef struct {
    float duty_a; // Zakres 0.0f - 1.0f
    float duty_b;
    float duty_c;
} Motor_Output_t;