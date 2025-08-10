#pragma once
#include "stm32g4xx_hal.h"

// ===== Konfiguracja bufora SPI =====
#define SPI_BUF_SIZE 64                 // Rozmiar bufora odbiorczego
extern uint8_t spiBuf[SPI_BUF_SIZE];    // Globalny bufor odbiorczy SPI

// ===== Klasa DriverSPI =====
class DriverSPI {
public:
    DriverSPI(SPI_HandleTypeDef& hspi);

    // Rejestracja callbacka do obsługi odebranych danych
    void onCommand(void (*callback)(uint8_t*, size_t));

    // Inicjacja odbioru w trybie slave
    void beginSlave();

    // Wysyłanie danych w trybie master
    void sendMaster(const uint8_t* data, size_t len);

    // Gettery do prywatnych pól (używane w callbackach HAL)
    SPI_HandleTypeDef& getHandle() { return _hspi; }
    void (*getCommandCallback())(uint8_t*, size_t) { return _commandCallback; }

private:
    SPI_HandleTypeDef& _hspi;
    void (*_commandCallback)(uint8_t*, size_t) = nullptr;
};

// Globalny wskaźnik do instancji DriverSPI
extern DriverSPI* gDriverSPI;