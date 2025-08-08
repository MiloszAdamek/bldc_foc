#include <Arduino.h>
#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>

// === Gate Driver (IHM03M) ===
// Piny PWM dla faz U, V, W
#define PWM_U_PIN PA8
#define PWM_V_PIN PA9
#define PWM_W_PIN PA10

#define ENABLE_U_PIN PB13
#define ENABLE_V_PIN PB14
#define ENABLE_W_PIN PB15

// === SPI3 ===
#define SPI3_SCK_PIN    PC10
#define SPI3_MISO_PIN   PC11
#define SPI3_MOSI_PIN   PC12
#define SPI3_CS_PIN     PD2

// === Enkoder AS5048A ===

#define BIT_RESOLUTION  14
#define ANGLE_REGISTER  0x3FFF

// === ADC - Current Sense (Low-side) ===
#define CURR_AMPL_U_PIN PA1
#define CURR_AMPL_V_PIN PB1
#define CURR_AMPL_W_PIN PB0

#define SHOUNT_RESISTANCE 0.33
#define AMP_GAIN          1.528

// === MOTOR ===
#define POLE_PAIRS        7
#define KV_RATING         168
#define PHASE_RESISTANCE  9

// === Voltage ===
#define DRIVER_VOLTAGE_POWER_SUPPLY 11.0 // Napięcie zasilania sterownika [V]
#define DRIVER_VOLTAGE_LIMIT        10.0 // Maksymalne napięcie podawane na silnik [V]

// === Parameters ===
#define INITIAL_TARGET_VELOCITY 10 // Początkowa prędkość docelowa [rad/s]

// ===========================================
// SIMPLEFOC OBJECTS INITIALIZATION
// ===========================================

BLDCMotor motor = BLDCMotor(POLE_PAIRS, PHASE_RESISTANCE, KV_RATING);

SPIClass SPI_3(SPI3_MOSI_PIN, SPI3_MISO_PIN, SPI3_SCK_PIN);
MagneticSensorSPI AS5048A_sensor = MagneticSensorSPI(SPI3_CS_PIN, BIT_RESOLUTION, ANGLE_REGISTER);

BLDCDriver3PWM driver = BLDCDriver3PWM(PWM_U_PIN, PWM_V_PIN, PWM_W_PIN, ENABLE_U_PIN, ENABLE_V_PIN, ENABLE_W_PIN);
LowsideCurrentSense current_sense = LowsideCurrentSense(SHOUNT_RESISTANCE, AMP_GAIN, CURR_AMPL_U_PIN, CURR_AMPL_V_PIN, CURR_AMPL_W_PIN);

Commander command = Commander(Serial);
void doTarget(char* cmd){ command.scalar(&motor.target, cmd); }

// ===========================================
// SETUP - Program configuration
// ===========================================

void setup() {
  Serial.begin(115200);
  Serial.println("SimpleFOC G431RB: Starting configuration...");

  // --------- ENCODER --------
  SPI_3.begin();
  AS5048A_sensor.init(&SPI_3);
  motor.linkSensor(&AS5048A_sensor);
  Serial.println("Sensor initialized.");

  // --------- DRIVER --------
  driver.pwm_frequency = 20000; // Częstotliwość PWM [Hz]
  driver.voltage_power_supply = DRIVER_VOLTAGE_POWER_SUPPLY;
  driver.voltage_limit = DRIVER_VOLTAGE_LIMIT;
  driver.init(); // Inicjalizacja sterownika
  driver.enable(); // Włączenie sterownika
  motor.linkDriver(&driver);
  Serial.println("Driver initialized and enabled.");

  // --------- CURRENT SENSE --------
  current_sense.linkDriver(&driver); // Link current sense to the driver (for phase mapping)
  current_sense.init(); // Inicjalizacja pomiaru prądu
  motor.linkCurrentSense(&current_sense); // Link current sense to the motor
  Serial.println("Current sense initialized.");

  // --------- MOTOR --------
  motor.controller = MotionControlType::velocity; // set control loop type to be used
  motor.init(); // initialize motor

  // Konfiguracja PID dla kontroli prędkości (wymaga strojenia!)
  // Zacznij od małego P, zwiększaj stopniowo, potem dodaj I.
  motor.PID_velocity.P = 0.1;
  motor.PID_velocity.I = 0.05;
  motor.PID_velocity.D = 0.0;
  motor.PID_velocity.output_ramp = 1000; // Przyspieszenie zmian wyjścia PID

  // Filtr dolnoprzepustowy dla prędkości (ważny dla stabilności)
  motor.LPF_velocity.Tf = 0.01; // Czas filtru w sekundach (np. 0.01s = 10ms)

  // Ograniczenia dla pętli FOC (często przydatne, np. dla momentu/prądu)
  motor.current_limit = 1.0; // Prąd [A]

  // === Inicjalizacja FOC ===
  // Ta funkcja wykonuje wyrównanie czujnika i uruchamia timery PWM.
  // Bez tego silnik nie będzie działał poprawnie w trybie FOC.
  motor.initFOC();
  Serial.println("Motor FOC initialized!");

    // Ustawienie początkowego celu ruchu
  motor.target = INITIAL_TARGET_VELOCITY;
  Serial.print("Initial motor target set to: ");
  Serial.print(motor.target);
  Serial.println(" rad/s");

  // Opcjonalnie: Użyj monitorowania przez Serial
  motor.useMonitoring(Serial);
  motor.monitor_downsample = 100; // Wysyłaj dane co 100 cykli pętli FOC
  motor.monitor_variables = _MON_VEL | _MON_ANGLE | _MON_TARGET | _MON_CURR_Q | _MON_CURR_D | _MON_VOLT_Q | _MON_VOLT_D;

  command.add('T', doTarget, "target velocity [rad/s]");

  Serial.println("SimpleFOC configuration complete. Entering loop...");
  _delay(1000); // Małe opóźnienie na start
}

void loop() {
  // KLUCZOWE FUNKCJE SIMPLEFOC W PĘTLI
  // 1. Niskopoziomowa pętla FOC:
  //    Odczytuje czujnik, oblicza prądy i ustawia odpowiednie sygnały PWM.
  motor.loopFOC();

  // // 2. Wysokopoziomowa pętla sterowania ruchem:
  // //    Wykonuje algorytm PID (dla trybu velocity/angle) i oblicza
  // //    wymagany moment/prąd/napięcie, które następnie podawane są do loopFOC().
  motor.move();

  // // Opcjonalnie: Wyślij dane monitorowania do portu szeregowego
  motor.monitor(); // Odkomentuj, aby widzieć dane
  
  command.run();

  // // Odczytaj aktualny kąt mechaniczny (0-2PI radiany)
  // float angle_radians = AS5048A_sensor.getAngle();
  // // Odczytaj aktualną pozycję raw (liczby całkowite)
  // // int raw_angle = AS5048A_sensor.getRawAngle(); // Tylko dla niektórych sensorów lub bezpośredniego dostępu do rejestru

  // // Odczytaj prędkość (opcjonalnie, wymaga pewnej zmienności kąta by była sensowna)
  // float velocity_radians_per_sec = AS5048A_sensor.getVelocity();

  // // Wyświetl dane na monitorze szeregowym
  // // Serial.print("Angle (rad): ");
  // // Serial.print(angle_radians, 4); // Wyświetl z 4 miejscami po przecinku
  // // Serial.print("\tAngle (deg): ");
  // // Serial.print(angle_radians * 180.0 / PI, 2); // Konwersja na stopnie
  // // Serial.print("\tVelocity (rad/s): ");
  // // Serial.println(velocity_radians_per_sec, 2);

  // delay(500); // Małe opóźnienie, aby nie zasypać monitora danymi
}