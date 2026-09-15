/*
 * motor_control.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include "board.h"
#include "motor_types.h"

extern BoardHandleTypeDef board;

void MotorControl_Init(BoardHandleTypeDef* p_board);

void MotorControl_SetPosition(float position);

void MotorControl_SetSpeed(float rpm);

void MotorControl_SetTorque_Iq(float iq);

void MotorControl_SetTorque_mNm(float torque_mNm);

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
void MotorControl_SlowLoopMeasurementsISR(void);
void MotorControl_OnCANISR(void);

void MotorControl_GetCANTelemetry(float *pos_rev, float *vel_rpm);
void MotorControl_GetCANHeartbeat(uint16_t *state, uint16_t *faults);

#endif /* INC_MOTOR_CONTROL_H_ */
