/*
 * motor_control.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include "tim.h"

typedef enum {
    STATE_IDLE,         	// Czeka na polecenia, PWM wyłączone
    STATE_ALIGNMENT,    	// Trwa kalibracja (FOC_AlignSensor)
    STATE_TORQUE_CONTROL, 	// Tryb regulacji momentu
    STATE_SPEED_CONTROL,  	// Tryb regulacji prędkości
    STATE_FAULT         	// Błąd krytyczny
} MotorState_t;

extern volatile MotorState_t g_motor_state;

void MotorControl_Init(TIM_HandleTypeDef* control_htim, TIM_HandleTypeDef* commander_htim);

void MotorControl_SetSpeed(float rpm);

void MotorControl_SetTorque(float iq);

float MotorControl_GetActualSpeed();

void MotorControl_Start(void);

void MotorControl_Stop(void);

void MotorControl_Reboot(void);

#endif /* INC_MOTOR_CONTROL_H_ */
