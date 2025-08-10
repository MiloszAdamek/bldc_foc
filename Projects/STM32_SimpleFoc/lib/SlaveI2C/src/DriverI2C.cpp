#include "DriverI2C.hpp"

DriverI2C* gDriverI2C = nullptr;

DriverI2C::DriverI2C(I2C_HandleTypeDef& hi2c, uint8_t* buffer, size_t bufSize)
    : _hi2c(hi2c), _buf(buffer), _bufSize(bufSize) {
    gDriverI2C = this;
}

void DriverI2C::begin() {
    // Rejestrujemy nasz callback dla zdarzenia "RX complete"
    HAL_I2C_RegisterCallback(&_hi2c, HAL_I2C_SLAVE_RX_COMPLETE_CB_ID, DriverI2C::HAL_RxCpltCallback);

    // Startujemy w trybie odbioru
    HAL_I2C_Slave_Receive_IT(&_hi2c, _buf, _bufSize);
}

void DriverI2C::onCommand(void (*callback)(uint8_t*, size_t)) {
    _commandCallback = callback;
}

void DriverI2C::HAL_RxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (gDriverI2C && hi2c == &gDriverI2C->_hi2c) {
        if (gDriverI2C->_commandCallback) {
            gDriverI2C->_commandCallback(gDriverI2C->_buf, gDriverI2C->_bufSize);
        }
        // Restart odbioru
        HAL_I2C_Slave_Receive_IT(&gDriverI2C->_hi2c, gDriverI2C->_buf, gDriverI2C->_bufSize);
    }
}