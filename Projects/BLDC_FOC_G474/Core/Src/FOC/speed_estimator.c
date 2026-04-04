/*
 * speed_estimator.c
 *
 *  Created on: 4 Apr 2026
 *      Author: miloush
 */

#include "FOC/speed_estimator.h"
#include "BSP/as5048a.h"
#include "BSP/encoder_hub.h"
#include <math.h>

#define TWO_PI (2.0f * (float)M_PI)
#define RAD_TO_RPM (60.0f / (2.0f * (float)M_PI))

static AngleUnwrap_t enc_unwrap;
static volatile SpeedEstimate_t s_out;

#ifdef SPEED_ESTIMATOR_KALMAN

static KalmanState  kf_state;
static KalmanParams kf_params;

#else /* SPEED_ESTIMATOR_LPF */

static float s_dt_sec   = 0.001f;
static float s_omega_lpf = 0.0f;

#endif

static void Publish(float theta, float omega)
{
    s_out.seq++;
    __DMB();
    s_out.theta     = theta;
    s_out.omega     = omega;
    s_out.omega_rpm = omega * RAD_TO_RPM;
    __DMB();
    s_out.seq++;
}

void SpeedEstimator_Init(float dt_sec)
{
    enc_unwrap.initialized = false;  // <-- reset bez zakładania kąta 0
    enc_unwrap.prev_wrapped = 0.0f;
    enc_unwrap.unwrapped    = 0.0f;

    s_out.theta = 0.0f;
    s_out.omega = 0.0f;
    s_out.omega_rpm = 0.0f;
    s_out.seq = 0;

#ifdef SPEED_ESTIMATOR_KALMAN
    Kalman_Init(&kf_state, &kf_params, dt_sec);

#else /* SPEED_ESTIMATOR_LPF */
    s_dt_sec    = dt_sec;
    s_omega_lpf = 0.0f;
#endif
}

void SpeedEstimator_Update(void)
{
    AngleSnapshot_t snap = EncoderHub_GetAngleSnapshot();

    static uint32_t last_tick = 0;
    bool new_sample = snap.tick != last_tick;

#ifdef SPEED_ESTIMATOR_KALMAN
    /* ---- Kalman ---- */
    if (!new_sample) {
        Kalman_Predict(&kf_state, &kf_params);
    } else {
        last_tick = snap.tick;
        float theta_unwrapped = AngleUnwrap_Update(&enc_unwrap, snap.theta_mech);
        Kalman_Update(&kf_state, &kf_params, theta_unwrapped);
    }
    Publish(kf_state.x[0], kf_state.x[1]);

#else /* SPEED_ESTIMATOR_LPF */
    /* ---- Różniczkowanie + LPF ---- */
    if (!new_sample) {
        /* Brak nowej próbki - nie aktualizuj, publikuj ostatnią wartość */
        Publish(enc_unwrap.unwrapped, s_omega_lpf);
        return;
    }

    last_tick = snap.tick;

    /* Ochrona przed glitchem enkodera */
    float dtheta = snap.theta_mech - enc_unwrap.prev_wrapped;

    /* Zawijanie różnicy do (-pi, pi) */
    if      (dtheta >  (float)M_PI) dtheta -= TWO_PI;
    else if (dtheta < -(float)M_PI) dtheta += TWO_PI;

    if (fabsf(dtheta) > MAX_DTHETA_RAD) {
        /* Glitch - zaktualizuj unwrap ale nie prędkość */
        AngleUnwrap_Update(&enc_unwrap, snap.theta_mech);
        Publish(enc_unwrap.unwrapped, s_omega_lpf);
        return;
    }

    /* Aktualizacja unwrap */
    float theta_unwrapped = AngleUnwrap_Update(&enc_unwrap, snap.theta_mech);

    /* Prędkość kątowa surowa */
    float omega_raw = dtheta / s_dt_sec;

    /* LPF */
    s_omega_lpf = VELOCITY_ALPHA * s_omega_lpf + (1.0f - VELOCITY_ALPHA) * omega_raw;

    Publish(theta_unwrapped, s_omega_lpf);

#endif /* SPEED_ESTIMATOR_KALMAN / LPF */
}

SpeedEstimate_t SpeedEstimator_Get(void)
{
    SpeedEstimate_t tmp;
    uint32_t s1, s2;
    do {
        s1  = s_out.seq;
        __DMB();
        tmp = (SpeedEstimate_t)s_out;
        __DMB();
        s2  = s_out.seq;
    } while ((s1 != s2) || (s1 & 1u));
    return tmp;
}

float SpeedEstimator_GetOmegaRPM(void)
{
    return SpeedEstimator_Get().omega_rpm;
}

#ifdef SPEED_ESTIMATOR_KALMAN

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
	p->Q[1][0] = 0.0;  p->Q[1][1] = 1e-1;

	// Encoder noise
	p->R = 5e-5;

	// Initial state
	s->x[0] = 0.0; // theta
	s->x[1] = 0.0; // omega

	// P
	s->P[0][0] = 1.0; s->P[0][1] = 0.0;
	s->P[1][0] = 0.0; s->P[1][1] = 1.0;
}

void Kalman_Update(KalmanState *s, KalmanParams *p, float z)
{
    /* --- Predykcja stanu x_pred = A*x --- */
    float x0_pred = p->A[0][0]*s->x[0] + p->A[0][1]*s->x[1];
    float x1_pred = p->A[1][0]*s->x[0] + p->A[1][1]*s->x[1];

    /* --- Predykcja kowariancji P_pred = A*P*A^T + Q --- */
    float AP00 = p->A[0][0]*s->P[0][0] + p->A[0][1]*s->P[1][0];
    float AP01 = p->A[0][0]*s->P[0][1] + p->A[0][1]*s->P[1][1];
    float AP10 = p->A[1][0]*s->P[0][0] + p->A[1][1]*s->P[1][0];
    float AP11 = p->A[1][0]*s->P[0][1] + p->A[1][1]*s->P[1][1];

    float Pp00 = AP00*p->A[0][0] + AP01*p->A[0][1] + p->Q[0][0];
    float Pp01 = AP00*p->A[1][0] + AP01*p->A[1][1] + p->Q[0][1];
    float Pp10 = AP10*p->A[0][0] + AP11*p->A[0][1] + p->Q[1][0];
    float Pp11 = AP10*p->A[1][0] + AP11*p->A[1][1] + p->Q[1][1];

    /* --- Innowacja y = z - H*x_pred --- */
    float y = z - (p->H[0][0]*x0_pred + p->H[0][1]*x1_pred);

    /* --- S = H*P_pred*H^T + R (skalar dla H=[1,0]) --- */
    float S = p->H[0][0]*(Pp00*p->H[0][0] + Pp01*p->H[0][1])
            + p->H[0][1]*(Pp10*p->H[0][0] + Pp11*p->H[0][1])
            + p->R;

    /* --- Wzmocnienie Kalmana K = P_pred*H^T / S --- */
    float K0 = (Pp00*p->H[0][0] + Pp01*p->H[0][1]) / S;
    float K1 = (Pp10*p->H[0][0] + Pp11*p->H[0][1]) / S;

    /* --- Aktualizacja stanu x = x_pred + K*y --- */
    s->x[0] = x0_pred + K0 * y;
    s->x[1] = x1_pred + K1 * y;

    /* --- Aktualizacja kowariancji P = (I - K*H)*P_pred --- */
    float IKH00 = 1.0f - K0*p->H[0][0];
    float IKH01 =      - K0*p->H[0][1];
    float IKH10 =      - K1*p->H[0][0];
    float IKH11 = 1.0f - K1*p->H[0][1];

    s->P[0][0] = IKH00*Pp00 + IKH01*Pp10;
    s->P[0][1] = IKH00*Pp01 + IKH01*Pp11;
    s->P[1][0] = IKH10*Pp00 + IKH11*Pp10;
    s->P[1][1] = IKH10*Pp01 + IKH11*Pp11;
}

void Kalman_Predict(KalmanState *s, KalmanParams *p)
{
    /* --- Predykcja stanu x = A*x --- */
    float x0_pred = p->A[0][0]*s->x[0] + p->A[0][1]*s->x[1];
    float x1_pred = p->A[1][0]*s->x[0] + p->A[1][1]*s->x[1];
    s->x[0] = x0_pred;
    s->x[1] = x1_pred;

    /* --- Predykcja kowariancji P = A*P*A^T + Q --- */
    float AP00 = p->A[0][0]*s->P[0][0] + p->A[0][1]*s->P[1][0];
    float AP01 = p->A[0][0]*s->P[0][1] + p->A[0][1]*s->P[1][1];
    float AP10 = p->A[1][0]*s->P[0][0] + p->A[1][1]*s->P[1][0];
    float AP11 = p->A[1][0]*s->P[0][1] + p->A[1][1]*s->P[1][1];

    float Pp00 = AP00*p->A[0][0] + AP01*p->A[0][1] + p->Q[0][0];
    float Pp01 = AP00*p->A[1][0] + AP01*p->A[1][1] + p->Q[0][1];
    float Pp10 = AP10*p->A[0][0] + AP11*p->A[0][1] + p->Q[1][0];
    float Pp11 = AP10*p->A[1][0] + AP11*p->A[1][1] + p->Q[1][1];

    s->P[0][0] = Pp00;
    s->P[0][1] = Pp01;
    s->P[1][0] = Pp10;
    s->P[1][1] = Pp11;
}
#endif /* SPEED_ESTIMATOR_KALMAN */

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
