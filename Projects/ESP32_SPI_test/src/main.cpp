#include <Arduino.h>
#include <AS5048A.h>
#include <SPI.h>

#define CS_PIN 22 // Chip Select pin

// put function declarations here:

AS5048A encoder(CS_PIN); // tworzysz obiekt enkodera z pinem CS

void setup() {

  Serial.begin(115200);
  SPI.begin();  // Używa domyślnych pinów: SCK=18, MISO=19, MOSI=23
  encoder.begin();  // inicjalizacja enkodera

  Serial.println("AS5048A ready");
}

void loop() {
  uint16_t angle = encoder.getRawRotation();  // 0 - 16383
  float degrees = (float)angle * 360.0 / 16383.0;

  String error = encoder.getErrors();
  if (error.length() > 0) {
      Serial.println("ERROR: " + error);
  }
  
  Serial.print("Raw: ");
  Serial.print(angle);
  Serial.print(" | Deg: ");
  Serial.println(degrees);

  delay(500);
}

