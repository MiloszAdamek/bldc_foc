/*
 * encoder_hub.c
 *
 *  Created on: 4 Apr 2026
 *      Author: miloush
 */

 
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "encoder_hub.h"
#include "main.h"


/* ------------------------------------------------------------------ */
/*  Wewnętrzny double-buffer: DMA pisze do [write_idx],               */
/*  FOC czyta z [write_idx ^ 1] po zamianie atomowej                  */
/* ------------------------------------------------------------------ */
static EncoderSample_t  s_buf[2];
static volatile uint8_t s_write_idx = 0;   /* DMA pisze tutaj        */
static volatile bool    s_new_sample = false;

/* Snapshot kąta (seqlock) */
static AngleSnapshot_t  s_angle = {0};

/* ------------------------------------------------------------------ */

void EncoderHub_Init(void)
{
    memset(s_buf, 0, sizeof(s_buf));
    s_write_idx = 0;
    s_new_sample = false;
    memset(&s_angle, 0, sizeof(s_angle));
}

/* Wywoływane TYLKO z FOC 10kHz */
bool EncoderHub_ConsumeSample(EncoderSample_t *out)
{
    if (!s_new_sample) return false;

    /* Czytamy z "poprzedniego" bufora (nie tego, do którego pisze DMA) */
    uint8_t read_idx = s_write_idx ^ 1u;
    *out = s_buf[read_idx];

    s_new_sample = false;
    return out->valid;
}

/* Wywoływane z przerwania od DMA AS5048A lub z obsługi enkodera inkrementalnego */
void EncoderHub_PublishSample(const EncoderSample_t *sample)
{
    uint8_t idx = s_write_idx;

    /* Zapisujemy kompletną próbkę do aktualnego bufora */
    s_buf[idx] = *sample;

    /*
     * Upewniamy się, że zapis struktury zakończył się
     * przed zmianą indeksu i ustawieniem flagi.
     */
    __DMB();

    s_write_idx = idx ^ 1u;
    s_new_sample = true;
}

/* Wywoływane z FOC 10kHz po wyliczeniu kątów */
void EncoderHub_PublishAngle(float theta_mech, float theta_el)
{
    s_angle.seq++;
    __DMB();
    s_angle.theta_mech = theta_mech;
    s_angle.theta_el   = theta_el;
    s_angle.tick       = DWT->CYCCNT;
    __DMB();
    s_angle.seq++;
}

/* Wywoływane z niższych priorytetów - seqlock read */
AngleSnapshot_t EncoderHub_GetAngleSnapshot(void)
{
    AngleSnapshot_t tmp;
    uint32_t s1, s2;
    do {
        s1 = s_angle.seq;
        __DMB();
        tmp = s_angle;
        __DMB();
        s2 = s_angle.seq;
    } while ((s1 != s2) || (s1 & 1u));
    return tmp;
}
