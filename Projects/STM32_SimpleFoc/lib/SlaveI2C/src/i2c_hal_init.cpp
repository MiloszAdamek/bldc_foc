#include "i2c_hal_init.hpp"

I2C_HandleTypeDef hi2c1;

extern "C" void MX_I2C1_Init(void) {

    // 1. Włącz zegary dla GPIOB oraz I2C1
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    // 2. Konfiguracja pinów PB6 (SCL) i PB7 (SDA) w trybie alternatywnym AF4 I2C1
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;  
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3. Konfiguracja struktury I2C1 (bazowana na CubeMX)
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x40B285C2; // Wartość z CubeMX (dopasowana do zegara konfigurowanego w Cube)
    // hi2c1.Init.OwnAddress1 = (0x12 << 1); // <-- Twój adres Slave (zmień wg potrzeb)
    hi2c1.Init.OwnAddress1 = 0; // 0 jeśli MASTER
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        while (1);
    }

    // 4. Konfiguracja filtrów (tak samo jak w CubeMX)
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        while (1);
    }

    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
        while (1);
    }

    // 5. Włączenie przerwań I2C1 w NVIC (na wypadek trybu IT)
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
}

extern "C" void MX_I2C1_Init_Master(void) {

    // 1. Włącz zegary dla GPIOB i I2C1
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    // 2. Konfiguracja pinów PB8 (SCL) i PB9 (SDA) jako AF Open-Drain z pull-up
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;              // Alternate Function, Open-Drain
    GPIO_InitStruct.Pull = GPIO_PULLUP;                  // Wewnętrzny pull-up (lepiej mieć też fizyczne rezystory)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;           // AF4 = I2C1
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3. Konfiguracja parametrów I2C1 w trybie Master
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x40B285C2;                      // Timing z CubeMX dla PCLK1=170MHz, 100kHz I2C
    hi2c1.Init.OwnAddress1 = 0;                          // Master nie potrzebuje własnego adresu
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        while (1); // Błąd inicjalizacji
    }

    // 4. Konfiguracja filtrów
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        while (1);
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
        while (1);
    }

    // 5. Włączenie przerwań I2C1 w NVIC (na wypadek trybu IT)
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
}