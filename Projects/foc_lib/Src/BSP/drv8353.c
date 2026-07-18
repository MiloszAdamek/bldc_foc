/*
 * drv8353.c
 *
 *  Created on: Jul 7, 2026
 *      Author: Milosz Adamek
 */

#include "drv8353.h"
#include "main.h"
#include <stdio.h>

DRV8353_Status_t DRV8353_Init(DRV8353_HandleTypeDef *drv, SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim)
{
    if(drv == NULL || hspi == NULL || htim == NULL)
    {
        return DRV8353_ERROR;
    }

    drv->hspi = hspi;
    drv->htim = htim;

    drv->cs_port = DRV8353_CS_GPIO_Port;
    drv->cs_pin = DRV8353_CS_Pin;

    drv->enable_port = DRV8353_ENABLE_GPIO_Port;
    drv->enable_pin = DRV8353_ENABLE_Pin;

    drv->fault_port = DRV8353_nFAULT_GPIO_Port;
    drv->fault_pin = DRV8353_nFAULT_Pin;

    DRV8353_Status_t enable_status = DRV8353_Enable(drv);
    if(enable_status != DRV8353_OK)
    {
        return DRV8353_ERROR;
    }
    HAL_Delay(1);

    // sprawdź nFAULT po wake
    if(DRV8353_CheckFaultPin(drv) != DRV8353_OK)
    {
        DRV8353_GetFaults(drv, &drv->faults);
        DRV8353_PrintFaults(&drv->faults);
        return DRV8353_ERROR;
    }

    DRV8353_Status_t clear_faults_status = DRV8353_ClearFaults(drv);
    if(clear_faults_status != DRV8353_OK)
    {
        printf("DRV8353 clear faults failed!\r\n");
        return DRV8353_ERROR;
    }

    DRV8353_Config_t default_config = {
        .pwm_mode   = DRV8353_PWM_MODE_3PWM,
        .csa_gain   = DRV8353_CSA_GAIN_20V,
        .idriven_hs = DRV8353_IDRIVEN_2000mA,
        .idrivep_hs = DRV8353_IDRIVEP_1000mA,
        .idriven_ls = DRV8353_IDRIVEN_2000mA,
        .idrivep_ls = DRV8353_IDRIVEP_1000mA,
        .dead_time  = DRV8353_DEADTIME_100ns,
        .ocp_level  = DRV8353_VDS_OCP_300mV,
        .drive_time = DRV8353_DRIVETIME_1000ns
    };

    DRV8353_SetOutputState(drv, DRV_OUTPUT_COAST);

    DRV8353_Status_t config_status = DRV8353_InitConfig(drv, &default_config);

    if(config_status != DRV8353_OK)
    {
        drv->initialized = false;
        printf("DRV8353 config failed!\r\n");
        return DRV8353_ERROR;
    }

    DRV8353_Status_t fault_status = DRV8353_CheckFaultPin(drv);
    if(fault_status != DRV8353_OK)
    {
        drv->initialized = false;
        printf("DRV8353 nFAULT pin is low after initialization!\r\n");
        DRV8353_GetFaults(drv, &drv->faults);
        DRV8353_PrintFaults(&drv->faults);
        return DRV8353_ERROR;
    }

    DRV8353_Status_t verify_status = DRV8353_VerifyConfig(drv, &default_config);
    if(verify_status != DRV8353_OK)
    {
        drv->initialized = false;
        printf("DRV8353 config verification failed!\r\n");
        return DRV8353_ERROR;
    }

    printf("DRV8353 initialized successfully!\r\n");

    drv->initialized = true;

    return DRV8353_OK;
}

DRV8353_Status_t DRV8353_InitConfig(DRV8353_HandleTypeDef *drv, DRV8353_Config_t *config)
{
    if(drv == NULL || config == NULL)
    {
        return DRV8353_ERROR;
    }

    // Set default configuration values
    DRV8353_Status_t status = DRV8353_SetPWMMode(drv, config->pwm_mode);
    if(status != DRV8353_OK)
    {
        return DRV8353_ERROR;
    }

    status = DRV8353_SetAmplifierGain(drv, config->csa_gain);
    if(status != DRV8353_OK)
    {
        return DRV8353_ERROR;
    }

    status = DRV8353_SetGateDriveCurrent(drv, config->idrivep_hs, 
                                        config->idriven_hs, 
                                        config->idrivep_ls, 
                                        config->idriven_ls,
                                        config->drive_time);
    if(status != DRV8353_OK)
    {
        return DRV8353_ERROR;
    }

    status = DRV8353_SetDeadTime(drv, config->dead_time);
    if(status != DRV8353_OK)
    {
        return DRV8353_ERROR;
    }

    status = DRV8353_SetOvercurrentProtection(drv, config->ocp_level);
    if(status != DRV8353_OK)
    {
        return DRV8353_ERROR;
    }

    return DRV8353_OK;
}

DRV8353_Status_t DRV8353_Enable(DRV8353_HandleTypeDef *drv){
    
    if(drv == NULL)
    {
        return DRV8353_ERROR;
    }

    drv->enable_port->BSRR = drv->enable_pin; // Set the enable pin high

    return DRV8353_OK;
}

DRV8353_Status_t DRV8353_Disable(DRV8353_HandleTypeDef *drv){
    
    if(drv == NULL)
    {
        return DRV8353_ERROR;
    }

    drv->enable_port->BSRR = ((uint32_t)drv->enable_pin << 16U); // Set the enable pin low

    return DRV8353_OK;
}

DRV8353_Status_t DRV8353_ReadRegister(DRV8353_HandleTypeDef *drv,
                                      DRV8353_Register_t reg,
                                      uint16_t *data)
{
    if(drv == NULL || data == NULL || drv->hspi == NULL)
    {
        return DRV8353_ERROR;
    }

    uint16_t tx;
    uint16_t rx;

    tx = (1U << 15) | (((uint16_t)reg & 0x0F) << 11); // Format: 1 (read) | 4 bits register address | 11 bits data (don't care for read)

    DRV8353_CS_LOW(drv);

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
            drv->hspi,
            (uint8_t*)&tx,
            (uint8_t*)&rx,
            1, // Transmit and receive 1 word (16 bits)
            100
    );

    DRV8353_CS_HIGH(drv);

    if(status != HAL_OK)
    {
        return DRV8353_ERROR;
    }

    // printf("READ REG 0x%02X\r\n", reg);
    // printf("TX: 0x%04X\r\n", tx);
    // printf("RX: 0x%04X\r\n", rx);

    *data = rx & 0x07FF;

    return DRV8353_OK;
}

DRV8353_Status_t DRV8353_WriteRegister(DRV8353_HandleTypeDef *drv,
                                       DRV8353_Register_t reg,
                                       uint16_t data)
{
    if(drv == NULL || drv->hspi == NULL)
    {
        return DRV8353_ERROR;
    }

    uint16_t tx;
    uint16_t rx;    

    tx = (((uint16_t)reg & 0x0F) << 11) | (data & 0x07FF);

    DRV8353_CS_LOW(drv);

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(drv->hspi, (uint8_t*)&tx, (uint8_t*)&rx, 1, 100);

    DRV8353_CS_HIGH(drv);

    // printf("WRITE REG 0x%02X DATA 0x%04X\r\n", reg, data);
    // printf("TX bytes: %02X %02X\r\n", tx >> 8, tx & 0xFF);

    return (status == HAL_OK) ? DRV8353_OK : DRV8353_ERROR;
}

DRV8353_Status_t DRV8353_CheckFaultPin(DRV8353_HandleTypeDef *drv)
{
    if(drv == NULL)
    {
        return DRV8353_ERROR;
    }

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(drv->fault_port, drv->fault_pin);

    return (pin_state == GPIO_PIN_RESET) ? DRV8353_ERROR : DRV8353_OK;
}

DRV8353_Status_t DRV8353_GetFaults(DRV8353_HandleTypeDef *drv, DRV8353_Faults_t *faults)
{
    if(drv == NULL || faults == NULL)
    {
        return DRV8353_ERROR;
    }

    uint16_t status1 = 0;
    uint16_t status2 = 0;

    if(DRV8353_ReadRegister(drv, DRV8353_REG_STATUS1, &status1) != DRV8353_OK)
    {
        return DRV8353_ERROR;
    }

    if(DRV8353_ReadRegister(drv, DRV8353_REG_STATUS2, &status2) != DRV8353_OK)
    {
        return DRV8353_ERROR;
    }

    faults->fault = (status1 & DRV8353_STATUS1_FAULT_Msk) != 0;
    faults->overcurrent = (status1 & DRV8353_STATUS1_VDS_OCP_Msk) != 0;
    faults->gate_fault = (status1 & DRV8353_STATUS1_GDF_Msk) != 0;
    faults->undervoltage = (status1 & DRV8353_STATUS1_UVLO_Msk) != 0;
    faults->overtemperature = (status2 & DRV8353_STATUS2_OTW_Msk) != 0;

    return DRV8353_OK;
}

void DRV8353_PrintFaults(DRV8353_Faults_t *faults)
{
    if(faults->fault)
        printf("DRV8353: FAULT\r\n");

    if(faults->overcurrent)
        printf("DRV8353: Overcurrent\r\n");

    if(faults->gate_fault)
        printf("DRV8353: Gate driver fault\r\n");

    if(faults->undervoltage)
        printf("DRV8353: Undervoltage\r\n");

    if(faults->overtemperature)
        printf("DRV8353: Overtemperature\r\n");

    if(!faults->fault)
        printf("DRV8353: No faults\r\n");
}

void DRV8353_PrintPWMMode(DRV8353_HandleTypeDef *drv)
{
    uint16_t reg_value = 0;
    DRV8353_ReadRegister(drv, DRV8353_REG_DRIVER_CONTROL, &reg_value);

    switch((reg_value & DRV8353_PWM_MODE_Msk) >> DRV8353_PWM_MODE_Pos)
    {
        case DRV8353_PWM_MODE_6PWM:
            printf("DRV8353: PWM Mode - 6PWM\r\n");
            break;
        case DRV8353_PWM_MODE_3PWM:
            printf("DRV8353: PWM Mode - 3PWM\r\n");
            break;
        case DRV8353_PWM_MODE_1PWM:
            printf("DRV8353: PWM Mode - 1PWM\r\n");
            break;
        case DRV8353_PWM_MODE_INDEPENDENT:
            printf("DRV8353: PWM Mode - Independent\r\n");
            break;
        default:
            printf("DRV8353: Unknown PWM Mode\r\n");
            break;
    }
}

static DRV8353_Status_t DRV8353_UpdateRegisterBits(DRV8353_HandleTypeDef *drv, DRV8353_Register_t reg, uint16_t mask, uint16_t value)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    uint16_t reg_value;

    // 1. Read
    if(DRV8353_ReadRegister(drv, reg, &reg_value) != DRV8353_OK)
        return DRV8353_ERROR;

    // 2. Clear masked bits
    reg_value &= ~mask;

    // 3. Set new value (masked!)
    reg_value |= (value & mask);

    // 4. Write back
    if(DRV8353_WriteRegister(drv, reg, reg_value) != DRV8353_OK)
        return DRV8353_ERROR;

    return DRV8353_OK;
}

DRV8353_Status_t DRV8353_SetPWMMode(
        DRV8353_HandleTypeDef *drv,
        DRV8353_PWM_Mode_t pwm_mode)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    return DRV8353_UpdateRegisterBits(
            drv,
            DRV8353_REG_DRIVER_CONTROL,
            DRV8353_PWM_MODE_Msk,
            (uint16_t)pwm_mode << DRV8353_PWM_MODE_Pos);
}

DRV8353_Status_t DRV8353_SetAmplifierGain(
        DRV8353_HandleTypeDef *drv,
        DRV8353_CSA_Gain_t gain)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    return DRV8353_UpdateRegisterBits(
            drv,
            DRV8353_REG_CSA_CONTROL,
            DRV8353_CSA_GAIN_Msk,
            ((uint16_t)gain << DRV8353_CSA_GAIN_Pos));
}

DRV8353_Status_t DRV8353_ClearFaults(DRV8353_HandleTypeDef *drv)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    // Clear faults by writing 1 to the FAULT bit in STATUS1 register
    return DRV8353_UpdateRegisterBits(
            drv,
            DRV8353_REG_DRIVER_CONTROL,
            DRV8353_CLR_FLT_Msk,
            DRV8353_CLR_FLT_Msk);
}   

DRV8353_Status_t DRV8353_PWMDisable(DRV8353_HandleTypeDef *drv)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    HAL_TIM_PWM_Stop(drv->htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(drv->htim, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(drv->htim, TIM_CHANNEL_3);
    
    return DRV8353_OK;
}

DRV8353_Status_t DRV8353_SetGateDriveCurrent(
        DRV8353_HandleTypeDef *drv,
        DRV8353_IDRIVEP_t idrivep_hs,
        DRV8353_IDRIVEN_t idriven_hs,
        DRV8353_IDRIVEP_t idrivep_ls,
        DRV8353_IDRIVEN_t idriven_ls,
        DRV8353_DriveTime_t drive_time)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    DRV8353_Status_t status;

    uint16_t hs_value =
        ((uint16_t)idrivep_hs << DRV8353_IDRIVEP_Pos) |
        ((uint16_t)idriven_hs << DRV8353_IDRIVEN_Pos);

    uint16_t ls_value =
        ((uint16_t)idrivep_ls << DRV8353_IDRIVEP_Pos) |
        ((uint16_t)idriven_ls << DRV8353_IDRIVEN_Pos) |
        ((uint16_t)drive_time << DRV8353_DRIVETIME_Pos);

    status = DRV8353_UpdateRegisterBits(
                drv,
                DRV8353_REG_GATE_DRIVE_HS,
                DRV8353_IDRIVEP_Msk | DRV8353_IDRIVEN_Msk,
                hs_value);
    if(status != DRV8353_OK)
        return status;
    
    return DRV8353_UpdateRegisterBits(
                drv,
                DRV8353_REG_GATE_DRIVE_LS,
                DRV8353_IDRIVEP_Msk | DRV8353_IDRIVEN_Msk | DRV8353_DRIVETIME_Msk,
                ls_value);
}

DRV8353_Status_t DRV8353_SetDeadTime(
        DRV8353_HandleTypeDef *drv,
        DRV8353_DeadTime_t dead_time)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    return DRV8353_UpdateRegisterBits(
            drv,
            DRV8353_REG_DRIVER_CONF,
            DRV8353_DEADTIME_Msk,
            ((uint16_t)dead_time << DRV8353_DEADTIME_Pos));
}

DRV8353_Status_t DRV8353_SetOvercurrentProtection(
        DRV8353_HandleTypeDef *drv,
        DRV8353_VDS_OCP_t ocp)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    return DRV8353_UpdateRegisterBits(
            drv,
            DRV8353_REG_OCP_CONTROL,
            DRV8353_VDS_OCP_Msk,
            ((uint16_t)ocp << DRV8353_VDS_OCP_Pos));
}

DRV8353_Status_t DRV8353_SetOutputState(
        DRV8353_HandleTypeDef *drv,
        DRV8353_OutputState_t state)
{
    if(drv == NULL)
        return DRV8353_ERROR;

    uint16_t mask = DRV8353_COAST_Msk | DRV8353_BRAKE_Msk;
    uint16_t value = 0;

    switch(state)
    {
        case DRV_OUTPUT_RUN:
            // COAST = 0, BRAKE = 0
            value = 0;
            break;

        case DRV_OUTPUT_COAST:
            // COAST = 1, BRAKE = 0
            value = DRV8353_COAST_Msk;
            break;

        case DRV_OUTPUT_BRAKE:
            // COAST = 0, BRAKE = 1
            value = DRV8353_BRAKE_Msk;
            break;

        default:
            return DRV8353_ERROR;
    }

    return DRV8353_UpdateRegisterBits(
            drv,
            DRV8353_REG_DRIVER_CONTROL,
            mask,
            value);
}

DRV8353_Status_t DRV8353_VerifyConfig(
        DRV8353_HandleTypeDef *drv,
        DRV8353_Config_t *cfg)
{
    if(drv == NULL || cfg == NULL)
        return DRV8353_ERROR;

    uint16_t reg;

    // DRIVER_CONTROL (PWM_MODE)
    if(DRV8353_ReadRegister(drv, DRV8353_REG_DRIVER_CONTROL, &reg) != DRV8353_OK)
        return DRV8353_ERROR;

    uint16_t pwm_mode = (reg & DRV8353_PWM_MODE_Msk) >> DRV8353_PWM_MODE_Pos;
    if(pwm_mode != cfg->pwm_mode)
        return DRV8353_ERROR;


    // CSA_CONTROL (GAIN)
    if(DRV8353_ReadRegister(drv, DRV8353_REG_CSA_CONTROL, &reg) != DRV8353_OK)
        return DRV8353_ERROR;

    uint16_t gain = (reg & DRV8353_CSA_GAIN_Msk) >> DRV8353_CSA_GAIN_Pos;
    if(gain != cfg->csa_gain)
        return DRV8353_ERROR;


    // GATE_DRIVE_HS
    if(DRV8353_ReadRegister(drv, DRV8353_REG_GATE_DRIVE_HS, &reg) != DRV8353_OK)
        return DRV8353_ERROR;

    uint16_t idrivep_hs = (reg & DRV8353_IDRIVEP_Msk) >> DRV8353_IDRIVEP_Pos;
    uint16_t idriven_hs = (reg & DRV8353_IDRIVEN_Msk) >> DRV8353_IDRIVEN_Pos;

    if(idrivep_hs != cfg->idrivep_hs ||
       idriven_hs != cfg->idriven_hs)
        return DRV8353_ERROR;


    // GATE_DRIVE_LS
    if(DRV8353_ReadRegister(drv, DRV8353_REG_GATE_DRIVE_LS, &reg) != DRV8353_OK)
        return DRV8353_ERROR;

    uint16_t idrivep_ls = (reg & DRV8353_IDRIVEP_Msk) >> DRV8353_IDRIVEP_Pos;
    uint16_t idriven_ls = (reg & DRV8353_IDRIVEN_Msk) >> DRV8353_IDRIVEN_Pos;
    uint16_t drive_time = (reg & DRV8353_DRIVETIME_Msk) >> DRV8353_DRIVETIME_Pos;

    if(idrivep_ls != cfg->idrivep_ls ||
       idriven_ls != cfg->idriven_ls ||
       drive_time != cfg->drive_time)
        return DRV8353_ERROR;


    // DRIVER_CONF (DEADTIME)
    if(DRV8353_ReadRegister(drv, DRV8353_REG_DRIVER_CONF, &reg) != DRV8353_OK)
        return DRV8353_ERROR;

    uint16_t dead_time = (reg & DRV8353_DEADTIME_Msk) >> DRV8353_DEADTIME_Pos;
    if(dead_time != cfg->dead_time)
        return DRV8353_ERROR;


    // OCP_CONTROL
    if(DRV8353_ReadRegister(drv, DRV8353_REG_OCP_CONTROL, &reg) != DRV8353_OK)
        return DRV8353_ERROR;

    uint16_t ocp = (reg & DRV8353_VDS_OCP_Msk) >> DRV8353_VDS_OCP_Pos;
    if(ocp != cfg->ocp_level)
        return DRV8353_ERROR;

    return DRV8353_OK;
}