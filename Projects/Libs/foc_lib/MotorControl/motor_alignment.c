/*
 * motor_alignment.c
 *
 *  Created on: Sep 9, 2025
 *      Author: Milosz Adamek
 */

#include "motor_alignment.h"
#include "board.h"
#include "as5048a.h"
#include "lut_sincos.h"
#include "foc_utils.h"
#include "config.h"
#include <stdio.h>
#include <math.h>

extern BoardHandleTypeDef board;

Motor_Calibration_t g_motor_calib = {
    .direction = 1,
    .zero_electric_angle = 0.0f,
    .aligned = false
};

static void MotorAlignment_SetPhaseVoltage(float Uq, float Ud, float angle_el)
{
    float Uref = sqrtf(Ud * Ud + Uq * Uq);
    float Umax = VOLTAGE_SUPPLY / M_SQRT3;

    if (Uref > Umax) {
        float scale = Umax / Uref;
        Ud *= scale;
        Uq *= scale;
    }

    float sin_t, cos_t;
    LUT_SinCos(angle_el, &sin_t, &cos_t); 

    float Ualpha, Ubeta;
    InvParkTransform(Ud, Uq, &sin_t, &cos_t, &Ualpha, &Ubeta);

    float Ua, Ub, Uc;
    InvClarkeTransform(Ualpha, Ubeta, &Ua, &Ub, &Uc);

    float dc_a = (Ua / VOLTAGE_SUPPLY) + 0.5f;
    float dc_b = (Ub / VOLTAGE_SUPPLY) + 0.5f;
    float dc_c = (Uc / VOLTAGE_SUPPLY) + 0.5f;

    uint32_t pwm_a = (uint32_t)(dc_a * PWM_PERIOD_ARR);
    uint32_t pwm_b = (uint32_t)(dc_b * PWM_PERIOD_ARR);
    uint32_t pwm_c = (uint32_t)(dc_c * PWM_PERIOD_ARR);

    if (pwm_a > PWM_PERIOD_ARR) pwm_a = PWM_PERIOD_ARR;
    if (pwm_b > PWM_PERIOD_ARR) pwm_b = PWM_PERIOD_ARR;
    if (pwm_c > PWM_PERIOD_ARR) pwm_c = PWM_PERIOD_ARR;

    __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_1, pwm_a);
    __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_2, pwm_b);
    __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_3, pwm_c);
}

bool MotorAlignment_AlignSensor(void)
{
    if (g_motor_calib.aligned) {
        return true;
    }

    Board_StartMotor(&board);

    printf("\n--- Start kalibracji sensora ---\n");
    printf("Krok 1: Wykrywanie kierunku...\n");

    for (int i = 0; i <= 500; i++) {
        float theta = _3PI_2 + (i * (M_TWOPI / 500.0f));
        MotorAlignment_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, theta);
        HAL_Delay(2);
    }

    float mid_angle = AS5048_GetAngleRad();
    if (mid_angle < 0.0f) {
        printf("Błąd: odczyt kąta (mid)\n");
        return (g_motor_calib.aligned = false);
    }

    for (int i = 500; i >= 0; i--) {
        float theta = _3PI_2 + (i * (M_TWOPI / 500.0f));
        MotorAlignment_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, theta);
        HAL_Delay(2);
    }

    float end_angle = AS5048_GetAngleRad();
    if (end_angle < 0.0f) {
        printf("Błąd: odczyt kąta (end)\n");
        return (g_motor_calib.aligned = false);
    }

    float moved = mid_angle - end_angle;
    if (moved < -M_PI) moved += M_TWOPI;
    if (moved >  M_PI) moved -= M_TWOPI;

    if (fabsf(moved) < 0.1f) {
        printf("Błąd: silnik nie poruszył się!\n");
        return (g_motor_calib.aligned = false);
    }

    g_motor_calib.direction = (moved > 0) ? SENSOR_DIRECTION_CCW : SENSOR_DIRECTION_CW;
    printf("Kierunek sensora: %d\n", g_motor_calib.direction);

    printf("Krok 2: Wyrównywanie do zera elektrycznego...\n");

    MotorAlignment_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, _3PI_2);
    HAL_Delay(700);

    float mech_angle = AS5048_GetAngleRad();
    if (mech_angle < 0.0f) {
        printf("Błąd odczytu kąta przy wyznaczaniu zera.\n");
        return (g_motor_calib.aligned = false);
    }

    float el_angle = normalize_angle((float)g_motor_calib.direction * MOTOR_POLE_PAIRS * mech_angle);
    g_motor_calib.zero_electric_angle = normalize_angle(el_angle - _3PI_2);

    printf("Offset elektryczny: %.4f rad\n", g_motor_calib.zero_electric_angle);

    MotorAlignment_SetPhaseVoltage(0, 0, 0);

    printf("--- Kalibracja zakończona pomyślnie! ---\n\n");

    printf("=== DEBUG ===\n");
    printf("mid_angle: %.4f rad\n", mid_angle);
    printf("end_angle: %.4f rad\n", end_angle);
    printf("moved: %.4f rad\n", moved);
    printf("mech_angle (final): %.4f rad\n", mech_angle);
    printf("el_angle: %.4f rad\n", el_angle);
    printf("zero_electric_angle: %.4f rad\n", g_motor_calib.zero_electric_angle);
    printf("=============\n");

    return (g_motor_calib.aligned = true);
}

bool MotorAlignment_IsAligned(void)
{
    return g_motor_calib.aligned;
}

float MotorAlignment_GetElectricalAngle(float mech)
{
    return normalize_angle((float)(g_motor_calib.direction * MOTOR_POLE_PAIRS) * mech - g_motor_calib.zero_electric_angle);
}

// void Motor_Motion_Test(void)
// {
//     static uint32_t t0 = 0;

//     float Uq = 0.6f * (VOLTAGE_SUPPLY / M_SQRT3);
//     float Ud = 0.0f;

//     uint32_t t_ms = HAL_GetTick() - t0;   // czas od startu w ms
//     float t = t_ms * 0.001f;              // sekundy

//     float freq = 20.0f;                   // 20 Hz elektryczne
//     float angle = 2.0f * M_PI * freq * t;

//     // zawijanie kąta (opcjonalne)
//     angle = fmodf(angle, 2.0f * M_PI);

//     FOC_SetPhaseVoltage(Uq, Ud, angle);
// }
