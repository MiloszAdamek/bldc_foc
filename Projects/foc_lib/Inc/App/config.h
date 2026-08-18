/*
 * motor_config.h
 *
 *  Created on: Nov 7, 2025
 *      Author: Milosz Adamek
 */

#ifndef MOTOR_CONFIG_H_
#define MOTOR_CONFIG_H_

#include "stm32g4xx_hal.h"
#include <math.h>

/* BOARD */ 

#define DRV8353
// #define G431_ESC
// #define IHM03

/* PWM MODE */ 

#define MODE_3PWM
//#define MODE_6PWM

/* CURRENT SENSE MODE */ 

// #define CURRENT_SENSE_DOUBLE_SHUNT
#define CURRENT_SENSE_TRIPLE_SHUNT


#define ENABLE_SERIAL_DEBUGGING
//#define ENABLE_RAMP
//#define TEST_MODE

/* =========================================================================
 * ----------------   PARAMETRY OGÓLNE SYSTEMU   ----------------------------
 * ========================================================================= */

#define PWM_FREQ_HZ			  20000.0f				// 20 kHz - PWM
#define PWM_PERIOD_SEC		  (1.0f / PWM_FREQ_HZ)
#define PWM_PERIOD_ARR        4249

#define FOC_FREQ_HZ      	  10000.0f              // 10 kHz - pętla algorytmu FOC
#define FOC_PERIOD_SEC        (1.0f / FOC_FREQ_HZ)

#define SPEED_FREQ_HZ		  1000.0f				// 1 kHz - pętla regulatora prędkości
#define SPEED_PERIOD_SEC      (1.0f / SPEED_FREQ_HZ)

#define POSITION_FREQ_HZ	  200.0f				// 200 Hz - pętla regulatora pozycji
#define POSITION_PERIOD_SEC   (1.0f / POSITION_FREQ_HZ)

// Napięcie zasilania i limity napięcia dla FOC
#define VOLTAGE_SUPPLY        13.0f
#define VOLTAGE_LIMIT         10.0f

// Rezystor pomiarowy i wzmocnienie
#define ADC_REF_VOLTAGE   	  3.3f       // Vref zasilania ADC
#define ADC_RESOLUTION        4096.0f    // 12-bit ADC

/* =========================================================================
 * ----------------  PARAMETRY SILNIKA I ENKODERA  -------------------------
 * ========================================================================= */

#define MOTOR_POLE_PAIRS      	7           // liczba par biegunów silnika
#define MOTOR_TORQUE_CONSTANT 	0.0306f		// Kt
#define MOTOR_VELOCITY_CONSTANT 168		    // Kv

#define VOLTAGE_SENSOR_ALIGN  	4.0f
#define ENCODER_RESOLUTION    	16384.0f    // enkoder AS5048A (14 bit)

#define SENSOR_DIRECTION_CW   1
#define SENSOR_DIRECTION_CCW -1

/* =========================================================================
 * ----------------  PARAMETRY REGULATORÓW PI  -----------------------------
 * ========================================================================= */

#define PI_KP_ID 3.0f
#define PI_KI_ID 300.0f
#define PI_LIMIT_ID (VOLTAGE_SUPPLY / M_SQRT3)

#define PI_KP_IQ 3.0f
#define PI_KI_IQ 300.0f
#define PI_LIMIT_IQ (VOLTAGE_SUPPLY / M_SQRT3)

#define PI_KP_V 0.003f
#define PI_KI_V 0.005f
#define PI_LIMIT_V 0.4f * (VOLTAGE_SUPPLY / M_SQRT3)

#define PI_KP_P 10.0f
#define PI_KI_P 2.0f
#define PI_LIMIT_P (1000.0f * 2.0f * M_PI / 60.0f) // 500 RPM -> rad/s

#ifdef ENABLE_SERIAL_DEBUGGING
    #define LOG(format, ...) printf(format, ##__VA_ARGS__)
#else
    #define LOG(format, ...) do {} while (0)
#endif

/* =========================================================================
 * ----------------  DEBUG I LOGOWANIE DANYCH  -----------------------------
 * ========================================================================= */

typedef struct {
    float current_a;
    float current_b;
    float current_c;
    float iq;
    float iq_ref;
    float id;
    float id_ref;
    float speed;
    float speed_ref;
    float theta_el;
    float theta_mech;
    float position_ref;
    float position_err;
    float position_reg_out;
} MonitorData_t;

#endif /* MOTOR_CONFIG_H_ */
