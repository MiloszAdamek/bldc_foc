/*
 * motor_control.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include "board.h"

typedef enum {
    STATE_IDLE,         	// Czeka na polecenia, PWM wyłączone
    STATE_ALIGNMENT,    	// Trwa kalibracja (FOC_AlignSensor)
    STATE_TORQUE_CONTROL, 	// Tryb regulacji momentu
	STATE_RUN,
    STATE_SPEED_CONTROL,  	// Tryb regulacji prędkości
	STATE_POSITION_CONTROL, // Tryb regulacji pozycji
    STATE_FAULT         	// Błąd krytyczny
} MotorState_t;

extern BoardHandleTypeDef board;

extern volatile MotorState_t g_motor_state;

void MotorControl_Init(BoardHandleTypeDef* p_board);

void MotorControl_SetPosition(float position);

void MotorControl_SetSpeed(float rpm);

void MotorControl_SetTorque(float iq);

float MotorControl_GetActualSpeed();

void MotorControl_Start(void);

void MotorControl_Stop(void);

void MotorControl_Reboot(void);

void MotorControl_SetState(MotorState_t new_state);

// Motor control callbacks for ISRs
void MotorControl_OnCurrentSampleISR(void);
void MotorControl_OnEncoderSampleISR(void);
void MotorControl_OnSpeedISR(void);
void MotorControl_OnPositionISR(void);
void MotorControl_OnCommandISR(void);

#endif /* INC_MOTOR_CONTROL_H_ */
