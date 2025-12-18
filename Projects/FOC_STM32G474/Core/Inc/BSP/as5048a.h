/*
 * as5048a.h
 *
 *  Created on: Apr 9, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_AS5048A_H_
#define INC_AS5048A_H_

#include "App/config.h"
#include "BSP/delay_us.h"
#include "main.h"
#include <stdbool.h>
#include "math.h"

#define AS_WRITE            0x0000  // bit14=0 -> zapis
#define AS_READ             0x4000  // bit14=1 -> odczyt
#define AS_NOP              0x0000  // „No Operation” – druga ramka, żeby odebrać dane
#define AS_CLR_ERR          0x0001  // odczyt tego kasuje flagę błędów
#define AS_PROG             0x0003  // programowanie OTP
#define AS_OTP_ZERO_POS_H   0x0016  // wysokie bity pozycji zerowej
#define AS_OTP_ZERO_POS_L   0x0017  // niskie bity pozycji zerowej
#define AS_DIAG_AGC         0x3FFD  // AGC
#define AS_MAGNITUDE        0x3FFE  // pole magnetyczne
#define AS_ANGLE            0x3FFF  // aktualny kąt (14 bitów danych)
#define AS_ERROR_BIT 		0x4000

#define AS_US_DELAY			4

#define AS5048_RESOLUTION   16384

# define AS5048_DECIMATION	4 // 14 bit -> 12 bit

extern volatile bool spi_ready;
extern volatile bool new_encoder_data_ready;

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
    uint16_t shifted_pos;
    AS5048_ErrorFlags errorFlags;
} AS5048_ReadResult;

extern volatile AS5048_ReadResult raw_angle;
extern volatile bool g_spi_ready;
extern volatile bool g_new_encoder_data_ready;

AS5048_ErrorFlags AS5048_GetErrorDetails(void);

void AS5048_GetRawPosition(void);

extern float el_from_mech(float mech);

float AS5048_GetAngleDeg(void);

float AS5048_GetAngleRad(void);

void AS5048_Diagnose(void);

void AS5048_ReadAngleDMA(void);

float AS5048_GetMechanicalAngle(void);

float AS5048_GetMechanicalAngleShifted(void);

void AS5048_Init();

#endif /* INC_AS5048A_H_ */
