/*
 * controller_utils.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Miloush
 */

#include "FOC/controller_utils.h"

float pi_control(PI_Controller *pi, float error, float dt){

    float u_p = pi->kp * error;
    pi->integral += pi->ki * error * dt;

    float u = u_p + pi->integral;
    if (u > pi->limit) { u = pi->limit; pi->integral = u - u_p; }
    else if (u < -pi->limit) { u = -pi->limit; pi->integral = u - u_p; }

    return u;
}

//static inline float pi_control(PI_Controller *pi, float error)
//{
//    pi->integral += error * pi->ki * PWM_PERIOD_SEC; // Ts = 50 us
//
//    if (pi->integral > pi->limit) pi->integral = pi->limit;
//    else if (pi->integral < -pi->limit) pi->integral = -pi->limit;
//
//    float output = pi->kp * error + pi->integral;
//
//    if (output > pi->limit) output = pi->limit;
//    else if (output < -pi->limit) output = -pi->limit;
//
//    return output;
//}
