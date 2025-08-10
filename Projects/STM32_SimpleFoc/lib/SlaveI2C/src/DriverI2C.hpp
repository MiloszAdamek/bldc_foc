#pragma once
#include <cstdint>
#include <cstddef>
#include "stm32g4xx_hal.h"

class DriverI2C {
public:
    DriverI2C(I2C_HandleTypeDef& hi2c, uint8_t* buffer, size_t bufSize);

    void begin();
    void onCommand(void (*callback)(uint8_t*, size_t));

    // Getter rozmiaru bufora
    size_t getBufSize() const { return _bufSize; }

    // Nasz wewnętrzny HAL-callback (statyczny)
    static void HAL_RxCpltCallback(I2C_HandleTypeDef *hi2c);

private:
    I2C_HandleTypeDef& _hi2c;
    uint8_t* _buf;
    size_t _bufSize;
    void (*_commandCallback)(uint8_t*, size_t) = nullptr;
};

// globalny wskaźnik
extern DriverI2C* gDriverI2C;