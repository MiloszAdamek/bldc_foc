/*
 * foc_loop.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_FOC_LOOP_H_
#define INC_FOC_LOOP_H_

#include "stm32g4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include "math.h"
#include "foc_utils.h"
#include "motor_types.h"
#include "motor_algorithm.h"
#include "current_sense.h"
#include "as5048a.h"
#include "board.h"

extern volatile MonitorData_t monitor_data;

/* -------- Wartość zadana regulatora -------- */
typedef struct {
    float d;
    float q;
} dq_ref_t;

/* -------- Stan kalibracji -------- */
typedef struct {
    int   direction;          // 1=CW, -1=CCW
    float zero_electric_angle;
    bool  aligned;
} SensorCalib_t;

/* -------- Stan rampy -------- */
typedef struct {
    float output;
    float step;
    bool  active;
} Ramp_t;

/* -------- Flagi synchronizacji -------- */
typedef struct {
    volatile bool new_current;
    volatile bool spi_angle;
    volatile bool encoder_prev;
} FocFlags_t;

/* -------- Kąty -------- */
typedef struct {
    volatile float theta_mech;
    volatile float theta_el;
} FocAngles_t;

/* -------- Statystyki -------- */
typedef struct {
    volatile uint32_t loop_ok;
    volatile uint32_t loop_err;
    volatile uint32_t err_current;
    volatile uint32_t encoder_stale;     // ile razy użyto starego kąta
    volatile uint32_t encoder_timeout;   // ile razy z rzędu brak nowej próbki
} FocStats_t;

/* -------- Główny kontekst FOC -------- */
typedef struct {
    BoardHandleTypeDef *board;

    PI_Controller  pi_id;
    PI_Controller  pi_iq;
    dq_ref_t       i_ref;
    abc_current_t  currents;

    float           v_bus;
    float           v_alpha, v_beta;
    float           v_d, v_q;
    float           id, iq;
    float           i_alpha, i_beta;

    Ramp_t         ramp;
    FocFlags_t     flags;
    FocAngles_t    angles;
    FocStats_t	   stats;
} PI_FOC_State_t;

extern PI_FOC_State_t s_foc;

// Flagi
extern volatile bool currents_ready;

ControlAlgorithm_t PI_FOC_Create(void);

void FOC_Init(void *ctx);
// void FOC_RunLoop(void); // Główna pętla FOC
// void FOC_Update(void *ctx, const Motor_Measurements_t *meas, const Motor_References_t *ref, Motor_Output_t *out);
void FOC_Stop(void *ctx); // Zatrzymanie PWM, wyłączenie driverów, zatrzymanie ADC
void FOC_Start(void *ctx);

float FOC_GetElectricalAngle(float mech);

void FOC_SetIqTarget(float new_target);
void FOC_SetIqTarget_Ramp(float new_target);
void FOC_SetTorqueTarget(float torque_mNm);

bool FOC_AlignSensor(void); // Kalibracja enkodera
bool FOC_IsSensorAligned(void);

void Motor_Motion_Test(void);

#endif /* INC_FOC_LOOP_H_ */
