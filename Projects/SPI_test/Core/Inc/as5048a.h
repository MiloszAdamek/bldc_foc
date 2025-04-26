/*
 * as5048a.h
 *
 *  Created on: Apr 9, 2025
 *      Author: Miloush
 */

#ifndef INC_AS5048A_H_
#define INC_AS5048A_H_

#include "main.h"
#include "spi.h"
#include <stdbool.h>
#include "delay_us.h"

// Control and Error Registers
#define AS_NOP 				0x0000
#define AS_CLR_ERR 			0x0001
#define AS_PROG				0x0003
// Programmable Customer Settings
#define AS_OTP_ZERO_POS_H 	0x0016
#define AS_OTP_ZERO_POS_L 	0x0017
// Readout Registers
#define AS_DIAG_AGC			0x3FFD
#define AS_MAGNITUDE		0x3FFE
#define AS_ANGLE			0x3FFF

#define AS5048_CS_LOW()    		HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET)
#define AS5048_CS_HIGH()   		HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET)
#define AS5048_SPI_HANDLE    	(&hspi1)


// Status funkcji
typedef enum {
    AS5048_OK = 0,
    AS5048_ERR_SPI,
    AS5048_ERR_PARITY,
    AS5048_ERR_FLAG
} AS5048_Status;

// Flagi błędów z rejestru 0x0001
typedef struct {
    bool watchdogError;
    bool offsetFinished;
    bool cordicOverflow;
} AS5048_ErrorFlags;

// Wynik odczytu pozycji
typedef struct {
    AS5048_Status status;
    uint16_t position;
    AS5048_ErrorFlags errorFlags;
} AS5048_ReadResult;

/**
 * @brief Read value from AS5048A register
 * @param regAddr 14-bit register address
 * @param dst Pointer to store the result
 * @return Status of the operation (AS5048_OK, AS5048_ERR_SPI, etc.)
 */
AS5048_Status AS5048_Reg_Read(uint16_t regAddr, uint16_t *dst);

/**
 * @brief Write value to AS5048A register
 * @param regAddr 14-bit register address
 * @param value Data to write into register
 * @param confirm Pointer to store the send comfirmation
 * @return Status of the operation (AS5048_OK, AS5048_ERR_SPI, etc.)
 */
AS5048_Status AS5048_Reg_Write(uint16_t regAddr, uint16_t value, uint16_t *confirm);

/**
 * @brief Get error details
 * @return ErrorFlag of error(watchdogError, offsetfinished, cordicOverflow)
 */
AS5048_ErrorFlags AS5048_Get_Error_Details(void);

void AS5048_Get_Raw_Position(AS5048_ReadResult *raw_angle);

/**
 * @brief Add parity bit value into command to send
 * @param cmd Command to send
 * @return Command to send with parity bit
 */
uint16_t AS5048_Add_Parity(uint16_t cmd);

/**
 * @brief Check error on 14th bit in received frame
 * @param response Received data
 * @return Bool 1 -> ERROR
 */
bool AS5048_Has_Error(uint16_t response);

/**
 * @brief Convert raw angle to degrees
 * @return float, -1.0f when raw.status != AS5048_OK
 */
float AS5048_Get_Angle_Deg(void);

void AS5048_Diagnose(void);

#endif /* INC_AS5048A_H_ */
