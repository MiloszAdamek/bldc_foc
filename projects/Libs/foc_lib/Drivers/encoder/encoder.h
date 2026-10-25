/*
 * encoder.h
 *
 *  Created on: 3 Oct 2026
 *      Author: Milosz Adamek
 */

 #ifndef INC_FOC_ENCODER_H_
 #define INC_FOC_ENCODER_H_

 #include <stdint.h>
 #include <stdbool.h>
 #include "encoder_types.h"

 void Encoder_Init(void);
 void Encoder_PublishAngle(float theta_mech, float theta_el);
 bool Encoder_GetAngle(EncoderAngle_t *angle, float omega_rad_s);
 
#endif /* INC_FOC_ENCODER_H_ */