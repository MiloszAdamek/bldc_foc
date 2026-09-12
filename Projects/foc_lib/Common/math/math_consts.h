/*
 * math_consts.h
 *
 *  Created on: Sep 1, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_MATH_CONSTS_H_
#define INC_MATH_CONSTS_H_

#include <math.h>

// Math constants
#define ONE_OVER_SQRT_3 (1.0f / M_SQRT3)
#define ONE_OVER_TWO_PI (1.0f / (2 * M_TWOPI))
#define _SQRT3_2 		(M_SQRT3 / 2.0f)
#define _3PI_2          4.71238898038f
#define _PI_3           (M_PI / 3.0f)
#define TWO_PI          (2.0f * (float)M_PI)

// Conversion constants
#define RPM_TO_RAD_S      (2.0f * M_PI / 60.0f)
#define RAD_S_TO_RPM      (60.0f / (2.0f * M_PI))
#define RAD_TO_DEG        (360.0f / (2.0f * M_PI))
#define DEG_TO_RAD        (2.0f * M_PI / 360.0f)

#endif /* INC_MATH_CONSTS_H_ */