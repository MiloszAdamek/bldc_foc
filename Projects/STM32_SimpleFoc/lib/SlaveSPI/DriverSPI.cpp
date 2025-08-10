#include "DriverSPI.hpp"

// ===== Definicje zmiennych globalnych =====
DriverSPI* gDriverSPI = nullptr;           // Wskaźnik globalny do obiektu DriverSPI
uint8_t spiBuf[SPI_BUF_SIZE];               // Globalny bufor odbiorczy SPI

// ===== Implementacja DriverSPI =====
DriverSPI::DriverSPI(SPI_HandleTypeDef& hspi) : _hspi(hspi) {
    gDriverSPI = this;
}

void DriverSPI::onCommand(void (*callback)(uint8_t*, size_t)) {
    _commandCallback = callback;
}

void DriverSPI::beginSlave() {
    // Rozpoczęcie pierwszego odbioru w trybie przerwań
    HAL_SPI_Receive_IT(&_hspi, spiBuf, SPI_BUF_SIZE);
}

void DriverSPI::sendMaster(const uint8_t* data, size_t len) {
    // Opuszczenie CS
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    // Wysyłka danych
    HAL_SPI_Transmit(&_hspi, const_cast<uint8_t*>(data), len, 100);
    // Podniesienie CS
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

// ===== Callback HAL po odebraniu danych przez SPI (Slave) =====
extern "C" void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (gDriverSPI && hspi == &gDriverSPI->getHandle()) {
        if (gDriverSPI->getCommandCallback()) {
            gDriverSPI->getCommandCallback()(spiBuf, SPI_BUF_SIZE);
        }
        // Restart odbioru
        HAL_SPI_Receive_IT(hspi, spiBuf, SPI_BUF_SIZE);
    }
}