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
#include "encoder_types.h"

void EncoderHub_Init(void);

/* Wywoływane TYLKO z FOC 10kHz - zwraca true jeśli nowa próbka */
bool EncoderHub_ConsumeSample(EncoderSample_t *out);

/* Wywoływane z FOC 10kHz po wyliczeniu kąta el - publikuje snapshot */
void EncoderHub_PublishAngle(float theta_mech, float theta_el);

/* Wywoływane z dowolnego kontekstu (Speed 1kHz, Position 200Hz) */
AngleSnapshot_t EncoderHub_GetAngleSnapshot(void);

/* Wywoływane z przerwania od DMA AS5048A lub z obsługi enkodera inkrementalnego */
void EncoderHub_PublishSample(const EncoderSample_t *sample);

#endif /* INC_FOC_ENCODER_HUB_H_ */
