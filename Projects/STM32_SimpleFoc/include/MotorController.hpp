#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include "Config.hpp"

class MotorController {
public:
  explicit MotorController(const AppConfig::MotorConfig& cfg);

  void begin();        // wywołaj w setup()
  void update();       // wywołuj w loop() - FOC + sterowanie
  void runCommand();   // wywołuj w loop() - Commander

  void setTarget(float rad_s);
  float getTarget() const { return motor.target; }

private:
  const AppConfig::MotorConfig config;

  // Commander: statyczny wskaźnik do instancji + statyczny callback
  static MotorController* instance;
  static void onTargetCmd(char* cmd);

  // Obiekty SimpleFOC
  BLDCMotor motor;
  BLDCDriver3PWM driver;
  LowsideCurrentSense current_sense;
  MagneticSensorSPI sensor;

  // SPI3
  SPIClass spi3;

  // Commander
  Commander command;
};