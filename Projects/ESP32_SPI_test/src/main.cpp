#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <SPI.h>

// --- Konfiguracja pinów VSPI ---
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CS     5

// --- Parametry UART ---
#define UART_BAUD     115200
#define MAX_QUEUE_LEN 32   // max liczba komunikatów w kolejce
#define BUF_LEN       64   // długość wiadomości

// --- Obiekty RTOS ---
QueueHandle_t spiQueue;
SPIClass spi(VSPI); // jawnie wybieramy VSPI

// --- Task odbioru UART i wrzucania do kolejki ---
void uartTask(void *pvParameters) {
  char buf[BUF_LEN];
  uint8_t idx = 0;

  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();

      if (c == '\n' || c == '\r') {
        if (idx > 0) {
          buf[idx] = '\0';
          if (xQueueSend(spiQueue, buf, portMAX_DELAY) != pdTRUE) {
            Serial.println("Queue full!");
          }
          idx = 0;
        }
      } else {
        if (idx < BUF_LEN - 1) {
          buf[idx++] = c;
        }
      }
    }
    vTaskDelay(1);
  }
}

// --- Task pobierania z kolejki i wysyłania po SPI ---
void spiTask(void *pvParameters) {
  char buf[BUF_LEN];

  for (;;) {
    if (xQueueReceive(spiQueue, buf, portMAX_DELAY) == pdTRUE) {
      
      spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
      digitalWrite(PIN_CS, LOW);

      for (size_t i = 0; i < strlen(buf); i++) {
        spi.transfer(buf[i]);
      }

      digitalWrite(PIN_CS, HIGH);
      spi.endTransaction();

      Serial.print("Sent via SPI: ");
      Serial.println(buf);
    }
  }
}

void setup() {
  Serial.begin(UART_BAUD);
  while (!Serial) {}

  // SPI init na jawnie wskazanych pinach
  spi.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);

  // kolejka na BUF_LEN bajtów
  spiQueue = xQueueCreate(MAX_QUEUE_LEN, BUF_LEN);
  if (spiQueue == NULL) {
    Serial.println("Queue creation failed!");
    for(;;);
  }

  // uruchamiamy taski FreeRTOS
  xTaskCreate(uartTask, "UART Task", 4096, NULL, 1, NULL);
  xTaskCreate(spiTask,  "SPI Task",  4096, NULL, 1, NULL);

  Serial.println("System ready. Type text + Enter to send over SPI.");
}

void loop() {
  // pusty - rządzi RTOS
}

// void setup() {
//   Serial.begin(115200);
//   spi.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
//   pinMode(PIN_CS, OUTPUT);
//   digitalWrite(PIN_CS, HIGH);
// }

// void loop() {
//   spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
//   digitalWrite(PIN_CS, LOW);
//   spi.transfer(0xAA);
//   spi.transfer(0x55);
//   digitalWrite(PIN_CS, HIGH);
//   spi.endTransaction();
//   delay(1000);
// }