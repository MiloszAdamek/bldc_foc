/*
 * brake_chopper.c
 *
 *  Created on: Oct 11, 2026
 *      Author: Milosz Adamek
 */

 #ifdef ENABLE_BRAKE_CHOPPER

 #include "brake_chopper.h"
 #include "board.h"
 #include "foc_utils.h"
 #include "config.h"

extern BoardHandleTypeDef board;

static BrakeChopperState_t s_chopper;

static void prv_set_pwm(float duty)
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
    s_chopper.latched = false;
    s_chopper.duty    = 0.0f;
    s_chopper.integ   = 0.0f;
    s_chopper.power_w = 0.0f;
    s_chopper.temp_c  = 0.0f;

    HAL_TIM_Base_Start_IT(board.htim_brake_chopper);
    __HAL_TIM_SET_COMPARE(board.htim_brake_chopper, TIM_CHANNEL_1, 0);
    HAL_TIM_PWM_Start(board.htim_brake_chopper, TIM_CHANNEL_1);
}

void BrakeChopper_Reset(void)
{
    if (s_chopper.temp_c < (BRAKE_TEMP_MAX_TRIP - BRAKE_TEMP_HYST) && s_chopper.temp_c > -10.0f)
        {
            s_chopper.latched = false;
        }
}

void BrakeChopper_Update(Motor_Measurements_t *meas, float dt)
{
    if (!meas || dt <= 0.0f) return;

    s_chopper.temp_c = meas->motor_temp;
    const float v = meas->v_bus;

    // Overtemperature lub wypięty NTC (poniżej -10°C) - wyłącz i zatrzaśnij błąd
    if (s_chopper.temp_c >= BRAKE_TEMP_MAX_TRIP || s_chopper.temp_c < -10.0f)
    {
        s_chopper.latched = true;
    }

    if (s_chopper.latched)
    {
        s_chopper.running = false;
        s_chopper.integ   = 0.0f;
        s_chopper.power_w = 0.0f;
        prv_set_pwm(0.0f);
        return;
    }

    // Histereza załączania na podstawie VBUS
    if (!s_chopper.running)
    {
        if (v > BRAKE_V_ON)
        {
            s_chopper.running = true;
            s_chopper.integ   = 0.0f;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (v < BRAKE_V_OFF)
        {
            s_chopper.running = false;
            s_chopper.integ   = 0.0f;
            s_chopper.power_w = 0.0f;
            prv_set_pwm(0.0f);
            return;
        }
    }

    // Regulator PI napięcia szyny DC
    float err = v - BRAKE_V_TARGET;

    s_chopper.integ += (BRAKE_KI * err * dt);
    s_chopper.integ = clampf(s_chopper.integ, 0.0f, BRAKE_DUTY_MAX);

    float duty_cmd = (BRAKE_KP * err) + s_chopper.integ;
    prv_set_pwm(duty_cmd);

    s_chopper.power_w = s_chopper.duty * (v * v) / BRAKE_R_OHM;
}

#endif // ENABLE_BRAKE_CHOPPER


