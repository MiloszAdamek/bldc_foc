/*
 * drv8353.h
 *
 *  Created on: Jul 17, 2026
 *      Author: Milosz Adamek
 */

#ifndef DRV8353_H
#define DRV8353_H

#include "main.h"
#include <stdbool.h>

#define DRV8353_STATUS1_FAULT_Msk     (1U << 10) // Logic OR of FAULT status registers, mirrors nFAULT pin
#define DRV8353_STATUS1_VDS_OCP_Msk   (1U << 9)  // VDS overcurrent fault
#define DRV8353_STATUS1_GDF_Msk       (1U << 8)  // Gate drive fault
#define DRV8353_STATUS1_UVLO_Msk      (1U << 7)  // Undervoltage lockout
#define DRV8353_STATUS1_OTSD_Msk      (1U << 6)  // Overtemperature shutdown

// GVDS overcurrent fault on specified mosfet
#define DRV8353_STATUS1_VDS_HA_Msk    (1U << 5)
#define DRV8353_STATUS1_VDS_LA_Msk    (1U << 4)
#define DRV8353_STATUS1_VDS_HB_Msk    (1U << 3)
#define DRV8353_STATUS1_VDS_LB_Msk    (1U << 2)
#define DRV8353_STATUS1_VDS_HC_Msk    (1U << 1)
#define DRV8353_STATUS1_VDS_LC_Msk    (1U << 0)

#define DRV8353_STATUS2_SA_OCP_Msk    (1U << 10) // Overcurrent on phase A sense amplifier
#define DRV8353_STATUS2_SB_OCP_Msk    (1U << 9)  // Overcurrent on phase B sense amplifier
#define DRV8353_STATUS2_SC_OCP_Msk    (1U << 8)  // Overcurrent on phase C sense amplifier
#define DRV8353_STATUS2_OTW_Msk       (1U << 7)  // Overtemperature warning
#define DRV8353_STATUS2_GDUV_Msk      (1U << 6)  // VCP charge pump and/or VGLS undervoltage fault

// Gate drive fault on specified mosfet
#define DRV8353_STATUS1_VGS_HA_Msk    (1U << 5)
#define DRV8353_STATUS1_VGS_LA_Msk    (1U << 4)
#define DRV8353_STATUS1_VGS_HB_Msk    (1U << 3)
#define DRV8353_STATUS1_VGS_LB_Msk    (1U << 2)
#define DRV8353_STATUS1_VGS_HC_Msk    (1U << 1)
#define DRV8353_STATUS1_VGS_LC_Msk    (1U << 0)

// Masks for control settings
#define DRV8353_CLR_FLT_Pos    0U
#define DRV8353_CLR_FLT_Msk    (0x01U << DRV8353_CLR_FLT_Pos)

#define DRV8353_PWM_MODE_Pos   5U
#define DRV8353_PWM_MODE_Msk   (0x03U << DRV8353_PWM_MODE_Pos)

#define DRV8353_CSA_GAIN_Pos   6U
#define DRV8353_CSA_GAIN_Msk   (0x03U << DRV8353_CSA_GAIN_Pos)

#define DRV8353_COAST_Pos      2U
#define DRV8353_COAST_Msk      (1U << DRV8353_COAST_Pos)

#define DRV8353_BRAKE_Pos      1U
#define DRV8353_BRAKE_Msk      (1U << DRV8353_BRAKE_Pos)

#define DRV8353_IDRIVEP_Pos    4U
#define DRV8353_IDRIVEP_Msk    (0x0FU << DRV8353_IDRIVEP_Pos)

#define DRV8353_IDRIVEN_Pos    0U
#define DRV8353_IDRIVEN_Msk    (0x0FU << DRV8353_IDRIVEN_Pos)

#define DRV8353_DEADTIME_Pos   8U
#define DRV8353_DEADTIME_Msk   (0x03U << DRV8353_DEADTIME_Pos)

#define DRV8353_VDS_OCP_Pos    0U
#define DRV8353_VDS_OCP_Msk    (0x0FU << DRV8353_VDS_OCP_Pos)

#define DRV8353_OCP_MODE_Pos   6U
#define DRV8353_OCP_MODE_Msk   (0x03U << DRV8353_OCP_MODE_Pos)

#define DRV8353_DRIVETIME_Pos  8U
#define DRV8353_DRIVETIME_Msk  (0x03U << DRV8353_DRIVETIME_Pos)

#define DRV8353_CSA_CAL_Pos    2U
#define DRV8353_CSA_CAL_Msk    (0x07U << DRV8353_CSA_CAL_Pos)

typedef enum {
    DRV8353_OK = 0,
    DRV8353_ERROR = 1
} DRV8353_Status_t;

typedef struct
{
    bool fault;
    bool gate_fault;
    bool overcurrent;
    bool overtemperature;
    bool undervoltage;
} DRV8353_Faults_t;

typedef enum
{
    DRV8353_PWM_MODE_6PWM        = 0x00,
    DRV8353_PWM_MODE_3PWM        = 0x01,
    DRV8353_PWM_MODE_1PWM        = 0x02,
    DRV8353_PWM_MODE_INDEPENDENT = 0x03
} DRV8353_PWM_Mode_t;

typedef enum
{
    DRV8353_CSA_GAIN_5V  = 0x00,  // 00
    DRV8353_CSA_GAIN_10V = 0x01,  // 01
    DRV8353_CSA_GAIN_20V = 0x02,  // 10
    DRV8353_CSA_GAIN_40V = 0x03   // 11
} DRV8353_CSA_Gain_t;

typedef enum // Peak source gate current (ładowanie bramki)
{
    // DRV8353_IDRIVEP_50mA   = 0b0000,
    DRV8353_IDRIVEP_50mA   = 0b0001,
    DRV8353_IDRIVEP_100mA  = 0b0010,
    DRV8353_IDRIVEP_150mA  = 0b0011,
    DRV8353_IDRIVEP_300mA  = 0b0100,
    DRV8353_IDRIVEP_350mA  = 0b0101,
    DRV8353_IDRIVEP_400mA  = 0b0110,
    DRV8353_IDRIVEP_450mA  = 0b0111,
    DRV8353_IDRIVEP_550mA  = 0b1000,
    DRV8353_IDRIVEP_600mA  = 0b1001,
    DRV8353_IDRIVEP_650mA  = 0b1010,
    DRV8353_IDRIVEP_700mA  = 0b1011,
    DRV8353_IDRIVEP_850mA  = 0b1100,
    DRV8353_IDRIVEP_900mA  = 0b1101,
    DRV8353_IDRIVEP_950mA  = 0b1110,
    DRV8353_IDRIVEP_1000mA = 0b1111,
} DRV8353_IDRIVEP_t;

typedef enum // Peak sink gate current (rozładowywanie bramki)
{
    // DRV8353_IDRIVEN_100mA   = 0b0000,
    DRV8353_IDRIVEN_100mA   = 0b0001,
    DRV8353_IDRIVEN_200mA   = 0b0010,
    DRV8353_IDRIVEN_300mA   = 0b0011,
    DRV8353_IDRIVEN_600mA   = 0b0100,
    DRV8353_IDRIVEN_700mA   = 0b0101,
    DRV8353_IDRIVEN_800mA   = 0b0110,
    DRV8353_IDRIVEN_900mA   = 0b0111,
    DRV8353_IDRIVEN_1100mA  = 0b1000,
    DRV8353_IDRIVEN_1200mA  = 0b1001,
    DRV8353_IDRIVEN_1300mA  = 0b1010,
    DRV8353_IDRIVEN_1400mA  = 0b1011,
    DRV8353_IDRIVEN_1700mA  = 0b1100,
    DRV8353_IDRIVEN_1800mA  = 0b1101,
    DRV8353_IDRIVEN_1900mA  = 0b1110,
    DRV8353_IDRIVEN_2000mA  = 0b1111,
} DRV8353_IDRIVEN_t;

typedef enum{
    DRV8353_DEADTIME_50ns   = 0b00,
    DRV8353_DEADTIME_100ns  = 0b01,
    DRV8353_DEADTIME_200ns  = 0b10,
    DRV8353_DEADTIME_400ns  = 0b11,
} DRV8353_DeadTime_t;

typedef enum{
    DRV8353_DRIVETIME_500ns   = 0b00,
    DRV8353_DRIVETIME_1000ns  = 0b01,
    DRV8353_DRIVETIME_2000ns  = 0b10,
    DRV8353_DRIVETIME_4000ns  = 0b11,
} DRV8353_DriveTime_t;

typedef enum {
    DRV8353_VDS_OCP_60mV   = 0b0000,
    DRV8353_VDS_OCP_70mV   = 0b0001,
    DRV8353_VDS_OCP_80mV   = 0b0010,
    DRV8353_VDS_OCP_90mV   = 0b0011,
    DRV8353_VDS_OCP_100mV  = 0b0100,
    DRV8353_VDS_OCP_200mV  = 0b0101,
    DRV8353_VDS_OCP_300mV  = 0b0110,
    DRV8353_VDS_OCP_400mV  = 0b0111,
    DRV8353_VDS_OCP_500mV  = 0b1000,
    DRV8353_VDS_OCP_600mV  = 0b1001,
    DRV8353_VDS_OCP_700mV  = 0b1010,
    DRV8353_VDS_OCP_800mV  = 0b1011,
    DRV8353_VDS_OCP_900mV  = 0b1100,
    DRV8353_VDS_OCP_1000mV = 0b1101,
    DRV8353_VDS_OCP_1500mV = 0b1110,
    DRV8353_VDS_OCP_2000mV = 0b1111,
} DRV8353_VDS_OCP_t;

typedef enum {
    DRV8353_OCP_MODE_LATCHED     = 0b00,
    DRV8353_OCP_MODE_AUTO_RETRY  = 0b01,
    DRV8353_OCP_MODE_REPORT_ONLY = 0b10,
    DRV8353_OCP_MODE_NO_ACTION   = 0b11
} DRV8353_OCP_Mode_t;

typedef struct {
    DRV8353_PWM_Mode_t pwm_mode;
    DRV8353_CSA_Gain_t csa_gain;
    DRV8353_IDRIVEP_t idrivep_hs;
    DRV8353_IDRIVEN_t idriven_hs;
    DRV8353_IDRIVEP_t idrivep_ls;
    DRV8353_IDRIVEN_t idriven_ls;
    DRV8353_DeadTime_t dead_time;
    DRV8353_VDS_OCP_t ocp_level;
    DRV8353_OCP_Mode_t ocp_mode;
    DRV8353_DriveTime_t drive_time;
} DRV8353_Config_t;

typedef enum{
    DRV8353_REG_STATUS1         = 0x00,
    DRV8353_REG_STATUS2         = 0x01,
    DRV8353_REG_DRIVER_CONTROL  = 0x02,
    DRV8353_REG_GATE_DRIVE_HS   = 0x03,
    DRV8353_REG_GATE_DRIVE_LS   = 0x04,
    DRV8353_REG_OCP_CONTROL     = 0x05,
    DRV8353_REG_CSA_CONTROL     = 0x06,
    DRV8353_REG_DRIVER_CONF     = 0x07,
} DRV8353_Register_t;

typedef enum
{
    DRV_OUTPUT_RUN = 0,
    DRV_OUTPUT_COAST,
    DRV_OUTPUT_BRAKE
} DRV8353_OutputState_t;

typedef struct {

    TIM_HandleTypeDef *htim;

    SPI_HandleTypeDef *hspi;

    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;

    GPIO_TypeDef *enable_port;
    uint16_t enable_pin;

    GPIO_TypeDef *fault_port;
    uint16_t fault_pin;

    bool initialized;

    DRV8353_Faults_t faults;

} DRV8353_HandleTypeDef;

void DRV8353_PrintFaults(DRV8353_Faults_t *faults);

void DRV8353_PrintPWMMode(DRV8353_HandleTypeDef *drv);

DRV8353_Status_t DRV8353_Init(DRV8353_HandleTypeDef *drv, SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim, DRV8353_PWM_Mode_t pwm_mode);

DRV8353_Status_t DRV8353_InitConfig(DRV8353_HandleTypeDef *drv, DRV8353_Config_t *config);

DRV8353_Status_t DRV8353_VerifyConfig(DRV8353_HandleTypeDef *drv, DRV8353_Config_t *cfg);

DRV8353_Status_t DRV8353_WriteRegister(DRV8353_HandleTypeDef *drv, DRV8353_Register_t reg, uint16_t data);

DRV8353_Status_t DRV8353_ReadRegister(DRV8353_HandleTypeDef *drv, DRV8353_Register_t reg, uint16_t *data);

DRV8353_Status_t DRV8353_Enable(DRV8353_HandleTypeDef *drv);

DRV8353_Status_t DRV8353_Disable(DRV8353_HandleTypeDef *drv);

DRV8353_Status_t DRV8353_CheckFaultPin(DRV8353_HandleTypeDef *drv);

DRV8353_Status_t DRV8353_GetFaults(DRV8353_HandleTypeDef *drv, DRV8353_Faults_t *faults);

DRV8353_Status_t DRV8353_ClearFaults(DRV8353_HandleTypeDef *drv);

DRV8353_Status_t DRV8353_SetPWMMode(DRV8353_HandleTypeDef *drv, DRV8353_PWM_Mode_t pwm_mode);

DRV8353_Status_t DRV8353_SetAmplifierGain(DRV8353_HandleTypeDef *drv, DRV8353_CSA_Gain_t gain);

DRV8353_Status_t DRV8353_SetDeadTime(DRV8353_HandleTypeDef *drv, DRV8353_DeadTime_t dead_time);

DRV8353_Status_t DRV8353_SetOvercurrentProtection(DRV8353_HandleTypeDef *drv, DRV8353_VDS_OCP_t ocp, DRV8353_OCP_Mode_t ocp_mode);

DRV8353_Status_t DRV8353_SetGateDriveCurrent(DRV8353_HandleTypeDef *drv, DRV8353_IDRIVEP_t idrivep_hs, DRV8353_IDRIVEN_t idriven_hs, DRV8353_IDRIVEP_t idrivep_ls, DRV8353_IDRIVEN_t idriven_ls, DRV8353_DriveTime_t drive_time);

DRV8353_Status_t DRV8353_SetOutputState(DRV8353_HandleTypeDef *drv, DRV8353_OutputState_t state);

DRV8353_Status_t DRV8353_SetCalibrationMode(DRV8353_HandleTypeDef *drv, bool enable);

static inline void DRV8353_CS_LOW(DRV8353_HandleTypeDef *drv)  { drv->cs_port->BSRR = ((uint32_t)drv->cs_pin << 16U); }

static inline void DRV8353_CS_HIGH(DRV8353_HandleTypeDef *drv) { drv->cs_port->BSRR = drv->cs_pin; }

#endif /* DRV8353_H */
