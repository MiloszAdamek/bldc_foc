/*
 * as5048a.c
 *
 *  Created on: Apr 9, 2025
 *      Author: Miloush
 */

#include "as5048a.h"

void AS5048_Init(){

	HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
}

void AS5048_Reg_Read(uint16_t reg_Adrr, uint16_t *dst){

	uint16_t cmd = 0x4000 | (reg_Adrr & 0x3FFF); // PAR = 0 R/W=R
	cmd = cmd | reg_adr;
	cmd = AS5048_Add_Parity(cmd);

	uint8_t txBuf[2] = {cmd >> 8, cmd & 0xFF};
	uint8_t rxBuf[2] = {0};

	HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi3, txBuf, rxBuf, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

	HAL_Delay(1);

	uint8_t txBuf2[2] = {0x00, 0x00};
	uint8_t rxBuf2[2] = {0};

    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi3, txBuf2, rxBuf2, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

	*dst = (rxBuf2[0] << 8) | rxBuf2[1];
}

void AS5048_Reg_Write(uint16_t reg, uint16_t value, uint16_t *dst){

	uint16_t cmd = 0x0000 | (reg_Adrr & 0x3FFF); // PAR = 0 R/W = W
	cmd = cmd | reg_adr;
	cmd = AS5048_Add_Parity(cmd);

	HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(IOEXP_CS_GPIO_Port, IOEXP_CS_Pin, GPIO_PIN_RESET);
}

void AS5048_Set_Zero_Position(){

}

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

