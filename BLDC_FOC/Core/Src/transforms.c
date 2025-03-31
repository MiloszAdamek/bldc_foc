/*
 * transforms.c
 *
 *  Created on: Mar 30, 2025
 *      Author: Miloush
 */

#include "transforms.h"
#include <math.h>

void clarke_transform(float Ia, float Ib, float *Valpha, float *Vbeta) {
    *Valpha = Ia;
    *Vbeta = (Ia + 2 * Ib) / sqrtf(3.0f);
}

void park_transform(float Valpha, float Vbeta, float theta, float *Vd, float *Vq) {
    *Vd = Valpha * cosf(theta) + Vbeta * sinf(theta);
    *Vq = -Valpha * sinf(theta) + Vbeta * cosf(theta);
}
