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






#endif /* INC_AS5048A_H_ */
