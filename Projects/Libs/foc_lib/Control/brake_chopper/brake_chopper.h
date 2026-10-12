/*
 * brake_chopper.h
 *
 *  Created on: Oct 11, 2026
 *      Author: Milosz Adamek
 */

#ifndef BRAKE_CHOPPER_H_
#define BRAKE_CHOPPER_H_

#include "motor_types.h"

typedef struct
{
    bool  running;

    float duty;          // aktualne duty
    float integ;         // integrator PI

    float p_inst;        // chwilowa moc (W) wynikająca z duty i Vbus
    float p_heat;        // filtrowana moc (W) - proxy nagrzania
    float overload_time; // czas przebywania powyżej P_TRIP

    bool  latched;       // latch przeciążenia rezystora

} BrakeChopperState_t;

void  BrakeChopper_Init(void);
void  BrakeChopper_Start(void);
void  BrakeChopper_Stop(void);

void  BrakeChopper_ClearLatch(void);          // wywołaj np. w ClearFaults()
bool  BrakeChopper_IsLatched(void);

void  BrakeChopper_SetDutyCycle(float duty);  // zwykle nie wołasz ręcznie (robi to Update)
float BrakeChopper_GetDuty(void);
float BrakeChopper_GetPowerHeat(void);        // “proxy temperatury”
float BrakeChopper_GetPowerInstant(void);

void  BrakeChopper_Update(Motor_Measurements_t *meas, float dt);

#endif /* BRAKE_CHOPPER_H_ */
