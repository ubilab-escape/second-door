#include "MAX7221.h"
#include <SPI.h>

MAX7221::MAX7221(uint8_t csPin, uint8_t numberOfDevices,
                 hardwareType_t hardwareType) {
  cs_pin_ = csPin;
  number_of_devices_ = numberOfDevices;
  hardware_type_ = hardwareType;
}

void MAX7221::transferData(uint8_t address, uint8_t value) {
  for (uint8_t i = 0; i < number_of_devices_; i++) {
    transferData(i, address, value);
  }
}

void MAX7221::transferData(uint8_t device, uint8_t address, uint8_t value) {
  uint8_t offset = device;
  uint8_t maxBytes = number_of_devices_;

  for (uint8_t i = 0; i < maxBytes; i++) {
    spi_address_[i] = 0;
    spi_value_[i] = 0;
  }

  spi_address_[offset] = address;
  spi_value_[offset] = value;

  digitalWrite(cs_pin_, LOW);
  for (uint8_t i = 0; i < maxBytes; i++) {
    SPI.transfer(spi_address_[i]);
    SPI.transfer(spi_value_[i]);
  }
  digitalWrite(cs_pin_, HIGH);
}





void MAX7221::initMAX() {

  Serial.print("MAX Init started ...");

  pinMode(cs_pin_, OUTPUT);

  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  SPI.begin();

  switch (hardware_type_) {
  case LEDMATRIX:
    // disable decode mode
    transferData(0x09, 0x00); // No decode for digits 7 - 0

    // set intensity
    transferData(0x0A, 0x00);

    // set scan-limit
    transferData(0x0B, 0x07); // Display Digits 0 1 2 3 4 5 6 7

    break;

  case SEGMENT:
    // enable decode mode
    transferData(
        0x09, 0x0F); // Code B decode for digits 3-0 No Decode for digits 7 - 4

    // set intensity
    transferData(0x0A, 0x06);

    // set scan-limit
    transferData(0x0B, 0x03); // Display Digits 0 1 2 3

    break;

  default:
    break;
  }

  // disable Display-Test
  transferData(0x0F, 0x00); // Normal Operation

  // enter normal operation mode
  transferData(0x0C, 0x01); // Normal Operation

  Serial.println(" done!");
}
