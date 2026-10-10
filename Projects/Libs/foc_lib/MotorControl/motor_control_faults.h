/*
 * motor_control_faults.h
 *
 *  Created on: Oct 10, 2025
 *      Author: Milosz Adamek
 */

#ifndef MOTOR_CONTROL_FAULTS_H
#define MOTOR_CONTROL_FAULTS_H

#include <stdint.h>
#include "motor_types.h"

typedef uint16_t MotorFaults_t;

#define MOTOR_FAULT_NONE                    0U

/* Bit 0–4: power stage */
#define MOTOR_FAULT_DRIVER                  (1UL << 0)
#define MOTOR_FAULT_DRIVER_GATE             (1UL << 1)
#define MOTOR_FAULT_DRIVER_OVERCURRENT      (1UL << 2)
#define MOTOR_FAULT_DRIVER_OVERTEMP         (1UL << 3)
#define MOTOR_FAULT_DRIVER_UNDERVOLTAGE     (1UL << 4)

/* Bit 5–12: control loop */
#define MOTOR_FAULT_OVERCURRENT             (1UL << 5)
#define MOTOR_FAULT_MOTOR_OVERTEMP          (1UL << 6)
#define MOTOR_FAULT_MOSFET_OVERTEMP         (1UL << 7)
#define MOTOR_FAULT_VBUS_OVERVOLTAGE        (1UL << 8)
#define MOTOR_FAULT_VBUS_UNDERVOLTAGE       (1UL << 9)
#define MOTOR_FAULT_ENCODER                 (1UL << 10)
#define MOTOR_FAULT_CONTROL                 (1UL << 11)
#define MOTOR_FAULT_INVALID_MEASUREMENTS    (1UL << 12)

typedef struct
{
    MotorFaults_t mask;
    const char *message;
} MotorFaultDescription_t;

static const MotorFaultDescription_t fault_descriptions[] =
{
    { MOTOR_FAULT_DRIVER,                "Driver fault" },
    { MOTOR_FAULT_DRIVER_GATE,           "Driver gate fault" },
    { MOTOR_FAULT_DRIVER_OVERCURRENT,    "Driver overcurrent" },
    { MOTOR_FAULT_DRIVER_OVERTEMP,       "Driver overtemperature" },
    { MOTOR_FAULT_DRIVER_UNDERVOLTAGE,   "Driver undervoltage" },
    { MOTOR_FAULT_OVERCURRENT,           "Motor overcurrent" },
    { MOTOR_FAULT_MOTOR_OVERTEMP,        "Motor overtemperature" },
    { MOTOR_FAULT_MOSFET_OVERTEMP,       "MOSFET overtemperature" },
    { MOTOR_FAULT_VBUS_OVERVOLTAGE,      "VBUS overvoltage" },
    { MOTOR_FAULT_VBUS_UNDERVOLTAGE,     "VBUS undervoltage" },
    { MOTOR_FAULT_ENCODER,              "Encoder fault" },
    { MOTOR_FAULT_CONTROL,              "Control loop fault" },
    { MOTOR_FAULT_INVALID_MEASUREMENTS, "Invalid measurements" },
};

/* Registry operations */
#define MOTOR_FAULT_SET(reg, fault) \
    ((reg) |= (fault))

#define MOTOR_FAULT_CLEAR(reg, fault) \
    ((reg) &= ~(fault))

#define MOTOR_FAULT_IS_SET(reg, fault) \
    (((reg) & (fault)) != 0U)

void MotorControl_ProcessControlFaults(Motor_Measurements_t *meas);
void MotorControl_ProcessDriverFaults(void);
void MotorControl_PrintFaults(MotorFaults_t faults);
void MotorControl_OnFaultDetected(void);

#endif /* MOTOR_CONTROL_FAULTS_H */