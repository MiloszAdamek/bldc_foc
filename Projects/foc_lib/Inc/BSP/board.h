/*
 * board.h
 *
 *  Created on: Jul 19, 2026
 *      Author: Milosz Adamek
 */

 #ifndef BOARD_H
 #define BOARD_H

 typedef enum
{    
    BOARD_G431_ESC = 0,
    BOARD_IHM03 = 1,
    BOARD_DRV8353 = 2
} BoardType_t;

typedef struct
{
    BoardType_t type;
    TIM_HandleTypeDef *htim_pwm;
    TIM_HandleTypeDef *htim_enc;
    ADC_HandleTypeDef *hadc_curr;
    SPI_HandleTypeDef *hspi_enc;
    SPI_HandleTypeDef *hspi_drv;
} BoardHandleTypeDef;

 #endif /* BOARD_H */