/*
 * as5048a.c
 *
 *  Created on: Apr 9, 2025
 *      Author: Miloush
 */

#include "BSP/as5048a.h"
#include "App/config.h"
#include "FOC/foc_loop.h"
#include <stdio.h>

static SPI_HandleTypeDef* as5048_hspi;

volatile bool spi_ready = false;

static uint8_t spi_tx_buf[2];
static uint8_t spi_rx_buf[2];

volatile AS5048_ReadResult raw_angle;

volatile bool new_encoder_data_ready = false;

//volatile float theta_el_last = 0.0f;
//volatile float theta_mech_last = 0.0f;

static inline void AS5048_CS_LOW(void)  { HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET); }
static inline void AS5048_CS_HIGH(void) { HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET); }

void AS5048_Init(SPI_HandleTypeDef *hspi){
	as5048_hspi = hspi;
	DWT_Init();
	AS5048_CS_HIGH();
	spi_ready = true;
}

static uint16_t AS5048_AddParity(uint16_t cmd)
{
    uint16_t count = 0;
    for (int i = 0; i < 15; i++)
        count += (cmd >> i) & 1u;

    if (count % 2)
        cmd |= 0x8000;
    else
        cmd &= ~0x8000;

    return cmd;
}

static bool AS5048_HasError(uint16_t response) {return (response & AS_ERROR_BIT);}

static AS5048_Status AS5048_TransceiveReceive(const uint8_t *tx, uint8_t *rx)
{
    AS5048_CS_LOW();
    delay_us(AS_US_DELAY/2);
    HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(as5048_hspi, tx, rx, 2, HAL_MAX_DELAY);
    AS5048_CS_HIGH();
    delay_us(AS_US_DELAY);
    return (result == HAL_OK) ? AS5048_OK : AS5048_ERR_SPI;
}

static AS5048_Status AS5048_Transceive(const uint8_t *tx)
{
    AS5048_CS_LOW();
    delay_us(AS_US_DELAY/2);
    HAL_StatusTypeDef result = HAL_SPI_Transmit(as5048_hspi, tx, 2, HAL_MAX_DELAY);
    AS5048_CS_HIGH();
    delay_us(AS_US_DELAY);
    return (result == HAL_OK) ? AS5048_OK : AS5048_ERR_SPI;
}

static AS5048_Status AS5048_RegRead(const uint16_t regAddr, uint16_t *dst)
{
    AS5048_Status s;
    uint16_t cmd = AS_READ | (regAddr & AS_ANGLE);
    cmd = AS5048_AddParity(cmd);

    uint8_t txBuf[2] = {cmd >> 8, cmd & 0xFF };
    uint8_t rxBuf[2] = {0};

    s = AS5048_TransceiveReceive(txBuf, rxBuf);
    if (s != AS5048_OK) return s;

    uint8_t txBuf2[2] = {0,0};
    uint8_t rxBuf2[2] = {0};

    s = AS5048_TransceiveReceive(txBuf2, rxBuf2);
    if (s != AS5048_OK) return s;

    uint16_t rx_data = ((uint16_t)rxBuf2[0] << 8) | rxBuf2[1];
    if (regAddr != AS_CLR_ERR && AS5048_HasError(rx_data))
        return AS5048_ERR_FLAG;

    *dst = rx_data & AS_ANGLE; // 14 bitów właściwych danych
    return AS5048_OK;
}

static AS5048_Status AS5048_RegWrite(const uint16_t regAddr, const uint16_t value, uint16_t *confirm){

	AS5048_Status s;

	uint16_t cmd = AS_WRITE | (regAddr & AS_ANGLE);
    cmd = AS5048_AddParity(cmd);
    uint8_t txCmd[2] = {cmd >> 8, cmd & 0xFF};

    s = AS5048_Transceive(txCmd);
    if(s != AS5048_OK) return s;

    uint8_t txData[2] = {value >> 8, value & 0xFF};

    s = AS5048_Transceive(txData);
    if(s != AS5048_OK) return s;

    uint8_t txBuf3[2] = {0x00, 0x00}, rxBuf3[2] = {0};

    s = AS5048_TransceiveReceive(txBuf3, rxBuf3);
    if(s != AS5048_OK) return s;

    *confirm = (rxBuf3[0] << 8) | rxBuf3[1];
    if (AS5048_HasError(*confirm)) return AS5048_ERR_FLAG;
    return AS5048_OK;
}

AS5048_ErrorFlags AS5048_GetErrorDetails(void) {
//    printf("[ERROR] Rozpoczynam odczyt rejestru błędów (0x0001)\n");

    AS5048_ErrorFlags err = {0};
    uint16_t response = 0;
    AS5048_RegRead(AS_CLR_ERR, &response);
    uint16_t reg = response & 0x3FFF;

    err.watchdogError   = reg & (1 << 0);
    err.offsetFinished  = reg & (1 << 1);
    err.cordicOverflow  = reg & (1 << 2);

//    printf("[ERROR] Rejestr błędów: 0x%04X\n", reg);
//    printf("        → Watchdog: %s\n", err.watchdogError ? "TAK" : "nie");
//    printf("        → Offset finished: %s\n", err.offsetFinished ? "TAK" : "nie");
//    printf("        → CORDIC overflow: %s\n", err.cordicOverflow ? "TAK" : "nie");

    // CLEAR ERROR FLAG mechanizm z dokumentacji: kolejny odczyt kasuje flagę
    uint16_t dummy;
    AS5048_RegRead(AS_NOP, &dummy);

    return err;
}

void AS5048_GetRawPosition(void) {

    uint16_t raw = 0;
    AS5048_Status status = AS5048_ERR_SPI;

    for (int attempt = 0; attempt < 3; attempt++) {
//        printf("\n[TRY] Próba odczytu kąta #%d\n", attempt + 1);
        status = AS5048_RegRead(AS_ANGLE, &raw);
        if (status == AS5048_OK) break;
        delay_us(20);
    }

    raw_angle.status = status;
    if (status == AS5048_OK) {
        raw_angle.position = raw;
//        printf("[INFO] Prawidłowy odczyt kąta: %u\n", raw);
    } else if (status == AS5048_ERR_FLAG) {
        raw_angle.errorFlags = AS5048_GetErrorDetails();
    } else {
//        printf("[ERROR] Błąd SPI podczas odczytu kąta\n");
    }
}

float AS5048_GetAngleDeg(void) {
    AS5048_GetRawPosition();

    if (raw_angle.status != AS5048_OK) {
        return -1.0f;
    }

    float angle_deg = ((float)raw_angle.position * 360.0f) / AS5048_RESOLUTION;
    return angle_deg;
}

float AS5048_GetAngleRad(void){
    AS5048_GetRawPosition();

    if (raw_angle.status != AS5048_OK) {
        return -1.0f;
    }

    float angle_rad = ((float)raw_angle.position / AS5048_RESOLUTION) * M_TWOPI;
    return angle_rad;
}

// Transmisja przez DMA
void AS5048_ReadAngleDMA(void)
{
	if (!spi_ready) return; // trwa poprzedni transfer

	spi_ready = false;
	uint16_t cmd = AS_READ | AS_ANGLE;
	cmd  = AS5048_AddParity(cmd);

	spi_tx_buf[0] = (uint8_t)(cmd >> 8);
	spi_tx_buf[1] = (uint8_t)(cmd & 0xFF);

	AS5048_CS_LOW();
	HAL_SPI_TransmitReceive_DMA(as5048_hspi, spi_tx_buf, spi_rx_buf, 2);
}

float AS5048_GetMechanicalAngle(void) {return (float)raw_angle.position / AS5048_RESOLUTION * M_TWOPI;}

float AS5048_GetMechanicalAngleShifted(void) {return ((float)raw_angle.shifted_pos / (AS5048_RESOLUTION >> AS5048_DECIMATION)) * M_TWOPI;}

// Callback wywoływany po zakończeniu transmisji po DMA
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == as5048_hspi)
	{
		AS5048_CS_HIGH();
		uint16_t frame = ((uint16_t)spi_rx_buf[0] << 8) | spi_rx_buf[1];

		spi_ready = true;

		if (frame & AS_ERROR_BIT) {
//			raw_angle.errorFlags = AS5048_GetErrorDetails(); // To funkcja blokująca, nie powinno jej tu być
			raw_angle.status = AS5048_ERR_FLAG;
		} else {
			raw_angle.position = frame & AS_ANGLE;
			raw_angle.shifted_pos = raw_angle.position >> AS5048_DECIMATION;
			raw_angle.status = AS5048_OK;

			new_encoder_data_ready = true;
		}
	}
}

//void AS5048_Diagnose(void) {0
//    uint16_t agc = 0, mag = 0, diag = 0;
//
//    printf("\n=== DIAGNOSTYKA AS5048A ===\n");
//
//    if (AS5048_Reg_Read(AS_DIAG_AGC, &agc) == AS5048_OK)
//        printf("[AGC] Automatic Gain Control: %3u\n", agc);
//    else
//        printf("[AGC] Błąd odczytu rejestru\n");
//
//    if (AS5048_Reg_Read(AS_MAGNITUDE, &mag) == AS5048_OK)
//        printf("[MAG] Magnituda pola (14 bit): %5u\n", mag);
//    else
//        printf("[MAG] Błąd odczytu rejestru\n");
//
//    if (AS5048_Reg_Read(0x0017, &diag) == AS5048_OK) {
//        printf("[DIAG] 0x%04X → ", diag);
//        printf("OCF=%d, COF=%d, COMP_low=%d, COMP_high=%d\n",
//            (diag >> 3) & 0x01,  // OCF (Offset Compensation Finished)
//            (diag >> 2) & 0x01,  // COF (CORDIC Overflow)
//            (diag >> 1) & 0x01,  // COMP_low (komparator niskiego poziomu)
//            diag & 0x01          // COMP_high (komparator wysokiego poziomu)
//        );
//    } else {
//        printf("[DIAG] Błąd odczytu rejestru\n");
//    }
//
//    printf("===========================\n\n");
//}

