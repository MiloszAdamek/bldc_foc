/*
 * encoder.c
 *
 *  Created on: Oct 10, 2025
 *      Author: Milosz Adamek
 */

 #include "encoder.h"
 #include "encoder_hub.h"
 #include "encoder_types.h"
 #include "as5048a.h"
 #include "encoder_incremental.h"
 #include "board.h"
 #include "speed_estimator.h"
 #include "foc_utils.h"

extern BoardHandleTypeDef board;

float s_last_theta_mech = 0.0f;
uint32_t s_last_encoder_tick = 0;
bool s_encoder_valid = false;

#define DT_S (1.0f / (float)HAL_RCC_GetHCLKFreq())

void Encoder_Init(void)
{
    EncoderHub_Init(); // Na razie tak, ponieważ przekazuje dane do estymatora prędkości, bez względu na rodzaj enkodera
    // Do poprawy, bo enkoder inkrementalny nie potrzebuje DMA i bufora
    if (board.encoder_type == ENCODER_AS5048A_ABSOLUTE)
    {
        
        AS5048_Init(board.hspi_enc);
    }
    else if (board.encoder_type == ENCODER_INCREMENTAL)
    {
        EncoderIncremental_Init(board.htim_enc);
    }

    s_last_theta_mech = 0.0f;
    s_last_encoder_tick = 0;
    s_encoder_valid = false;
}

bool Encoder_GetAngle(EncoderAngle_t *angle, float omega_rad_s)
{
    if (angle == NULL)
        return false;

    angle->theta_raw = 0.0f;
    angle->theta_predicted = 0.0f;
    angle->dt = 0.0f;
    angle->new_sample = false;
    angle->valid = false;

    // Absolute encoder - AS5048A (SPI)
    if (board.encoder_type == ENCODER_AS5048A_ABSOLUTE)
    {
        EncoderSample_t sample;

        // New DMA sample
        if (EncoderHub_ConsumeSample(&sample))
        {
            s_last_theta_mech = sample.theta_mech;
            s_last_encoder_tick = sample.tick;

            s_encoder_valid = true;

            angle->new_sample = true;
        }

        // Predicted angle is only valid if we have a valid encoder sample
        if (!s_encoder_valid)
            return false;

        // Current CPU timestamp.
        uint32_t now = DWT->CYCCNT;

        // Time from the last encoder sample to now in seconds
        uint32_t delta_cycles = now - s_last_encoder_tick;
        angle->dt = (float)delta_cycles * DT_S;
        
        // Raw angle
        angle->theta_raw = s_last_theta_mech;

        // Predicted angle based on the last sample and the estimated speed
        angle->theta_predicted = normalize_angle(s_last_theta_mech + omega_rad_s * angle->dt);

        // Test
        // angle->theta_predicted = normalize_angle(s_last_theta_mech);

        angle->valid = true;

        return true;
    }

    // Incremental encoder
    if (board.encoder_type == ENCODER_INCREMENTAL)
    {
        float theta = EncoderIncremental_GetMechanicalAngle();

        angle->theta_raw = theta;
        angle->theta_predicted = theta;
        angle->dt = 0.0f; // No prediction for incremental encoder
        angle->new_sample = true;
        angle->valid = true;

        return true;
    }

    return false;
}

void Encoder_PublishAngle(float theta_mech, float theta_el)
{
    EncoderHub_PublishAngle(theta_mech, theta_el);
}