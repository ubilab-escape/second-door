#include <Arduino.h>

#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  // put your setup code here, to run once:
}

void loop() {
  Serial2.print(0);
  delay(1000);
  // put your main code here, to run repeatedly:
}