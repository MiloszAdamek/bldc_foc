#include "Config.hpp"

namespace AppConfig {

// Uzupełnij zgodnie z Twoim hardware
const MotorConfig BoardConfig = {
  // PWM U, V, W
  PA8, PA9, PA10,
  // EN U, V, W
  PB13, PB14, PB15,

  // SPI3: MOSI, MISO, SCK, CS
  PC12, PC11, PC10, PD2,

  // Enkoder AS5048A
  14,         // bit_resolution
  0x3FFF,     // angle_register

  // Current sense pins + parametry
  PA1, PB1, PB0,
  0.33f,      // shunt_resistance
  1.528f,     // amp_gain

  // Silnik
  7,          // pole pairs
  168.0f,     // KV
  9.0f,       // phase resistance

  // Driver/Voltage
  12.0f,      // v_supply
  10.0f,      // v_limit
  20000,      // pwm_frequency [Hz]

  // Sterowanie
  0.1f, 0.05f, 0.0f, // PID P,I,D
  1000.0f,           // output ramp
  0.01f,             // LPF Tf
  1.0f,              // current_limit [A]

  // Startowy target
  10.0f              // [rad/s]
};

} // namespace AppConfig