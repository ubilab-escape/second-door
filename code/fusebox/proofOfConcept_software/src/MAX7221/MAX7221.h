#ifndef MAX7221_H
#define MAX7221_H

#include <stdint.h>
#include <Arduino.h>

class MAX7221 {

public:
    MAX7221(uint8_t csPin, uint8_t numberOfDigits);

    void initMAX();
    void transferData(uint8_t address, uint8_t value);

protected:
   

private:
    uint8_t cs_pin_;
    uint8_t number_of_digits_;
};

#endif