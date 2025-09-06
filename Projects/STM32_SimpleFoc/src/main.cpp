#include "MotorController.hpp"
#include "Config.hpp"


MotorController foc(AppConfig::BoardConfig);

void setup() {
    foc.begin();

    foc.runAlignmentTest();
}

void loop() {
    // foc.update();
    // foc.runCommand();
}