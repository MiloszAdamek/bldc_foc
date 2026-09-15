/*
 * encoder_hub.h
 *
 *  Created on: 4 Apr 2026
 *      Author: miloush
 */

#ifndef INC_FOC_ENCODER_HUB_H_
#define INC_FOC_ENCODER_HUB_H_
#pragma once
#include <stdint.h>
#include <stdbool.h>

/* Surowa próbka z enkodera - wypełniana TYLKO przez SPI DMA callback */
typedef struct {
    uint16_t  raw;          /* 14-bit pozycja */
    float     theta_mech;   /* [rad] 0..2pi */
    uint32_t  tick;         /* DWT timestamp przy odebraniu */
    bool      valid;
} EncoderSample_t;

/* Snapshot kąta publikowany przez pętlę FOC */
typedef struct {
    volatile uint32_t seq;
    float     theta_mech;   /* [rad] mechaniczny */
    float     theta_el;     /* [rad] elektryczny  */
    uint32_t  tick;
} AngleSnapshot_t;

/* API */
void              EncoderHub_Init(void);

/* Wywoływane TYLKO z HAL_SPI_TxRxCpltCallback */
void              EncoderHub_OnDmaComplete(const uint8_t *rx_buf);

/* Wywoływane TYLKO z FOC 10kHz - zwraca true jeśli nowa próbka */
bool              EncoderHub_ConsumeSample(EncoderSample_t *out);

/* Wywoływane z FOC 10kHz po wyliczeniu kąta el - publikuje snapshot */
void              EncoderHub_PublishAngle(float theta_mech, float theta_el);

/* Wywoływane z dowolnego kontekstu (Speed 1kHz, Position 200Hz) */
AngleSnapshot_t   EncoderHub_GetAngleSnapshot(void);

#endif /* INC_FOC_ENCODER_HUB_H_ */
