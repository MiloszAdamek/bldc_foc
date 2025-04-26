/*
 * transforms.c
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#include "transforms.h"
#include <math.h>

void ClarkeTransform(float ia, float ib, float *ialpha, float *ibeta)
{
    *ialpha = ia;
    *ibeta  = (ia + 2.0f * ib) * 0.57735026919f;  // 1/sqrt(3) ≈ 0.577
}

void ParkTransform(float ialpha, float ibeta, float theta, float *id, float *iq)
{
    float sin_theta = sinf(theta);
    float cos_theta = cosf(theta);

    *id =  ialpha * cos_theta + ibeta * sin_theta;
    *iq = -ialpha * sin_theta + ibeta * cos_theta;
}

void InvParkTransform(float vd, float vq, float theta, float *valpha, float *vbeta)
{
    float sin_theta = sinf(theta);
    float cos_theta = cosf(theta);

    *valpha = vd * cos_theta - vq * sin_theta;
    *vbeta  = vd * sin_theta + vq * cos_theta;
}
