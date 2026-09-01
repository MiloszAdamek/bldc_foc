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
#include "svpwm.h"
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

    SensorCalib_t  calib;
    Ramp_t         ramp;
    FocFlags_t     flags;
    FocAngles_t    angles;
    FocStats_t	   stats;
} FOC_HandleTypeDef;

// Flagi
extern volatile bool currents_ready;

void FOC_Init(BoardHandleTypeDef *board);
void FOC_RunLoop(void); // Główna pętla FOC
void FOC_Update(float theta_el);
void FOC_SetIqTarget(float new_target);
float FOC_GetElecticalAngle(float mech);
void FOC_SetIqTarget_Ramp(float new_target);
void FOC_SetTorqueTarget(float torque_mNm);
bool FOC_AlignSensor(void); // Kalibracja enkodera

void FOC_Stop(void); // Zatrzymanie PWM, wyłączenie driverów, zatrzymanie ADC
void FOC_Start(void);

bool FOC_IsSensorAligned(void);

void Motor_Motion_Test(void);

#endif /* INC_FOC_LOOP_H_ */
