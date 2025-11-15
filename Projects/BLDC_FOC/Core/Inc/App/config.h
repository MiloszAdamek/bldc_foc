/*
 * motor_config.h
 *
 *  Created on: Nov 7, 2025
 *      Author: Miloush
 */

#ifndef MOTOR_CONFIG_H_
#define MOTOR_CONFIG_H_

#include "stm32g4xx_hal.h"
#include <math.h>

#define ENABLE_SERIAL_DEBUGGING
//#define TEST_MODE

/* =========================================================================
 * ----------------   PARAMETRY OGÓLNE SYSTEMU   ----------------------------
 * ========================================================================= */

// Częstotliwość PWM i odpowiadający okres [s]
#define PWM_FREQUENCY_HZ      10000.0f                     	// 10 kHz
#define PWM_PERIOD_SEC        (1.0f / PWM_FREQUENCY_HZ)

#define FSM_LOOP_HZ			  1000.0f
#define FSM_PERIOD_SEC		  (1.0f / FSM_LOOP_HZ)

// Wartość ARR timera (dla 170 MHz taktowania i prescalera = 0)
#define PWM_PERIOD_ARR        4250

// Napięcie zasilania i limity napięcia dla FOC
#define VOLTAGE_SUPPLY        12.0f
#define VOLTAGE_LIMIT         8.0f

// Rezystor pomiarowy i wzmocnienie
#define SHUNT_RESISTOR        0.33f // Ohm
#define CURRENT_SENSE_GAIN    1.528f

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
 * ----------------  PARAMETRY ALGORYTMU FOC  -------------------------------
 * ========================================================================= */

#define DEFAULT_IQ_TARGET     0.5f             // A – moment zadany przy starcie

#define PI_KP_ID 5.0f
#define PI_KI_ID 1000.0f
#define PI_LIMIT_ID (VOLTAGE_SUPPLY / M_SQRT3)

#define PI_KP_IQ 5.0f
#define PI_KI_IQ 1000.0f
#define PI_LIMIT_IQ (VOLTAGE_SUPPLY / M_SQRT3)

#define PI_KP_V 0.07f
#define PI_KI_V 0.03f
#define PI_LIMIT_V 2.0f

#ifdef ENABLE_SERIAL_DEBUGGING
    #define LOG(format, ...) printf(format, ##__VA_ARGS__)
#else
    #define LOG(format, ...) do {} while (0)
#endif

#endif /* MOTOR_CONFIG_H_ */
