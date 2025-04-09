/*
 * foc_loop.c
 *
 *  Created on: Apr 1, 2025
 *      Author: Miloush
 */

#include "foc_loop.h"
#include "transforms.h"
#include "svpwm.h"
#include "current_sense.h"

static PI_Controller pi_id = { .kp = 2.0f, .ki = 200.0f, .limit = 5.0f, .integral = 0.0f };
static PI_Controller pi_iq = { .kp = 2.0f, .ki = 200.0f, .limit = 5.0f, .integral = 0.0f };

static float pi_control(PI_Controller *pi, float error)
{
    pi->integral += error * pi->ki * 0.00005f; // Ts = 50 us

    if (pi->integral > pi->limit) pi->integral = pi->limit;
    else if (pi->integral < -pi->limit) pi->integral = -pi->limit;

    float output = pi->kp * error + pi->integral;

    if (output > pi->limit) output = pi->limit;
    else if (output < -pi->limit) output = -pi->limit;

    return output;
}

void FOC_Init(ADC_HandleTypeDef *hadc)
{
    pi_id.integral = 0.0f;
    pi_iq.integral = 0.0f;

    CurrentSense_Init(hadc);
}

void FOC_Update(const abc_current_t *currents, float theta_el, const dq_ref_t *i_ref)
{
    float ialpha, ibeta;
    float id, iq;
    float vd, vq;
    float valpha, vbeta;

    // Clarke
    ClarkeTransform(currents->a, currents->b, &ialpha, &ibeta);

    // Park
    ParkTransform(ialpha, ibeta, theta_el, &id, &iq);

    // PI kontrola
    vd = pi_control(&pi_id, i_ref->d - id);
    vq = pi_control(&pi_iq, i_ref->q - iq);

    // Inverse Park
    InvParkTransform(vd, vq, theta_el, &valpha, &vbeta);

    // SVPWM
    SVPWM_Update(valpha, vbeta);
}

// FOC Loop
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        // 1. Pomiar prądów
        CurrentSense_Meassurement(hadc);

        // 2. Pobranie zmierzonych wartości
        abc_current_t currents;
        CurrentSense_Read(&currents);

//        // 3. Kąt elektryczny (np. testowo 0)
//        float theta_el = 0.0f; // TODO: podłącz enkoder
//
//        // 4. Referencje
//        dq_ref_t current_ref = {
//            .d = 0.0f,
//            .q = 1.0f  // 1A dla testu
//        };
//
//        // 5. FOC
//        FOC_Update(&currents, theta_el, &current_ref);
    }
}


