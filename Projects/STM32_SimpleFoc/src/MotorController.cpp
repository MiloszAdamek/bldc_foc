#include "MotorController.hpp"

MotorController* MotorController::instance = nullptr;

MotorController::MotorController(const AppConfig::MotorConfig& cfg)
  : config(cfg),
    motor(cfg.pole_pairs, cfg.phase_resistance, cfg.kv_rating),
    driver(cfg.pwm_u, cfg.pwm_v, cfg.pwm_w, cfg.en_u, cfg.en_v, cfg.en_w),
    current_sense(cfg.shunt_resistance, cfg.amp_gain, cfg.curr_u, cfg.curr_v, cfg.curr_w),
    sensor(cfg.spi3_cs, cfg.bit_resolution, cfg.angle_register),
    // Uwaga: jeżeli Twój core wymaga jawnego wyboru peryferium, użyj wersji:
    // SPIClass spi3(SPI3, cfg.spi3_mosi, cfg.spi3_miso, cfg.spi3_sck);
    spi3(cfg.spi3_mosi, cfg.spi3_miso, cfg.spi3_sck),
    command(Serial)
{
  instance = this;
}

void MotorController::begin() {
  Serial.begin(115200);
  Serial.println("SimpleFOC G431RB: Starting configuration...");

  // ENCODER
  spi3.begin();
  sensor.init(&spi3);
  motor.linkSensor(&sensor);
  Serial.println("Sensor initialized.");

  // DRIVER
  driver.pwm_frequency = config.pwm_frequency;
  driver.voltage_power_supply = config.v_supply;
  driver.voltage_limit = config.v_limit;
  driver.init();
  driver.enable();
  motor.linkDriver(&driver);
  Serial.println("Driver initialized and enabled.");

  // CURRENT SENSE
  current_sense.linkDriver(&driver);
  current_sense.init();
  motor.linkCurrentSense(&current_sense);
  Serial.println("Current sense initialized.");

  SimpleFOCDebug::enable(&Serial);

  // MOTOR
  motor.controller = MotionControlType::torque;
  motor.torque_controller = TorqueControlType::foc_current;
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
  motor.init();

  // PID / FILTRY / LIMITY

  motor.P_angle.P = 5.0f;

  // PID - velocity
  motor.PID_velocity.P = config.pid_v_p;
  motor.PID_velocity.I = config.pid_v_i;
  motor.PID_velocity.D = config.pid_v_d;
  motor.PID_velocity.output_ramp = config.pid_v_output_ramp;
  motor.LPF_velocity.Tf = config.lpf_velocity_Tf;
  motor.current_limit = config.current_limit;

  // PID - torque -> Id, Iq

  // Q axis
  motor.PID_current_q.P = config.pid_iq_p;                        
  motor.PID_current_q.I = config.pid_iq_i;                        
  motor.PID_current_q.D = config.pid_iq_d;
  motor.PID_current_q.limit = motor.voltage_limit; 
  motor.PID_current_q.output_ramp = config.pid_iq_output_ramp;    
  motor.LPF_current_q.Tf= config.lpf_iq_Tf;                      

  // D axis
  motor.PID_current_d.P = config.pid_id_p;                        
  motor.PID_current_d.I = config.pid_id_i;                        
  motor.PID_current_d.D = config.pid_id_d;
  motor.PID_current_d.limit = motor.voltage_limit; 
  motor.PID_current_d.output_ramp = config.pid_id_output_ramp;    
  motor.LPF_current_d.Tf= config.lpf_id_Tf;                       

  // // // FOC init
  // motor.initFOC();
  // // Serial.println("Motor FOC initialized!");

  // // // Target startowy
  // motor.target = config.initial_target_velocity;
  // Serial.print("Initial motor target set to: ");
  // Serial.print(motor.target);
  // Serial.println(" rad/s");

  // // Monitoring (opcjonalnie)
  // motor.useMonitoring(Serial);
  // motor.monitor_downsample = 100;
  // motor.monitor_variables = _MON_VEL | _MON_ANGLE | _MON_TARGET | _MON_CURR_Q | _MON_CURR_D | _MON_VOLT_Q | _MON_VOLT_D;

  // Commander
  command.add('T', onTargetCmd, "target velocity [rad/s]");
  command.add('M', onModeCmd, "mode: 0-torque, 1-velocity, 2-angle"); 
  command.add('C', onTargetCmd, "target current [A]"); // Zadanie prądu w trybie torque

  _delay(1000);
  Serial.println("SimpleFOC configuration complete. Entering loop...");
}

void MotorController::update() {
  motor.loopFOC();
  motor.move();
  // opcjonalnie: motor.monitor();
}

void MotorController::runCommand() {
  command.run();
}

void MotorController::setTarget(float rad_s) {
  motor.target = rad_s;
}

void MotorController::onTargetCmd(char* cmd) { // Velocity target in velocity mode
  if (instance) {
    instance->command.scalar(&instance->motor.target, cmd);
  }
}

void MotorController::onCurrentCmd(char* cmd) { // Current target in torque mode
  if (instance) {
    instance->command.scalar(&instance->motor.target, cmd);
  }
}

void MotorController::onModeCmd(char* cmd) {
  if (instance) {
    int mode = atoi(cmd); // zamiana tekstu na liczbę
    switch (mode) {
      case 0:
        instance->motor.controller = MotionControlType::torque;
        Serial.println("Mode: torque");
        break;
      case 1:
        instance->motor.controller = MotionControlType::velocity;
        Serial.println("Mode: velocity");
        break;
      case 2:
        instance->motor.controller = MotionControlType::angle;
        Serial.println("Mode: angle");
        break;
      default:
        Serial.println("Unknown mode!");
        break;
    }
  }
}

void MotorController::feedCommand(char* cmdString) {
  command.run(cmdString);
}

// W pliku lib/MotorControllerLib/MotorController.cpp

void MotorController::runAlignmentTest() {
    // --- ZDEFINIUJ TABLICE TUTAJ, WEWNĄTRZ FUNKCJI ---
    static const float voltageAlign[] = { 3.0f, 6.0f }; // Tablica napięć testowych
    
    // TABLICA ZDEFINIOWANA W STOPNIACH - bardziej czytelna
    static const int positionAlignDeg[] = { 45, 180, 270};

    Serial.println("\n--- Rozpoczynam procedure testowania kalibracji ---");
    
    // Ustaw tryb ANGLE, aby móc precyzyjnie pozycjonować silnik
    motor.controller = MotionControlType::angle;
    Serial.println("Tryb sterowania ustawiony na: ANGLE");

    // Pętla po różnych napięciach kalibracji
    for (int i = 0; i < 2; i++) {
        float current_voltage = voltageAlign[i];
        Serial.printf("\n============================================\n");
        Serial.printf("  TESTUJE DLA NAPIECIA: %.1f V\n", current_voltage);
        Serial.printf("============================================\n");
        
        motor.voltage_sensor_align = current_voltage;

        // Pętla po różnych pozycjach startowych
        for (int j = 0; j < 3; j++) {
            // Pobierz pozycję w stopniach z tablicy
            int target_position_deg = positionAlignDeg[j];
            
            // Przelicz pozycję na radiany - TYLKO do użycia w `motor.target`
            float target_position_rad = (float)target_position_deg * DEG_TO_RAD;

            // Loguj w stopniach dla czytelności
            Serial.printf("\n--- Test dla pozycji startowej: %d deg (%.2f rad) ---\n", target_position_deg, target_position_rad);

            // --- Krok 1: Dojedź do pozycji startowej ---
            Serial.println("  1. Dojazd do pozycji startowej...");
            
            motor.target = target_position_rad; // Ustaw target w radianach
            long move_start_time = millis();
            
            while (millis() - move_start_time < 3000) {
                motor.loopFOC();
                motor.move();
                // Porównuj w radianach
                if (abs(target_position_rad - motor.shaft_angle) < 0.02) {
                    break;
                }
            }
            // Loguj wynik końcowy w stopniach i radianach
            Serial.printf("     -> Pozycja koncowa: %.3f rad (%.1f deg)\n", motor.shaft_angle, motor.shaft_angle * RAD_TO_DEG);
            HAL_Delay(500);

            // --- Krok 2: Uruchom kalibrację ---
            Serial.println("  2. Reset i uruchomienie initFOC()...");
            motor.zero_electric_angle = NOT_SET;
            _delay(1000);
            motor.initFOC();
            _delay(1000);
            
            // Loguj wynik (offset) w radianach (standard) i stopniach (dla łatwiejszej interpretacji)
            Serial.print("  3. >>> WYNIK: Znaleziony offset: ");
            Serial.print(motor.zero_electric_angle, 4);
            Serial.print(" rad (");
            Serial.print(motor.zero_electric_angle * RAD_TO_DEG, 2);
            Serial.println(" deg) <<<");
        }
    }

    Serial.println("\n--- TEST ZAKONCZONY ---");
}