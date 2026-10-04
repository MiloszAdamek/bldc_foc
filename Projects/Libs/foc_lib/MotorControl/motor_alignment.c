/*
 * motor_alignment.c
 *
 *  Created on: Sep 9, 2025
 *      Author: Milosz Adamek
 */

#include "motor_alignment.h"
#include "board.h"
#include "as5048a.h"
#include "encoder_incremental.h"
#include "encoder_types.h"
#include "lut_sincos.h"
#include "foc_utils.h"
#include "config.h"
#include <stdio.h>
#include <math.h>

extern BoardHandleTypeDef board;

float MotorAlignment_GetAngle();

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
    // Dociągnięcie silnika do kąta początkowego
    MotorAlignment_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, _3PI_2);
    HAL_Delay(700);

    const float steps = 600.0f;

    const float sweep_angle = 3.0f * M_PI; // 3 okresy elektryczne, 3 * (360 / 14) = 77 stopni mechanicznych

    printf("Krok 1: Wykrywanie kierunku...\n");

    // Ruch w przód
    for (int i = 0; i <= steps; i++) {
        float theta = _3PI_2 + (i * (sweep_angle / steps));
        MotorAlignment_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, theta);
        HAL_Delay(3);
    }
    HAL_Delay(200);

    float mid_angle = MotorAlignment_GetAngle();

    if (mid_angle < 0.0f) {
        printf("Błąd: odczyt kąta (mid)\n");
        return (g_motor_calib.aligned = false);
    }

    // Ruch w tył
    for (int i = steps; i >= 0; i--) {
        float theta = _3PI_2 + (i * (sweep_angle / steps));
        MotorAlignment_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, theta);
        HAL_Delay(2);
    }
    HAL_Delay(200);

    float end_angle = MotorAlignment_GetAngle();

    if (end_angle < 0.0f) {
        printf("Błąd: odczyt kąta (end)\n");
        return (g_motor_calib.aligned = false);
    }

    float moved = mid_angle - end_angle;
    if (moved < -M_PI) moved += M_TWOPI;
    if (moved >  M_PI) moved -= M_TWOPI;

    if (fabsf(moved) < 0.2f) {
        printf("Błąd: silnik nie poruszył się!\n");
        return (g_motor_calib.aligned = false);
    }

    g_motor_calib.direction = (moved > 0) ? SENSOR_DIRECTION_CCW : SENSOR_DIRECTION_CW;
    printf("Kierunek sensora: %d\n", g_motor_calib.direction);

    printf("Krok 2: Wyrównywanie do zera elektrycznego...\n");

    MotorAlignment_SetPhaseVoltage(0, VOLTAGE_SENSOR_ALIGN, _3PI_2);
    HAL_Delay(1000);

    if (board.encoder_type == ENCODER_INCREMENTAL) {
        EncoderIncremental_SetZero();
        g_motor_calib.zero_electric_angle = 0.0f;
    }
    else
    {
        float mech_angle = MotorAlignment_GetAngle();

        if (mech_angle < 0.0f) {
            printf("Błąd odczytu kąta przy wyznaczaniu zera.\n");
            return (g_motor_calib.aligned = false);
        }
        float el_angle = normalize_angle((float)g_motor_calib.direction * MOTOR_POLE_PAIRS * mech_angle);
        g_motor_calib.zero_electric_angle = normalize_angle(el_angle - _3PI_2);
    }


    printf("Offset elektryczny: %.4f rad\n", g_motor_calib.zero_electric_angle);

    MotorAlignment_SetPhaseVoltage(0, 0, 0);

    printf("--- Kalibracja zakończona pomyślnie! ---\n\n");

    printf("=== DEBUG ===\n");
    printf("mid_angle: %.4f rad\n", mid_angle);
    printf("end_angle: %.4f rad\n", end_angle);
    printf("moved: %.4f rad\n", moved);
    // printf("mech_angle (final): %.4f rad\n", mech_angle);
    // printf("el_angle: %.4f rad\n", el_angle);
    printf("zero_electric_angle: %.4f rad\n", g_motor_calib.zero_electric_angle);
    printf("=============\n");

    return (g_motor_calib.aligned = true);
}

float MotorAlignment_GetAngle()
{
    float theta = -1.0f;
    if(board.encoder_type == ENCODER_INCREMENTAL) {
        theta = EncoderIncremental_GetMechanicalAngle();
    }
    else if (board.encoder_type == ENCODER_AS5048A_ABSOLUTE) {
        theta = AS5048_GetAngleRad();
    }
    return theta;
}

bool MotorAlignment_IsAligned(void)
{
    return g_motor_calib.aligned;
}

float MotorAlignment_GetElectricalAngle(float mech)
{
    return normalize_angle((float)(g_motor_calib.direction * MOTOR_POLE_PAIRS) * mech - g_motor_calib.zero_electric_angle);
}


void Test_OpenLoop_Spin(void)
{
    printf("\n--- Start Testu Pętli Otwartej (Pure Sine) ---\n");
    Board_StartMotor(&board);

    // Tarot 5010 ma 14 par biegunów: 2 obroty mechaniczne = 2 * 14 * 2*PI rad elektrycznych
    const float total_angle = 2.0f * 14.0f * 2.0f * (float)M_PI;
    const int steps = 2000;
    const float U_amp = 3.0f; // Amplituda 3V (jeśli nie ruszy, podbij ostrożnie do 4.0-5.0V)

    for (int i = 0; i < steps; i++) {
        float theta = (total_angle * (float)i) / (float)steps;

        // Czysta modulacja 3 faz przesunięta o 120 stopni (2*PI/3)
        float ua = U_amp * cosf(theta);
        float ub = U_amp * cosf(theta - 2.0943951f); // 2*PI/3
        float uc = U_amp * cosf(theta + 2.0943951f);

        // Wyznaczenie współczynników wypełnienia (0.0 do 1.0)
        float dc_a = (ua / VOLTAGE_SUPPLY) + 0.5f;
        float dc_b = (ub / VOLTAGE_SUPPLY) + 0.5f;
        float dc_c = (uc / VOLTAGE_SUPPLY) + 0.5f;

        // Ograniczenie nasycenia
        if (dc_a < 0.05f) dc_a = 0.05f; else if (dc_a > 0.95f) dc_a = 0.95f;
        if (dc_b < 0.05f) dc_b = 0.05f; else if (dc_b > 0.95f) dc_b = 0.95f;
        if (dc_c < 0.05f) dc_c = 0.05f; else if (dc_c > 0.95f) dc_c = 0.95f;

        __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_1, (uint32_t)(dc_a * PWM_PERIOD_ARR));
        __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_2, (uint32_t)(dc_b * PWM_PERIOD_ARR));
        __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_3, (uint32_t)(dc_c * PWM_PERIOD_ARR));

        HAL_Delay(3); // 2000 kroków * 3 ms = 6 sekund na 2 pełne obroty
    }

    // Bezpieczne zdjęcie napięcia (wypełnienie 50% = zerowe napięcie międzyfazowe)
    __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_1, (uint32_t)(0.5f * PWM_PERIOD_ARR));
    __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_2, (uint32_t)(0.5f * PWM_PERIOD_ARR));
    __HAL_TIM_SET_COMPARE(board.htim_pwm, TIM_CHANNEL_3, (uint32_t)(0.5f * PWM_PERIOD_ARR));
    
    printf("--- Koniec Testu ---\n");
}

void Motor_Motion_Test(void)
{
    static uint32_t t0 = 0;
    static bool initialized = false;

    if (!initialized) {
        t0 = HAL_GetTick();
        initialized = true;
        Board_StartMotor(&board);
        HAL_TIM_Base_Start(board.htim_pwm);
    }

    // Bezpieczne napięcie testowe: zacznij od 2.5V - 3.5V zamiast procentu zasilania
    float Uq = 3.5f; 
    float Ud = 0.0f;

    uint32_t t_ms = HAL_GetTick() - t0;
    float t = (float)t_ms * 0.001f; // sekundy

    // Bardzo wolny obrót pola elektrycznego: 1.5 Hz
    // 1.5 Hz / 14 par biegunów = ok. 0.1 obr/s (pełny obrót wału w ~9 sekund)
    float freq = 10.0f; 
    float angle = 2.0f * (float)M_PI * freq * t;

    // Bezpieczna normalizacja kąta
    angle = fmodf(angle, 2.0f * (float)M_PI);
    if (angle < 0.0f) {
        angle += 2.0f * (float)M_PI;
    }

    MotorAlignment_SetPhaseVoltage(Uq, Ud, angle);
}