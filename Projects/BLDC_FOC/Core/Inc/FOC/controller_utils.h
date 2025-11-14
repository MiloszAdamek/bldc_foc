/*
 * transforms.h
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#ifndef INC_CONTROLLER_UTILS_H_
#define INC_CONTROLLER_UTILS_H_

#include "stm32g4xx_hal.h"
#include <math.h>
#include <stdint.h>
#include "App/config.h"

#define ONE_OVER_SQRT_3 (1.0f / M_SQRT3)

// Parametry regulatorów PI
typedef struct {
    float kp;
    float ki;
    float integral;
    float limit;
    float dt;
} PI_Controller;

float pi_control(PI_Controller *pi, float error);

static inline float normalize_angle(float angle) {
    float result = fmodf(angle, M_TWOPI);
    return result >= 0.0f ? result : result + M_TWOPI;
}

// Clarke transform: 3 fazy → αβ (z pomiarów Ia, Ib)
static inline void ClarkeTransform(float ia, float ib, float *ialpha, float *ibeta)
{
    *ialpha = ia;
    *ibeta  = (ia + 2.0f * ib) * ONE_OVER_SQRT_3;  // 1/sqrt(3) ≈ 0.577
}

// Park transform: αβ → dq (theta w radianach)
static inline void ParkTransform(float ialpha, float ibeta, float theta, float *id, float *iq)
{
    float sin_theta = sinf(theta);
    float cos_theta = cosf(theta);

    *id =  ialpha * cos_theta + ibeta * sin_theta;
    *iq = -ialpha * sin_theta + ibeta * cos_theta;
}

// Inverse Park transform: dq → αβ (theta w radianach)
static inline void InvParkTransform(float vd, float vq, float theta, float *valpha, float *vbeta)
{
    float sin_theta = sinf(theta);
    float cos_theta = cosf(theta);

    *valpha = vd * cos_theta - vq * sin_theta;
    *vbeta  = vd * sin_theta + vq * cos_theta;
}

static inline void ParkTransformTrig(float ialpha, float ibeta, float *sin_theta, float *cos_theta, float *id, float *iq)
{
    *id =  ialpha * (*cos_theta) + ibeta * (*sin_theta);
    *iq = -ialpha * (*sin_theta) + ibeta * (*cos_theta);
}

static inline void InvParkTransformTrig(float vd, float vq, float *sin_theta, float *cos_theta, float *valpha, float *vbeta)
{
    *valpha = vd * (*cos_theta) - vq * (*sin_theta);
    *vbeta  = vd * (*sin_theta) + vq * (*cos_theta);
}

// Normalizuje kąt do zakresu (-PI, PI]
static inline float wrap_pi(float x){
    x = fmodf(x + M_PI, M_TWOPI);
    return (x < 0) ? x + M_TWOPI - M_PI : x - M_PI;
}

static inline float wrap_pi_dtheta(float x)
{
    if (x >  M_PI) x -= M_TWOPI;
    if (x < -M_PI) x += M_TWOPI;
    return x;
}

#endif /* INC_CONTROLLER_UTILS_H_ */
