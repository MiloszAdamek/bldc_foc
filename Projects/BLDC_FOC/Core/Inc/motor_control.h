/*
 * motor_control.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Miloush
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

typedef enum {
    STATE_IDLE,         	// Czeka na polecenia, PWM wyłączone
    STATE_ALIGNMENT,    	// Trwa kalibracja (FOC_AlignSensor)
    STATE_TORQUE_CONTROL, 	// Tryb regulacji momentu
    STATE_SPEED_CONTROL,  	// Tryb regulacji prędkości
    STATE_FAULT         	// Błąd krytyczny
} MotorState_t;

extern volatile MotorState_t g_motor_state;

void MotorControl_SetMode_Speed(float rpm);

void MotorControl_SetMode_Torque(float iq);

void MotorControl_Start(void);

void MotorControl_Stop(void);

#endif /* INC_MOTOR_CONTROL_H_ */
