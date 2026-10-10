
#include "motor_control_faults.h"
#include "motor_control.h"
#include <stdio.h>

extern MotorState_t g_motor_state;
extern BoardHandleTypeDef board;

volatile bool g_drv_fault_pending = false;
volatile bool g_motor_stop_pending = false;
volatile bool g_break_event_latched = false;

volatile MotorFaults_t g_motor_faults = MOTOR_FAULT_NONE;

void MotorControl_ProcessControlFaults(Motor_Measurements_t *meas)
{
    if (meas == NULL)
    {
        g_motor_state = STATE_FAULT;
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_INVALID_MEASUREMENTS);
        return;
    }

    // Sprawdzenie temperatury silnika i mosfetów
    if (meas->motor_temp > MOTOR_MAX_TEMP_C)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_MOTOR_OVERTEMP);
    }

    if (meas->mosfet_temp > MOSFET_MAX_TEMP_C)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_MOSFET_OVERTEMP);
    }

    // Sprawdzenie napięcia zasilania
    if (meas->v_bus > OVERVOLTAGE_TRESHOLD)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_VBUS_OVERVOLTAGE);
    }
    else if (meas->v_bus < UNDERVOLTAGE_TRESHOLD)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_VBUS_UNDERVOLTAGE);
    }

    if(g_motor_faults != MOTOR_FAULT_NONE)
    {
        g_motor_state = STATE_FAULT;
        MotorControl_Stop();
    }
}

void MotorControl_ProcessDriverFaults()
{   
    if (!g_drv_fault_pending)
    {
        return;
    }

    g_drv_fault_pending = false;

    PowerStage_Faults_t powerstage_faults = PowerStage_GetFaults(&board.powerstage);

    // Obsługa błędów drivera
    bool fault_detected = false;

    if (powerstage_faults.fault)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_DRIVER);
        fault_detected = true;
    }

    if (powerstage_faults.gate_fault)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_DRIVER_GATE);
        fault_detected = true;
    }

    if (powerstage_faults.overcurrent)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_DRIVER_OVERCURRENT);
        fault_detected = true;
    }

    if (powerstage_faults.overtemperature)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_DRIVER_OVERTEMP);
        fault_detected = true;
    }

    if (powerstage_faults.undervoltage)
    {
        MOTOR_FAULT_SET(g_motor_faults, MOTOR_FAULT_DRIVER_UNDERVOLTAGE);
        fault_detected = true;
    }

    if (fault_detected)
    {
        g_motor_state = STATE_FAULT;
    }

    MotorControl_PrintFaults(g_motor_faults);
}

void MotorControl_PrintFaults(MotorFaults_t faults)
{
    printf("Fault mask: 0x%04X\r\n", (unsigned int)faults);

    for (size_t i = 0;
         i < sizeof(fault_descriptions) / sizeof(fault_descriptions[0]);
         i++)
    {
        if (MOTOR_FAULT_IS_SET(faults, fault_descriptions[i].mask))
        {
            printf("%s detected.\r\n", fault_descriptions[i].message);
        }
    }
}

void MotorControl_ClearFaults(void)
{
    g_motor_state = STATE_FAULT;

    g_drv_fault_pending = true;
    g_motor_stop_pending = false;

    Board_ClearFaults(&board);

    g_motor_faults = MOTOR_FAULT_NONE;

    Motor_Measurements_t meas;
    MotorControl_GetMeasurements(&meas);

    MotorControl_ProcessControlFaults(&meas);

    MotorControl_ProcessDriverFaults();

    if (g_motor_faults != MOTOR_FAULT_NONE)
    {
        printf("Failed to clear faults.\r\n");
        MotorControl_PrintFaults(g_motor_faults);
        g_motor_state = STATE_FAULT;
        return;
    }

    // Allow motor to be started again
    
    HAL_NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);

    g_break_event_latched = false;
    g_motor_state = STATE_IDLE;

    printf("Faults cleared.\r\n");
}

// while(1) loop in main.c should call this function to handle pending faults
void MotorControl_OnFaultDetected(void)
{
    if (g_motor_stop_pending)
    {
        g_motor_stop_pending = false;
        // Board_StopMotor(&board);
        MotorControl_Stop();
    }

    MotorControl_ProcessDriverFaults();
}

void HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != board.htim_pwm->Instance)
        return;

    if (g_break_event_latched) // Trzeba wyzerować w porcedurze obsługi awarii
        return;
    
    // Wyłącznie przerwania, zapobiega zapętleniu w przypadku, gdy przerwanie Break jest wywoływane wielokrotnie
    HAL_NVIC_DisableIRQ(TIM1_BRK_TIM15_IRQn); // Trzeba włączyć ponownie w procedurze obsługi awarii

    g_break_event_latched = true;

    g_motor_state = STATE_FAULT;
    g_drv_fault_pending = true;
    g_motor_stop_pending = true;
}
