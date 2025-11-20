/*
 * controller_utils.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Miloush
 */

#include "FOC/controller_utils.h"

float pi_control(PI_Controller *pi, float error){

    float u_p = pi->kp * error;
    pi->integral += pi->ki * error * pi->dt;

    float u = u_p + pi->integral;
    if (u > pi->limit) { u = pi->limit; pi->integral = u - u_p; }
    else if (u < -pi->limit) { u = -pi->limit; pi->integral = u - u_p; }

    return u;
}
