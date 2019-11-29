#include "MAX7221.h"
#include <SPI.h>


MAX7221::MAX7221(uint8_t csPin, uint8_t numberOfDigits = 1) {
    cs_pin_ = csPin;
    number_of_digits_ = numberOfDigits;
}

void MAX7221::transferData(uint8_t address, uint8_t value) {

    // TODO: Implement daisy chaining

    digitalWrite(cs_pin_, LOW);
    SPI.transfer(address);
    SPI.transfer(value);
    digitalWrite(cs_pin_, HIGH);
}

void MAX7221::initMAX() {
    pinMode(cs_pin_, OUTPUT);
    
    SPI.setBitOrder(MSBFIRST);
    SPI.begin();

    // enable decode mode
    transferData(0x09, 0x0F); // Code B decode for digits 3-0 No Decode for digits 7 - 4

    // set intensity
    transferData(0x0A, 0x06); // Duty Cycle 1/16 (min on) 

    // set scan-limit
    transferData(0x0B, 0x03); // Display Digits 0 1 2 3

    // enter normal operation mode
    transferData(0x0C, 0x01); // Normal Operation   
    Serial.println("MAX init done");
}
