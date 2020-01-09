#ifndef MAX7221_H
#define MAX7221_H

#include <Arduino.h>
#include <stdint.h>

class MAX7221 {

public:
  enum hardwareType_t { LEDMATRIX, SEGMENT };

  MAX7221(uint8_t csPin, uint8_t numberOfDevices, hardwareType_t hardwareType);

  void initMAX();
  void transferData(uint8_t device, uint8_t address, uint8_t value);
  void transferData(uint8_t address, uint8_t value);

protected:
private:
  uint8_t cs_pin_;
  uint8_t number_of_devices_;
  hardwareType_t hardware_type_;
  uint8_t spi_value_[8];
  uint8_t spi_address_[8];

};

#endif