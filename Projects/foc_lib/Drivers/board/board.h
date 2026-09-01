/*
 * board.h
 *
 *  Created on: Jul 19, 2026
 *      Author: Milosz Adamek
 */

 #ifndef BOARD_H
 #define BOARD_H

 #include "powerstage.h"

typedef enum
{    
    BOARD_G431_ESC = 0,
    BOARD_IHM03 = 1,
    BOARD_DRV8353 = 2
} BoardType_t;

typedef struct
{
    BoardType_t type;

    // Peripherals handlers
    TIM_HandleTypeDef *htim_pwm;
    TIM_HandleTypeDef *htim_enc;
    TIM_HandleTypeDef *htim_pos;
    TIM_HandleTypeDef *htim_speed;
    TIM_HandleTypeDef *htim_cmd;
    ADC_HandleTypeDef *hadc_currA;
    ADC_HandleTypeDef *hadc_currB;
    ADC_HandleTypeDef *hadc_VDC;
    SPI_HandleTypeDef *hspi_enc;
    SPI_HandleTypeDef *hspi_drv;
    UART_HandleTypeDef *huart_com;

    // Powerstage handler
    PowerStage_HandleTypeDef powerstage;

    // VDD voltage
    float vdd_voltage;

} BoardHandleTypeDef;

void Board_Init(BoardHandleTypeDef *board);
void Board_StartMotor(BoardHandleTypeDef *board);
void Board_StopMotor(BoardHandleTypeDef *board);
void Board_CheckFaults(BoardHandleTypeDef *board);
void Board_GetVddVoltage(BoardHandleTypeDef *board);

 #endif /* BOARD_H */