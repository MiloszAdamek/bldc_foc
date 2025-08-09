#include "MotorController.hpp"
#include "Config.hpp"

MotorController foc(AppConfig::BoardConfig);

void setup() {
  foc.begin();
}

void loop() {
  foc.update();
  foc.runCommand(); // w monitorze szeregowym: "T 15" => 15 rad/s
}