/*
 * encoder_hub.c
 *
 *  Created on: 4 Apr 2026
 *      Author: miloush
 */

#include <stdint.h>
#include <stdbool.h>

#include "BSP/encoder_hub.h"
#include "FOC/foc_utils.h"
#include "App/config.h"
#include "main.h"
#include <string.h>

#define AS_ANGLE     0x3FFFu
#define AS_ERROR_BIT 0x4000u

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

/* Wywoływane z HAL_SPI_TxRxCpltCallback - kontekst IRQ */
void EncoderHub_OnDmaComplete(const uint8_t *rx_buf)
{
    uint16_t frame = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];

    if (frame & AS_ERROR_BIT) {
        /* Nie aktualizuj bufora - zostaw poprzednią próbkę */
        return;
    }

    /* Zapisz do aktywnego bufora */
    uint8_t idx = s_write_idx;
    s_buf[idx].raw        = frame & AS_ANGLE;
    s_buf[idx].theta_mech = (float)(frame & AS_ANGLE) /
                             16384.0f * M_TWOPI;
    s_buf[idx].tick       = DWT->CYCCNT;
    s_buf[idx].valid      = true;

    /* Opublikuj atomowo - zamień indeks */
    __DMB();
    s_write_idx  = idx ^ 1u;   /* FOC zacznie czytać stary idx */
    s_new_sample = true;
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
