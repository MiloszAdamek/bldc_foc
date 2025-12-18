/*
 * transforms.h
 *
 *  Created on: Mar 30, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_CONTROLLER_UTILS_H_
#define INC_CONTROLLER_UTILS_H_

#include "stm32g4xx_hal.h"
#include <math.h>
#include <stdint.h>
#include "App/config.h"

#define ONE_OVER_SQRT_3 (1.0f / M_SQRT3)
#define ONE_OVER_TWO_PI (1.0f / (2 * M_TWOPI))
#define _SQRT3_2 		(M_SQRT3 / 2.0f)

// Parametry regulatorów PI
typedef struct {
    float kp;
    float ki;
    float integral;
    float limit;
    float dt;
} PI_Controller;

static inline float pi_control(PI_Controller *pi, float error){

    float u_p = pi->kp * error;
    pi->integral += pi->ki * error * pi->dt;
    float u = u_p + pi->integral;

    if (u > pi->limit) { u = pi->limit; pi->integral = u - u_p; }
    else if (u < -pi->limit) { u = -pi->limit; pi->integral = u - u_p; }

    return u;
}

// Transformacja Clarka,: 3 fazy (a,b,c)  αβ →
static inline void ClarkeTransform(float ia, float ib, float *ialpha, float *ibeta)
{
    *ialpha = ia;
    *ibeta  = (ia + 2.0f * ib) * ONE_OVER_SQRT_3;  // 1/sqrt(3) ≈ 0.577
}

// Odwrotna transformacja Clarka,: αβ → (a,b,c)
static inline void InvClarkeTransform(float ialpha, float ibeta, float *ia, float *ib, float *ic)
{
    *ia = ialpha;
    *ib = -0.5f * ialpha - _SQRT3_2 * ibeta;
    *ic = -0.5f * ialpha + _SQRT3_2 * ibeta;
}

//Transformacja Parka: αβ → dq (theta w radianach)
static inline void ParkTransform(float ialpha, float ibeta, float *sin_theta, float *cos_theta, float *id, float *iq)
{
    *id =  ialpha * (*cos_theta) + ibeta * (*sin_theta);
    *iq = -ialpha * (*sin_theta) + ibeta * (*cos_theta);
}

// Odwrotna transformacja parka: dq → αβ (theta w radianach)
static inline void InvParkTransform(float vd, float vq, float *sin_theta, float *cos_theta, float *valpha, float *vbeta)
{
    *valpha = vd * (*cos_theta) - vq * (*sin_theta);
    *vbeta  = vd * (*sin_theta) + vq * (*cos_theta);
}

// Normalizacja kąta do zakresu (0, 2pi)
static inline float normalize_angle(float a)
{
    while (a >= M_TWOPI) a -= M_TWOPI;
    while (a < 0.0f)    a += M_TWOPI;
    return a;
}

// Normalizuje kąt do zakresu (-PI, PI]
static inline float wrap_pi(float x){
    x = fmodf(x + M_PI, M_TWOPI);
    return (x < 0) ? x + M_TWOPI - M_PI : x - M_PI;
}

#endif /* INC_CONTROLLER_UTILS_H_ */
