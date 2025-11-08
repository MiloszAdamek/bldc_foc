/*
 * tests.h
 *
 *  Created on: Nov 8, 2025
 *      Author: Miloush
 */

#ifndef INC_TESTS_H_
#define INC_TESTS_H_

#include "as5048a.h"
#include "main.h"
#include "float.h"

// SPI + encoder tests

#ifdef TEST_MODE

void TestMode_Init(SPI_HandleTypeDef *hspi){
	AS5048_Init(hspi);
}

void Test_AS5048AStability(void)
{
    const uint32_t samples = 1000;
    double min = DBL_MAX, max = -DBL_MAX;
    double sum = 0.0, sum_sq = 0.0;
    uint32_t err = 0, valid = 0;

    HAL_Delay(50);    // chwilka na ustalenie AGC i ustabilizowanie czujnika

    for (uint32_t i = 0; i < samples; i++)
    {
        // odczyt blokujący kąta
        AS5048_GetRawPosition();

        if (raw_angle.status != AS5048_OK) {
            err++;
            continue;
        }

        double angle = ((double)raw_angle.position / AS5048_RESOLUTION) * M_TWOPI;

        if (angle < min) min = angle;
        if (angle > max) max = angle;
        sum     += angle;
        sum_sq  += angle * angle;
        valid++;

        // odstęp między pomiarami – 1000 µs = 1 ms
        delay_us(1000);
    }

    double avg = (valid > 0) ? (sum / valid) : 0.0;
    double jitter = max - min;
    double var = (valid > 0) ? (sum_sq / valid - avg * avg) : 0.0;
    if (var < 0.0) var = 0.0;
    double stddev = sqrt(var);

    printf("\n=== TEST STABILNOSCI AS5048A (blokujacy) ===\n");
    printf("Probek: %lu, Bledy: %lu\n", (unsigned long)valid, (unsigned long)err);
    printf("Sredni kat: %.4f rad (%.2f°)\n", avg, avg * 180.0 / M_PI);
    printf("Jitter: %.6f rad (%.4f°)\n", jitter, jitter * 180.0 / M_PI);
    printf("Odch.std.: %.6f rad (%.4f°)\n", stddev, stddev * 180.0 / M_PI);
    printf("===========================================\n");
}

void Test_AS5048AStabilityDMA(void)
{
    const uint32_t samples = 1000;
    double min = DBL_MAX, max = -DBL_MAX;
    double sum = 0.0, sum_sq = 0.0;
    uint32_t err = 0, valid = 0;

    HAL_Delay(50);   // czas dla ustalenia AGC i stabilizacji

    for (uint32_t i = 0; i < samples; i++)
    {
        // uruchom DMA jeśli gotowe
        if (spi_ready)
            AS5048_ReadAngleDMA();

        // czekaj na zakończenie transmisji DMA (blokująco)
        while (!spi_ready);

        if (raw_angle.status != AS5048_OK) {
            err++;
            continue;
        }

        double angle = ((double)raw_angle.position / AS5048_RESOLUTION) * M_TWOPI;

        if (angle < min) min = angle;
        if (angle > max) max = angle;

        sum     += angle;
        sum_sq  += angle * angle;
        valid++;

        delay_us(1);
    }

    double avg = (valid > 0) ? (sum / valid) : 0.0;
    double jitter = max - min;
    double var = (valid > 0) ? (sum_sq / valid - avg * avg) : 0.0;
    if (var < 0.0) var = 0.0;
    double stddev = sqrt(var);

    printf("\n=== TEST STABILNOSCI AS5048A (DMA) ===\n");
    printf("Probek: %lu, Bledy: %lu\n", (unsigned long)valid, (unsigned long)err);
    printf("Sredni kat: %.4f rad (%.2f°)\n", avg, avg * 180.0 / M_PI);
    printf("Jitter: %.6f rad (%.4f°)\n", jitter, jitter * 180.0 / M_PI);
    printf("Odch.std.: %.6f rad (%.4f°)\n", stddev, stddev * 180.0 / M_PI);
    printf("===========================================\n");
}

#endif

#endif /* INC_TESTS_H_ */
