/*
 * as5048a.c
 *
 *  Created on: Apr 9, 2025
 *      Author: Miloush
 */

#include "as5048a.h"
#include <stdio.h>

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

    printf("\n[READ] Rejestr: 0x%04X\n", regAddr);

    uint16_t cmd = 0x4000 | (regAddr & 0x3FFF);
    cmd = AS5048_Add_Parity(cmd);

    uint8_t txBuf[2] = {cmd >> 8, cmd & 0xFF};
    uint8_t rxBuf[2] = {0};

    printf("[SPI] TX (read cmd): %02X %02X\n", txBuf[0], txBuf[1]);

    AS5048_CS_LOW();
    delay_us(1);
    if (HAL_SPI_TransmitReceive(AS5048_SPI_HANDLE, txBuf, rxBuf, 2, HAL_MAX_DELAY) != HAL_OK) {
        AS5048_CS_HIGH();
        printf("[SPI] Błąd transmisji komendy\n");
        return AS5048_ERR_SPI;
    }
    AS5048_CS_HIGH();

    printf("[SPI] RX (read dummy): %02X %02X\n", rxBuf[0], rxBuf[1]);

    delay_us(5);

    uint8_t txBuf2[2] = {0x00, 0x00};
    uint8_t rxBuf2[2] = {0};

    printf("[SPI] TX (NOP): %02X %02X\n", txBuf2[0], txBuf2[1]);

    AS5048_CS_LOW();
    delay_us(1);
    if (HAL_SPI_TransmitReceive(AS5048_SPI_HANDLE, txBuf2, rxBuf2, 2, HAL_MAX_DELAY) != HAL_OK) {
        AS5048_CS_HIGH();
        printf("[SPI] Błąd transmisji NOP\n");
        return AS5048_ERR_SPI;
    }
    AS5048_CS_HIGH();

    printf("[SPI] RX (data): %02X %02X\n", rxBuf2[0], rxBuf2[1]);

    uint16_t response = (rxBuf2[0] << 8) | rxBuf2[1];
    if (AS5048_Has_Error(response)) {
        printf("[ERROR] Bit błędu ustawiony w odpowiedzi: 0x%04X\n", response);
        return AS5048_ERR_FLAG;
    }

    *dst = response & 0x3FFF;
    printf("[READ] Dane: 0x%04X (%u)\n", *dst, *dst);
    return AS5048_OK;
}


AS5048_Status AS5048_Reg_Write(uint16_t regAddr, uint16_t value, uint16_t *confirm) {

	// WRITE command
    uint16_t cmd = 0x0000 | (regAddr & 0x3FFF);
    cmd = AS5048_Add_Parity(cmd);
    uint8_t txCmd[2] = {cmd >> 8, cmd & 0xFF};

    AS5048_CS_LOW();
    if (HAL_SPI_Transmit(AS5048_SPI_HANDLE, txCmd, 2, HAL_MAX_DELAY) != HAL_OK) {
        AS5048_CS_HIGH();
        return AS5048_ERR_SPI;
    }
    AS5048_CS_HIGH();

    delay_us(5);

    // Send command
    uint8_t txData[2] = {value >> 8, value & 0xFF};
    AS5048_CS_LOW();
    if (HAL_SPI_Transmit(AS5048_SPI_HANDLE, txData, 2, HAL_MAX_DELAY) != HAL_OK) {
        AS5048_CS_HIGH();
        return AS5048_ERR_SPI;
    }
    AS5048_CS_HIGH();

    HAL_Delay(1);

    // Confirmation
    uint8_t txBuf3[2] = {0x00, 0x00}, rxBuf3[2] = {0};
    AS5048_CS_LOW();
    if (HAL_SPI_TransmitReceive(AS5048_SPI_HANDLE, txBuf3, rxBuf3, 2, HAL_MAX_DELAY) != HAL_OK) {
        AS5048_CS_HIGH();
        return AS5048_ERR_SPI;
    }
    AS5048_CS_HIGH();

    *confirm = (rxBuf3[0] << 8) | rxBuf3[1];
    if (AS5048_Has_Error(*confirm)) return AS5048_ERR_FLAG; // Check ERROR bit

    return AS5048_OK;
}

bool AS5048_Has_Error(uint16_t response) {
    return (response & 0x4000); // bit 14 = 1 → ERROR
}

AS5048_ErrorFlags AS5048_Get_Error_Details(void) {

    printf("[ERROR] Rozpoczynam odczyt rejestru błędów (0x0001)\n");

    AS5048_ErrorFlags err = {0};
    uint16_t reg = 0;
    if (AS5048_Reg_Read(0x0001, &reg) != AS5048_OK) {
        printf("[ERROR] Nie udało się odczytać rejestru błędów!\n");
        return err;
    }

    err.watchdogError   = reg & (1 << 0);
    err.offsetFinished  = reg & (1 << 1);
    err.cordicOverflow  = reg & (1 << 2);

    printf("[ERROR] Rejestr błędów: 0x%04X\n", reg);
    printf("        → Watchdog: %s\n", err.watchdogError ? "TAK" : "nie");
    printf("        → Offset finished: %s\n", err.offsetFinished ? "TAK" : "nie");
    printf("        → CORDIC overflow: %s\n", err.cordicOverflow ? "TAK" : "nie");

    return err;
}


void AS5048_Get_Raw_Position(AS5048_ReadResult *raw_angle) {

    *raw_angle = (AS5048_ReadResult){0};  // zerowanie struktury

    uint16_t raw = 0;
    AS5048_Status status = AS5048_ERR_SPI;

    for (int attempt = 0; attempt < 3; attempt++) {
        printf("\n[TRY] Próba odczytu kąta #%d\n", attempt + 1);
        status = AS5048_Reg_Read(AS_ANGLE, &raw);
        if (status == AS5048_OK) break;
        delay_us(10);
    }

    raw_angle->status = status;
    if (status == AS5048_OK) {
        raw_angle->position = raw;
        printf("[INFO] Prawidłowy odczyt kąta: %u\n", raw);
    } else if (status == AS5048_ERR_FLAG) {
        raw_angle->errorFlags = AS5048_Get_Error_Details();
    } else {
        printf("[ERROR] Błąd SPI podczas odczytu kąta\n");
    }
}

float AS5048_Get_Angle_Deg(void) {
    AS5048_ReadResult raw = {0};
    AS5048_Get_Raw_Position(&raw);

    if (raw.status != AS5048_OK) {
        return -1.0f; // lub możesz zwrócić np. NAN, jeśli masz <math.h>
    }

    return ((float)raw.position * 360.0f) / 16384.0f; // 2^14 = 16384
}

void AS5048_Diagnose(void) {
    uint16_t agc = 0, mag = 0, diag = 0;

    printf("\n=== DIAGNOSTYKA AS5048A ===\n");

    if (AS5048_Reg_Read(AS_DIAG_AGC, &agc) == AS5048_OK)
        printf("[AGC] Automatic Gain Control: %3u\n", agc);
    else
        printf("[AGC] Błąd odczytu rejestru\n");

    if (AS5048_Reg_Read(AS_MAGNITUDE, &mag) == AS5048_OK)
        printf("[MAG] Magnituda pola (14 bit): %5u\n", mag);
    else
        printf("[MAG] Błąd odczytu rejestru\n");

    if (AS5048_Reg_Read(0x0017, &diag) == AS5048_OK) {
        printf("[DIAG] 0x%04X → ", diag);
        printf("OCF=%d, COF=%d, COMP_low=%d, COMP_high=%d\n",
            (diag >> 3) & 0x01,  // OCF (Offset Compensation Finished)
            (diag >> 2) & 0x01,  // COF (CORDIC Overflow)
            (diag >> 1) & 0x01,  // COMP_low (komparator niskiego poziomu)
            diag & 0x01          // COMP_high (komparator wysokiego poziomu)
        );
    } else {
        printf("[DIAG] Błąd odczytu rejestru\n");
    }

    printf("===========================\n\n");
}

