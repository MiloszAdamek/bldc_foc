/*
 * motor_algorithm.h
 *
 *  Created on: Aug 31, 2026
 *      Author: Milosz Adamek
 */

#ifndef INC_MOTOR_ALGORITHM_H_
#define INC_MOTOR_ALGORITHM_H_

#include "motor_types.h"

typedef struct {
    // Wskaźnik na prywatną strukturę danych algorytmu (FOC_State_t, MPC_State_t itp.)
    void *ctx; 
    
    void (*Init)(void *ctx);
    void (*Start)(void *ctx);
    void (*Stop)(void *ctx);
    void (*Update)(void *ctx, const Motor_Measurements_t *meas, const Motor_References_t *ref, Motor_Output_t *out);
    void (*GetTelemetry)(const void *ctx, Motor_Telemetry_t *telem);
} ControlAlgorithm_t;

#endif /* INC_MOTOR_ALGORITHM_H_ */