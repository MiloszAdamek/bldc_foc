/*
 * speed_estimator.h
 *
 *  Created on: 4 Apr 2026
 *      Author: miloush
 */
#include <stdint.h>
#include <stdbool.h>

#ifndef INC_BSP_SPEED_ESTIMATOR_H_
#define INC_BSP_SPEED_ESTIMATOR_H_

typedef struct {
	float x[2];     // [theta, omega]
    float P[2][2];  // Cov matrix, state
} KalmanState;

typedef struct {
    float A[2][2];
    float H[1][2];
    float Q[2][2]; // Cov matrix, process noise
    float R;       // Meassure noise (theta)
    float dt;
} KalmanParams;

typedef struct {
    float theta;       // rad
    float omega;       // rad/s
    float omega_rpm;   // rpm
    uint32_t seq;      // counter
} SpeedEstimate_t;

typedef struct {
    bool  initialized;
    float prev_wrapped;   // poprzedni pomiar 0..2pi
    float unwrapped;      // kąt ciągły
} AngleUnwrap_t;

void  AngleUnwrap_Reset(AngleUnwrap_t *u, float theta_wrapped_rad);

float AngleUnwrap_Update(AngleUnwrap_t *u, float theta_wrapped_rad);

void SpeedEstimator_Init(float dt_sec);

void SpeedEstimator_Update(float theta_meas_rad);

SpeedEstimate_t SpeedEstimator_Get(void);

float SpeedEstimator_GetOmegaRPM(void);

void Kalman_Init(KalmanState *s, KalmanParams *p, float dt);

void Kalman_Update(KalmanState *s, KalmanParams *p, float z);

#endif /* INC_BSP_SPEED_ESTIMATOR_H_ */
