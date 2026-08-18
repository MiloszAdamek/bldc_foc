/*
 * board.c
 *
 *  Created on: Jul 25, 2026
 *      Author: Milosz Adamek
 */

 #include "App/config.h"
 #include "BSP/board.h"
 #include "BSP/current_sense.h"
 #include "BSP/PowerStage.h"
 #include "BSP/drv8353.h"
 #include "BSP/as5048a.h"

 #ifdef DRV8353

 static void Board_SetPinsToPWM(BoardHandleTypeDef *board);
 static void Board_SetLowPinsToGPIO(BoardHandleTypeDef *board);
 static void Board_SetHighPinsToGPIO(BoardHandleTypeDef *board);
 static void Board_SetPWMMode(BoardHandleTypeDef *board, PowerStage_PWM_Mode_t mode);
 static void Board_StartPWM(BoardHandleTypeDef *board);
 static void Board_StopPWM(BoardHandleTypeDef *board);

static const PowerStage_Pins_t inverter_pins = {
    .IN_H_A = {IN_H_A_GPIO_Port, IN_H_A_Pin, GPIO_AF6_TIM1},
    .IN_H_B = {IN_H_B_GPIO_Port, IN_H_B_Pin, GPIO_AF6_TIM1},
    .IN_H_C = {IN_H_C_GPIO_Port, IN_H_C_Pin, GPIO_AF6_TIM1},

    .IN_L_A = {IN_L_A_GPIO_Port, IN_L_A_Pin, GPIO_AF6_TIM1},
    .IN_L_B = {IN_L_B_GPIO_Port, IN_L_B_Pin, GPIO_AF6_TIM1},
    .IN_L_C = {IN_L_C_GPIO_Port, IN_L_C_Pin, GPIO_AF6_TIM1},
};

#elif defined(IHM03)

static const PowerStage_Pins_t inverter_pins = {
    ...
};

#endif

void Board_Init(BoardHandleTypeDef *board)
{   
    if (PowerStage_Init(&board->powerstage,
                    board->hspi_drv,
                    board->htim_pwm,
                    &inverter_pins) != POWERSTAGE_OK)
    {
        Error_Handler();
    }

    PowerStage_Off(&board->powerstage); // Disable PWM outputs

    Board_SetPWMMode(board, board->powerstage.pwm_mode); // Konfiguracja pinów STM32 w zależności od trybu PWM (3PWM lub 6PWM)

    DRV8353_PrintPWMMode(&board->powerstage.drv);
    
    // Current sense initialization and calibration

	HAL_TIM_Base_Start(board->htim_pwm);
	HAL_TIM_OC_Start(board->htim_pwm, TIM_CHANNEL_4); 	// Start CH4 -> wyzwalanie ADC

    #ifdef DRV8353
        PowerStage_SetCalibrationMode(&board->powerstage, true);
    #elif defined(IHM03)
        // Calibration for IHM03
    #endif

    CurrentSense_Init(board->hadc_curr);

    #ifdef DRV8353
        PowerStage_SetCalibrationMode(&board->powerstage, false);
    #elif defined(IHM03)
        // Calibration for IHM03
    #endif

    // Encoder initialization and calibration

    AS5048_Init(board->hspi_enc);

    // Powerstage initialization

    PowerStage_On(&board->powerstage);

    Board_StartPWM(board);
}

void Board_StartMotor(BoardHandleTypeDef *board)
{
    PowerStage_On(&board->powerstage);
}

void Board_StopMotor(BoardHandleTypeDef *board)
{
    PowerStage_Off(&board->powerstage);
}

static void Board_StartPWM(BoardHandleTypeDef *board)
{
    Board_SetPinsToPWM(board);

    if (board->powerstage.pwm_mode == POWERSTAGE_PWM_MODE_3PWM) {
        HAL_TIM_PWM_Start(board->powerstage.htim, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(board->powerstage.htim, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(board->powerstage.htim, TIM_CHANNEL_3);
    } else if (board->powerstage.pwm_mode == POWERSTAGE_PWM_MODE_6PWM) {
        HAL_TIM_PWM_Start(board->powerstage.htim, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Start(board->powerstage.htim, TIM_CHANNEL_1);

        HAL_TIM_PWM_Start(board->powerstage.htim, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Start(board->powerstage.htim, TIM_CHANNEL_2);

        HAL_TIM_PWM_Start(board->powerstage.htim, TIM_CHANNEL_3);
        HAL_TIMEx_PWMN_Start(board->powerstage.htim, TIM_CHANNEL_3);
    }
}

static void Board_StopPWM(BoardHandleTypeDef *board)
{
    if (board->powerstage.pwm_mode == POWERSTAGE_PWM_MODE_3PWM) {
        HAL_TIM_PWM_Stop(board->powerstage.htim, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(board->powerstage.htim, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(board->powerstage.htim, TIM_CHANNEL_3);
    } else if (board->powerstage.pwm_mode == POWERSTAGE_PWM_MODE_6PWM) {
        HAL_TIM_PWM_Stop(board->powerstage.htim, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Stop(board->powerstage.htim, TIM_CHANNEL_1);

        HAL_TIM_PWM_Stop(board->powerstage.htim, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Stop(board->powerstage.htim, TIM_CHANNEL_2);

        HAL_TIM_PWM_Stop(board->powerstage.htim, TIM_CHANNEL_3);
        HAL_TIMEx_PWMN_Stop(board->powerstage.htim, TIM_CHANNEL_3);
    }
}

void Board_SetPWMMode(BoardHandleTypeDef *board, PowerStage_PWM_Mode_t mode){
    
    // DRV8353_SetOutputState(&board->powerstage.drv, DRV_OUTPUT_COAST);

    board->powerstage.pwm_mode = mode;

    if (mode == POWERSTAGE_PWM_MODE_3PWM) {
        Board_SetLowPinsToGPIO(board);
        HAL_GPIO_WritePin(board->powerstage.pins.IN_L_A.port, board->powerstage.pins.IN_L_A.pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(board->powerstage.pins.IN_L_B.port, board->powerstage.pins.IN_L_B.pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(board->powerstage.pins.IN_L_C.port, board->powerstage.pins.IN_L_C.pin, GPIO_PIN_SET);
        Board_SetPinsToPWM(board);

    } else if (mode == POWERSTAGE_PWM_MODE_6PWM) {
        Board_SetPinsToPWM(board);
    }
}

static void Board_SetLowPinsToGPIO(BoardHandleTypeDef *board)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    PowerPin_t *low_pins[] =
    {
        &board->powerstage.pins.IN_L_A,
        &board->powerstage.pins.IN_L_B,
        &board->powerstage.pins.IN_L_C
    };

    for (int i = 0; i < 3; i++)
    {
        GPIO_InitStruct.Pin = low_pins[i]->pin;
        HAL_GPIO_Init(low_pins[i]->port, &GPIO_InitStruct);
    }
}

static void Board_SetHighPinsToGPIO(BoardHandleTypeDef *board)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    PowerPin_t *high_pins[] =
    {
        &board->powerstage.pins.IN_H_A,
        &board->powerstage.pins.IN_H_B,
        &board->powerstage.pins.IN_H_C
    };

    for (int i = 0; i < 3; i++)
    {
        GPIO_InitStruct.Pin = high_pins[i]->pin;
        HAL_GPIO_Init(high_pins[i]->port, &GPIO_InitStruct);
    }
}

static void Board_SetPinsToPWM(BoardHandleTypeDef *board)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    if (board->powerstage.pwm_mode == POWERSTAGE_PWM_MODE_3PWM)
    {
        /* Tylko IN_H_x jako AF */
        PowerPin_t *pins[] =
        {
            &board->powerstage.pins.IN_H_A,
            &board->powerstage.pins.IN_H_B,
            &board->powerstage.pins.IN_H_C
        };

        for (int i = 0; i < 3; i++)
        {
            GPIO_InitStruct.Pin       = pins[i]->pin;
            GPIO_InitStruct.Alternate = pins[i]->alternate;
            HAL_GPIO_Init(pins[i]->port, &GPIO_InitStruct);
        }
    }
    else if (board->powerstage.pwm_mode == POWERSTAGE_PWM_MODE_6PWM)
    {
        /* Wszystkie jako AF */
        PowerPin_t *pins[] =
        {
            &board->powerstage.pins.IN_H_A, &board->powerstage.pins.IN_H_B, &board->powerstage.pins.IN_H_C,
            &board->powerstage.pins.IN_L_A, &board->powerstage.pins.IN_L_B, &board->powerstage.pins.IN_L_C
        };

        for (int i = 0; i < 6; i++)
        {
            GPIO_InitStruct.Pin       = pins[i]->pin;
            GPIO_InitStruct.Alternate = pins[i]->alternate;
            HAL_GPIO_Init(pins[i]->port, &GPIO_InitStruct);
        }
    }
}

void Board_CheckFaults(BoardHandleTypeDef *board)
{
    DRV8353_Faults_t faults;
    if (DRV8353_GetFaults(&board->powerstage.drv, &faults) == DRV8353_OK)
    {
        DRV8353_PrintFaults(&faults);
    }
}