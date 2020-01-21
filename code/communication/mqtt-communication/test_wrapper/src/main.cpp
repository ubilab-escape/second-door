
#include <Arduino.h>
#include <mqtt_base.h>
#include "wifi_secure.h"

//const char* ssid = "Thomass iPhone";
//const char* pw = "12345670";

mqtt_base *base;


void setup() {
  Serial.begin(115200);
  base = new mqtt_base();
  base->init(ssid, password, "7/laser");
}

void loop() {
  base->loop();
  //base->publish("TRIGGER", "ON");
  delay(1000);

  // put your main code here, to run repeatedly:
}