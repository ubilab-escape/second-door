/**
 * http://henrysbench.capnfatz.com/henrys-bench/arduino-projects-tips-and-more/arduino-ide-nodemcu-esp-12e-digital-input-basics/#The_pinMode_Command
 */
#include <Arduino.h>

// #define LED 5
// #define BUTTON 6

void blink() {
  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(5, LOW);
  delay(100);
}

void setup() {
  Serial.begin(9600);
  pinMode(5, OUTPUT);
  pinMode(16, INPUT_PULLDOWN_16);
}

void loop() {
  if (digitalRead(16)) {
    blink();
  } else {
    digitalWrite(5, HIGH);
  }
}