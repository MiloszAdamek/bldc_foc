/*
 * lut_math.h
 *
 *  Created on: Nov 25, 2025
 *      Author: Miloush
 */

#ifndef INC_FOC_LUT_SINCOS_H_
#define INC_FOC_LUT_SINCOS_H_

#include <stdint.h>

#define LUT_SIZE        1024u
#define LUT_MASK        (LUT_SIZE - 1u)

// Skalowanie: index = angle * (LUT_SIZE / TWO_PI)
#define LUT_SCALE       ((float)LUT_SIZE / M_TWOPI)

extern const float sin_lut[LUT_SIZE];

float LUT_Sin(float angle_rad);
float LUT_Cos(float angle_rad);
void  LUT_SinCos(float angle_rad, float *s, float *c);

#endif /* INC_FOC_LUT_SINCOS_H_ */
