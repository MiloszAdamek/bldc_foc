/*
 * as5048a.c
 *
 *  Created on: Apr 9, 2025
 *      Author: Miloush
 */

#include "as5048a.h"

uint16_t AS5048_Add_Parity(uint16_t cmd) {
    uint16_t count = 0;
    uint16_t temp = cmd;

    while (temp) {
        count += temp & 1;
        temp >>= 1;
    }

    if (count % 2 == 0) {
        cmd |= 1; // ustaw LSB na 1 żeby było nieparzyście
    } else {
        cmd &= ~1; // wyczyść LSB (już jest nieparzyście)
    }

    return cmd;
}

AS5048_Status AS5048_Reg_Read(uint16_t regAddr, uint16_t *dst) {

	// READ command
    uint16_t cmd = 0x4000 | (regAddr & 0x3FFF);
    cmd = AS5048_Add_Parity(cmd);

    uint8_t txBuf[2] = {cmd >> 8, cmd & 0xFF};
    uint8_t rxBuf[2] = {0};

    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_TransmitReceive(&hspi3, txBuf, rxBuf, 2, HAL_MAX_DELAY) != HAL_OK) {
        HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);
        return AS5048_ERR_SPI;
    }
    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

    HAL_Delay(1);

    // NOP to push response
    uint8_t txBuf2[2] = {0x00, 0x00};
    uint8_t rxBuf2[2] = {0};

    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_TransmitReceive(&hspi3, txBuf2, rxBuf2, 2, HAL_MAX_DELAY) != HAL_OK) {
        HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);
        return AS5048_ERR_SPI;
    }
    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

    uint16_t response = (rxBuf2[0] << 8) | rxBuf2[1];
    if (AS5048_Has_Error(response)) return AS5048_ERR_FLAG; // Check ERROR bit

    *dst = response & 0x3FFF; // dane 14 biotwe, Bit 13-0
    return AS5048_OK;
}

AS5048_Status AS5048_Reg_Write(uint16_t regAddr, uint16_t value, uint16_t *confirm) {

	// WRITE command
    uint16_t cmd = 0x0000 | (regAddr & 0x3FFF);
    cmd = AS5048_Add_Parity(cmd);
    uint8_t txCmd[2] = {cmd >> 8, cmd & 0xFF};

    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit(&hspi3, txCmd, 2, HAL_MAX_DELAY) != HAL_OK) {
        HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);
        return AS5048_ERR_SPI;
    }
    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

    HAL_Delay(1);

    // Send command
    uint8_t txData[2] = {value >> 8, value & 0xFF};
    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit(&hspi3, txData, 2, HAL_MAX_DELAY) != HAL_OK) {
        HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);
        return AS5048_ERR_SPI;
    }
    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

    HAL_Delay(1);

    // Confirmation
    uint8_t txBuf3[2] = {0x00, 0x00}, rxBuf3[2] = {0};
    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_TransmitReceive(&hspi3, txBuf3, rxBuf3, 2, HAL_MAX_DELAY) != HAL_OK) {
        HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);
        return AS5048_ERR_SPI;
    }
    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

    *confirm = (rxBuf3[0] << 8) | rxBuf3[1];
    if (AS5048_Has_Error(*confirm)) return AS5048_ERR_FLAG; // Check ERROR bit

    return AS5048_OK;
}

bool AS5048_Has_Error(uint16_t response) {
    return (response & 0x4000); // bit 14 = 1 → ERROR
}

AS5048_ErrorFlags AS5048_Get_Error_Details(void) {
    AS5048_ErrorFlags err = {0};
    uint16_t reg = 0;
    if (AS5048_Reg_Read(0x0001, &reg) != AS5048_OK) return err;

    err.watchdogError   = reg & (1 << 0);
    err.offsetFinished  = reg & (1 << 1);
    err.cordicOverflow  = reg & (1 << 2);
    return err;
}

AS5048_ReadResult AS5048_Get_Raw_Position(void) {
    AS5048_ReadResult result = { .status = AS5048_OK, .position = 0 };

    uint16_t raw = 0;
    AS5048_Status status = AS5048_Reg_Read(AS_ANGLE, &raw);

    result.status = status;
    if (status == AS5048_OK) {
        result.position = raw;
    } else if (status == AS5048_ERR_FLAG) {
        result.errorFlags = AS5048_Get_Error_Details();
    }

    return result;
}
