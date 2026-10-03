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
void EncoderIncremental_Update();
void EncoderIncremental_GetSample();
void EncoderIncremental_SetZero();

float EncoderIncremental_GetMechanicalAngle(void);

#endif /* INC_FOC_ENCODER_INCREMENTAL_H_ */
