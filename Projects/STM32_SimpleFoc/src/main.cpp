#include "MotorController.hpp"
#include "Config.hpp"
#include "DriverSPI.hpp"
#include "spi1_hal_init.hpp"

MotorController foc(AppConfig::BoardConfig);

// DriverSPI driver(hspi1); // DriverSPI z naszym SPI1

void spiCommandHandler(uint8_t* data, size_t len) {
    data[len] = '\0';
    foc.feedCommand((char*)data);
}

void setup() {

    foc.begin();

    // MX_SPI1_Init_Master();  

    // MX_SPI1_Init_Slave();

    // driver.onCommand(spiCommandHandler);
    // driver.beginSlave();
}

void loop() {

    // const char cmd[] = "T 15\n";
    // driver.sendMaster((uint8_t*)cmd, strlen(cmd));
    // delay(1000);

    foc.update();
    foc.runCommand();
}

