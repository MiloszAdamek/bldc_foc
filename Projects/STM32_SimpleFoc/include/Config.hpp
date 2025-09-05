#pragma once
#include <Arduino.h>

namespace AppConfig {

struct MotorConfig {
  // Piny PWM i EN
  uint8_t pwm_u, pwm_v, pwm_w;
  uint8_t en_u, en_v, en_w;

  // SPI3
  uint8_t spi3_mosi, spi3_miso, spi3_sck, spi3_cs;

  // Enkoder AS5048A
  uint8_t  bit_resolution;
  uint16_t angle_register;

  // ADC - Current sense (low-side)
  uint8_t curr_u, curr_v, curr_w;
  float   shunt_resistance;
  float   amp_gain;

  // Parametry silnika
  int   pole_pairs;
  float kv_rating;
  float phase_resistance;

  // Driver/Voltage
  float    v_supply;
  float    v_limit;
  uint32_t pwm_frequency;

  // Sterowanie

    // Velocity
    float pid_v_p, pid_v_i, pid_v_d;
    float pid_v_output_ramp;
    float lpf_velocity_Tf;
    float current_limit;

    // Torque, Id, Iq
    // I_Q axis
    float pid_iq_p, pid_iq_i, pid_iq_d;
    float pid_iq_output_ramp;
    float lpf_iq_Tf;

    // I_D axis
    float pid_id_p, pid_id_i, pid_id_d;
    float pid_id_output_ramp;
    float lpf_id_Tf;

  // Startowy target
  float initial_target_velocity;
};

// Domyślna konfiguracja płytki/połączeń
extern const MotorConfig BoardConfig;

} // namespace AppConfig