/*
 * speed_estimator.c
 *
 *  Created on: 4 Apr 2026
 *      Author: miloush
 */

#include "FOC/speed_estimator.h"
#include "BSP/as5048a.h"
#include <math.h>

#define TWO_PI (2.0f * (float)M_PI)
#define RAD_TO_RPM (60.0f / (2.0f * (float)M_PI))

static KalmanState  kf_state;
static KalmanParams kf_params;

static AngleUnwrap_t enc_unwrap;

static volatile SpeedEstimate_t s_out;

void SpeedEstimator_Init(float dt_sec)
{
    enc_unwrap.initialized = false;  // <-- reset bez zakładania kąta 0
    enc_unwrap.prev_wrapped = 0.0f;
    enc_unwrap.unwrapped    = 0.0f;
	AngleUnwrap_Reset(&enc_unwrap, 0.0f);

    Kalman_Init(&kf_state, &kf_params, dt_sec);

    s_out.theta = 0.0f;
    s_out.omega = 0.0f;
    s_out.omega_rpm = 0.0f;
    s_out.seq = 0;
}

void SpeedEstimator_Update(float theta_wrapped_rad)
{
	float theta_unwrapped = AngleUnwrap_Update(&enc_unwrap, theta_wrapped_rad);
    Kalman_Update(&kf_state, &kf_params, theta_unwrapped);

    /* Publikacja atomowa „na dwa razy” (seq nieparzyste = w trakcie zapisu) */
    s_out.seq++;
    __DMB();
    s_out.theta = kf_state.x[0];
    s_out.omega = kf_state.x[1];
    s_out.omega_rpm = s_out.omega * RAD_TO_RPM;
    __DMB();
    s_out.seq++;
}

SpeedEstimate_t SpeedEstimator_Get(void)
{
    SpeedEstimate_t tmp;
    uint32_t s1, s2;

    /* Bez wyłączania IRQ: czytaj aż trafisz na spójny snapshot */
    do {
        s1 = s_out.seq;
        tmp = (SpeedEstimate_t)s_out;
        s2 = s_out.seq;
    } while ((s1 != s2) || (s1 & 1u));

    return tmp;
}

float SpeedEstimator_GetOmegaRPM(void)
{
    return SpeedEstimator_Get().omega_rpm;
}

void Kalman_Init(KalmanState *s, KalmanParams *p, float dt){

	p->dt = dt;

	// A
	p->A[0][0] = 1.0; p->A[0][1] = dt;
	p->A[1][0] = 0.0; p->A[1][1] = 1.0;

	// H
	p->H[0][0] = 1.0;
	p->H[0][1] = 0.0;

	// Q = diag(q_theta, q_omega)
	p->Q[0][0] = 1e-4; p->Q[0][1] = 0.0;
	p->Q[1][0] = 0.0;  p->Q[1][1] = 1e-2;

	// Encoder noise
	p->R = 5e-5;

	// Initial state
	s->x[0] = 0.0; // theta
	s->x[1] = 0.0; // omega

	// P
	s->P[0][0] = 1.0; s->P[0][1] = 0.0;
	s->P[1][0] = 0.0; s->P[1][1] = 1.0;
}

void Kalman_Update(KalmanState *s, KalmanParams *p, float z){

    float x_pred[2];
    float P_pred[2][2];

	// PREDICITON

    // x_pred = A * x
    x_pred[0] = p->A[0][0]*s->x[0] + p->A[0][1]*s->x[1];
    x_pred[1] = p->A[1][0]*s->x[0] + p->A[1][1]*s->x[1];

    // P_pred = A * P * A^T + Q

    // Temp = A * P
    float AP00 = p->A[0][0]*s->P[0][0] + p->A[0][1]*s->P[1][0];
    float AP01 = p->A[0][0]*s->P[0][1] + p->A[0][1]*s->P[1][1];
    float AP10 = p->A[1][0]*s->P[0][0] + p->A[1][1]*s->P[1][0];
    float AP11 = p->A[1][0]*s->P[0][1] + p->A[1][1]*s->P[1][1];

    // P_pred = Temp * A^T + Q
    P_pred[0][0] = AP00*p->A[0][0] + AP01*p->A[0][1] + p->Q[0][0];
    P_pred[0][1] = AP00*p->A[1][0] + AP01*p->A[1][1] + p->Q[0][1];
    P_pred[1][0] = AP10*p->A[0][0] + AP11*p->A[0][1] + p->Q[1][0];
    P_pred[1][1] = AP10*p->A[1][0] + AP11*p->A[1][1] + p->Q[1][1];

	// CORECTION

    // y = z - H * x_pred
    float z_pred = p->H[0][0]*x_pred[0] + p->H[0][1]*x_pred[1];
    float y = z - z_pred;

    // S = H * P_pred * H^T + R  (scalar)
    float S = p->H[0][0]*(P_pred[0][0]*p->H[0][0] + P_pred[0][1]*p->H[0][1])
             + p->H[0][1]*(P_pred[1][0]*p->H[0][0] + P_pred[1][1]*p->H[0][1])
             + p->R;

    // K = P_pred * H^T * S^{-1}  -> vector 2x1
    float K0 = (P_pred[0][0]*p->H[0][0] + P_pred[0][1]*p->H[0][1]) / S;
    float K1 = (P_pred[1][0]*p->H[0][0] + P_pred[1][1]*p->H[0][1]) / S;

    // State update: x = x_pred + K * y
    s->x[0] = x_pred[0] + K0 * y;
    s->x[1] = x_pred[1] + K1 * y;

    // P Update: P = (I - K*H) * P_pred
    float I_KH00 = 1.0 - K0*p->H[0][0];
    float I_KH01 =     - K0*p->H[0][1];
    float I_KH10 =     - K1*p->H[0][0];
    float I_KH11 = 1.0 - K1*p->H[0][1];

    float P00 = I_KH00*P_pred[0][0] + I_KH01*P_pred[1][0];
    float P01 = I_KH00*P_pred[0][1] + I_KH01*P_pred[1][1];
    float P10 = I_KH10*P_pred[0][0] + I_KH11*P_pred[1][0];
    float P11 = I_KH10*P_pred[0][1] + I_KH11*P_pred[1][1];

    s->P[0][0] = P00; s->P[0][1] = P01;
    s->P[1][0] = P10; s->P[1][1] = P11;
}

void AngleUnwrap_Reset(AngleUnwrap_t *u, float theta_wrapped_rad)
{
    u->initialized  = true;
    u->prev_wrapped = theta_wrapped_rad;
    u->unwrapped    = theta_wrapped_rad;
}

float AngleUnwrap_Update(AngleUnwrap_t *u, float theta_wrapped_rad)
{
    if (!u->initialized) {
        AngleUnwrap_Reset(u, theta_wrapped_rad);
        return u->unwrapped;
    }

    float d = theta_wrapped_rad - u->prev_wrapped;

    // korekcja skoku na granicy 0..2pi
    if (d > (float)M_PI)       d -= TWO_PI;
    else if (d < -(float)M_PI) d += TWO_PI;

    u->unwrapped    += d;
    u->prev_wrapped  = theta_wrapped_rad;
    return u->unwrapped;
}
