/*
 * brake_chopper.c
 *
 *  Created on: Oct 11, 2026
 *      Author: Milosz Adamek
 */

 #include "brake_chopper.h"
 #include "board.h"
 #include "foc_utils.h"
 #include "config.h"

 extern BoardHandleTypeDef board;

static BrakeChopperState_t s_chopper;

bool BrakeChopper_IsLatched(void) { return s_chopper.latched; }
float BrakeChopper_GetDuty(void) { return s_chopper.duty; }
float BrakeChopper_GetPowerHeat(void) { return s_chopper.p_heat; }
float BrakeChopper_GetPowerInstant(void) { return s_chopper.p_inst; }

static void prv_apply_pwm(float duty)
{
    duty = clampf(duty, 0.0f, BRAKE_DUTY_MAX);
    s_chopper.duty = duty;

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(board.htim_brake_chopper);
    uint32_t ccr = (uint32_t)((float)arr * duty);
    __HAL_TIM_SET_COMPARE(board.htim_brake_chopper, TIM_CHANNEL_1, ccr);
}

void BrakeChopper_Init(void)
{
    s_chopper.running = false;
    s_chopper.duty = 0.0f;
    s_chopper.integ = 0.0f;
    s_chopper.p_inst = 0.0f;
    s_chopper.p_heat = 0.0f;
    s_chopper.overload_time = 0.0f;
    s_chopper.latched = false;

    __HAL_TIM_SET_COMPARE(board.htim_brake_chopper, TIM_CHANNEL_1, 0);
}

void BrakeChopper_Start(void)
{
    if (!s_chopper.running)
    {
        HAL_TIM_PWM_Start(board.htim_brake_chopper, TIM_CHANNEL_1);
        s_chopper.running = true;
        // nie resetuj p_heat – to ma pamiętać “nagrzanie”
        s_chopper.integ = 0.0f;
        prv_apply_pwm(0.0f);
    }
}

void BrakeChopper_Stop(void)
{
    HAL_TIM_PWM_Stop(board.htim_brake_chopper, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(board.htim_brake_chopper, TIM_CHANNEL_1, 0);

    s_chopper.running = false;
    s_chopper.duty = 0.0f;
    s_chopper.integ = 0.0f;
    s_chopper.p_inst = 0.0f;
    s_chopper.overload_time = 0.0f;
}

void BrakeChopper_ClearLatch(void)
{
    s_chopper.latched = false;
    s_chopper.overload_time = 0.0f;
    // zostaw p_heat – jeśli rezystor jest gorący, to i tak limiter mocy zadziała
}

void BrakeChopper_SetDutyCycle(float duty_cycle)
{
    if (!s_chopper.running) return;
    prv_apply_pwm(duty_cycle);
}

void BrakeChopper_Update(Motor_Measurements_t *meas, float dt)
{
    if (!meas || dt <= 0.0f) return;

    const float v = meas->v_bus;

    // Jeśli zadziałał latch: chopper OFF, wymagaj ręcznego skasowania
    if (s_chopper.latched)
    {
        BrakeChopper_Stop();
        return;
    }

    // 1) Histereza start/stop na VBUS
    if (!s_chopper.running)
    {
        if (v > BRAKE_V_ON) BrakeChopper_Start();
    }
    else
    {
        if (v < BRAKE_V_OFF) BrakeChopper_Stop();
    }

    // 2) Gdy nie pracuje – p_heat powoli opada (symulacja stygnięcia)
    if (!s_chopper.running)
    {
        const float alpha = dt / (BRAKE_HEAT_TAU_SEC + dt);
        s_chopper.p_heat += alpha * (0.0f - s_chopper.p_heat);
        s_chopper.p_inst = 0.0f;
        return;
    }

    // 3) PI na VBUS -> duty
    // err>0 => Vbus powyżej target -> zwiększ duty
    float err = v - BRAKE_V_TARGET;

    // Uwaga: integrator tylko w zakresie, żeby nie "pompował" przy ograniczeniach
    s_chopper.integ += (BRAKE_KI * err * dt);

    // Limit mocy ciągłej: duty <= P_CONT * R / V^2
    float duty_max_power = (BRAKE_P_CONT_W * BRAKE_R_OHM) / (v * v + 1e-6f);
    duty_max_power = clampf(duty_max_power, 0.0f, BRAKE_DUTY_MAX);

    // clamp integratora do sensownego max (prosty anti-windup)
    s_chopper.integ = clampf(s_chopper.integ, 0.0f, duty_max_power);

    float duty_cmd = BRAKE_KP * err + s_chopper.integ;
    duty_cmd = clampf(duty_cmd, 0.0f, duty_max_power);

    // P_avg ≈ duty * V^2 / R
    s_chopper.p_inst = duty_cmd * (v * v) / BRAKE_R_OHM;

    // 6) “Nagrzanie” jako filtrowana moc (IIR)
    const float alpha = dt / (BRAKE_HEAT_TAU_SEC + dt);
    s_chopper.p_heat += alpha * (s_chopper.p_inst - s_chopper.p_heat);

    // 7) Trip: jeśli “p_heat” > P_TRIP przez BRAKE_TRIP_TIME_SEC -> latch
    if (s_chopper.p_heat > BRAKE_P_TRIP_W)
        s_chopper.overload_time += dt;
    else
        s_chopper.overload_time = 0.0f;

    if (s_chopper.overload_time >= BRAKE_TRIP_TIME_SEC)
    {
        s_chopper.latched = true;
        BrakeChopper_Stop();

        // WAŻNE: tutaj powinieneś wyłączyć regenerację / zatrzymać napęd,
        // inaczej VBUS może uciec w overvoltage.
        // np. MotorControl_Stop(); + ustaw fault

        return;
    }

    // 8) Ustaw PWM
    prv_apply_pwm(duty_cmd);
}


