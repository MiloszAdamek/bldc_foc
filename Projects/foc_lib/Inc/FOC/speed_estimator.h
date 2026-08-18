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

#define SPEED_ESTIMATOR_KALMAN
//#define SPEED_ESTIMATOR_LPF

#define VELOCITY_ALPHA      0.95f
#define MAX_DTHETA_RAD      0.2f

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

#ifdef SPEED_ESTIMATOR_KALMAN

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

void Kalman_Init(KalmanState *s, KalmanParams *p, float dt);
void Kalman_Update(KalmanState *s, KalmanParams *p, float z);
void Kalman_Predict(KalmanState *s, KalmanParams *p);

#endif /* SPEED_ESTIMATOR_KALMAN */

void SpeedEstimator_Init(float dt_sec);
void SpeedEstimator_Update();
SpeedEstimate_t SpeedEstimator_Get(void);
float SpeedEstimator_GetOmegaRPM(void);
float SpeedEstimator_GetOmegaRPM_ISR(void);

void  AngleUnwrap_Reset(AngleUnwrap_t *u, float theta_wrapped_rad);
float AngleUnwrap_Update(AngleUnwrap_t *u, float theta_wrapped_rad);

#endif /* INC_BSP_SPEED_ESTIMATOR_H_ */
