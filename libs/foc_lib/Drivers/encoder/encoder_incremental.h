/*
 * encoder_incremental.h
 *
 *  Created on: 3 Oct 2026
 *      Author: Milosz Adamek
 */

 #ifndef INC_FOC_ENCODER_INCREMENTAL_H_
 #define INC_FOC_ENCODER_INCREMENTAL_H_

 #include "tim.h"
 
void EncoderIncremental_Init(TIM_HandleTypeDef* htim);
float EncoderIncremental_GetMechanicalAngle(void);
void EncoderIncremental_SetZero();

#endif /* INC_FOC_ENCODER_INCREMENTAL_H_ */
