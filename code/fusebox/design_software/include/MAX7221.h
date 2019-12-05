#ifndef MAX7221_H
#define MAX7221_H

#include <stdint.h>
#include <Arduino.h>

class MAX7221 {

public:


    enum hardwareType_t {LEDMATRIX, SEGMENT};

    MAX7221(uint8_t csPin, uint8_t numberOfDevices, hardwareType_t hardwareType);

    void commit();
    void clear();
    void setPixel(uint8_t x, uint8_t y);
    void setColumn(uint8_t column, uint8_t value);

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

    uint8_t *cols;
};

#endif