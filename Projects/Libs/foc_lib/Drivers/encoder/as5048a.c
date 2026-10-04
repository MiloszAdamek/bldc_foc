/*
 * as5048a.c
 *
 *  Created on: Apr 9, 2025
 *      Author: Milosz Adamek
 */



#include "as5048a.h"
#include "encoder_hub.h"
#include <stdio.h>
#include "delay_us.h"
#include "math_consts.h"

static SPI_HandleTypeDef* s_hspi;

volatile bool spi_ready = false;
volatile bool theta_ready = false;

static uint8_t s_tx[2];
static uint8_t s_rx[2];

volatile AS5048_ReadResult raw_angle;

#if defined(USE_AS5048A_ENCODER)
static inline void CS_LOW(void)  { HAL_GPIO_WritePin(AS5048A_CS_GPIO_Port, AS5048A_CS_Pin, GPIO_PIN_RESET); }
static inline void CS_HIGH(void) { HAL_GPIO_WritePin(AS5048A_CS_GPIO_Port, AS5048A_CS_Pin, GPIO_PIN_SET); }
#else
static inline void CS_HIGH(void) {};
static inline void CS_LOW(void) {};
#endif

static void AS5048_ClearError();

void AS5048_Init(SPI_HandleTypeDef *hspi)
{
	s_hspi = hspi;
	DWT_Init();

	CS_HIGH();

    HAL_Delay(100);

    AS5048_ClearError();

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
    CS_LOW();

    delay_us(AS_US_DELAY/2);
    HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(s_hspi, tx, rx, 2, HAL_MAX_DELAY);

    CS_HIGH();

    delay_us(AS_US_DELAY);
    return (result == HAL_OK) ? AS5048_OK : AS5048_ERR_SPI;
}

static AS5048_Status AS5048_Transceive(const uint8_t *tx)
{
    CS_LOW();

    delay_us(AS_US_DELAY/2);
    HAL_StatusTypeDef result = HAL_SPI_Transmit(s_hspi, tx, 2, HAL_MAX_DELAY);

    CS_HIGH();

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

    // printf("CMD=%02X %02X\n", txBuf[0], txBuf[1]);
    // printf("regAddr=0x%04X\n", regAddr);
    // printf("RX1=%02X %02X\n", rxBuf[0], rxBuf[1]);
    // printf("RX2=%02X %02X\n", rxBuf2[0], rxBuf2[1]);

    uint16_t rx_data = ((uint16_t)rxBuf2[0] << 8) | rxBuf2[1];
    if (regAddr != AS_CLR_ERR && AS5048_HasError(rx_data))
        return AS5048_ERR_FLAG;

    *dst = rx_data & AS_ANGLE; // 14 bitów właściwych danych
    return AS5048_OK;
}

static AS5048_Status AS5048_RegWrite(const uint16_t regAddr, const uint16_t value, uint16_t *confirm)
{

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

static void AS5048_ClearError() 
{
    uint16_t response = 0;
    AS5048_RegRead(AS_CLR_ERR, &response);
    AS5048_RegRead(AS_NOP, &response);
}

AS5048_ErrorFlags AS5048_GetErrorDetails(void)
{
//    printf("[ERROR] Rozpoczynam odczyt rejestru błędów (0x0001)\n");

    AS5048_ErrorFlags err = {0};
    uint16_t response = 0;
    AS5048_RegRead(AS_CLR_ERR, &response);
    uint16_t reg = response & 0x3FFF;

    err.framingError   = reg & (1 << 0);
    err.commandInvalid  = reg & (1 << 1);
    err.parityError  = reg & (1 << 2);

    printf("[ERROR] Rejestr błędów: 0x%04X\n", reg);
    printf("        → Framing: %s\n", err.framingError ? "TAK" : "nie");
    printf("        → Command invalid: %s\n", err.commandInvalid ? "TAK" : "nie");
    printf("        → Parity: %s\n", err.parityError ? "TAK" : "nie");

    // CLEAR ERROR FLAG mechanizm z dokumentacji: kolejny odczyt kasuje flagę
    uint16_t dummy;
    AS5048_RegRead(AS_NOP, &dummy);

    return err;
}

void AS5048_GetRawPosition(void)
{

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

float AS5048_GetAngleDeg(void)
{
    AS5048_GetRawPosition();

    if (raw_angle.status != AS5048_OK) {
        return -1.0f;
    }

    float angle_deg = ((float)raw_angle.position * 360.0f) / AS5048_RESOLUTION;
    return angle_deg;
}

float AS5048_GetAngleRad(void)
{
    AS5048_GetRawPosition();

    if (raw_angle.status != AS5048_OK) {
        return -1.0f;
    }

    float angle_rad = ((float)raw_angle.position / AS5048_RESOLUTION) * M_TWOPI;
    return angle_rad;
}

bool AS5048_TryGetMechanicalAngle(float *theta_rad)
{
    if (!theta_ready) return false;
    if (raw_angle.status != AS5048_OK) return false;

    theta_ready = false; // „konsumujesz” próbkę
    *theta_rad = (float)raw_angle.position / AS5048_RESOLUTION * M_TWOPI;
    return true;
}

float AS5048_GetMechanicalAngle(void) {return (float)raw_angle.position / AS5048_RESOLUTION * M_TWOPI;}

float AS5048_GetMechanicalAngleShifted(void) {return ((float)raw_angle.shifted_pos / (AS5048_RESOLUTION >> AS5048_DECIMATION)) * M_TWOPI;}

// Transmisja przez DMA
void AS5048_ReadAngleDMA(void)
{
	spi_ready = false;
	uint16_t cmd = AS_READ | AS_ANGLE;
	cmd  = AS5048_AddParity(cmd);

	s_tx[0] = (uint8_t)(cmd >> 8);
	s_tx[1] = (uint8_t)(cmd & 0xFF);

	CS_LOW();
	HAL_SPI_TransmitReceive_DMA(s_hspi, s_tx, s_rx, 2);
}

/* Wywoływane z HAL_SPI_TxRxCpltCallback - kontekst IRQ */
void AS5048A_OnDmaComplete(const uint8_t *rx_buf)
{
    uint16_t frame =
        ((uint16_t)rx_buf[0] << 8) | rx_buf[1];

    if (frame & AS_ERROR_BIT)
        return;

    uint16_t raw = frame & AS_ANGLE;

    EncoderSample_t sample = {
        .raw = raw,
        .theta_mech = (float)raw / AS5048_RESOLUTION * M_TWOPI,
        .tick = DWT->CYCCNT,
        .valid = true
    };

    EncoderHub_PublishSample(&sample);
}

// Callback wywoływany po zakończeniu transmisji po DMA
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == s_hspi)
	{
		CS_HIGH();

		AS5048A_OnDmaComplete(s_rx);

		spi_ready = true;
//		SPI_Flag_GPIO_Port->BSRR = (uint32_t)SPI_Flag_Pin << 16; // GPIO PC9 reset, debug
	}
}
