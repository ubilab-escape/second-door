#include "communication.h"
#include "wifi_secure.h"

const char* mqtt_topic = "7/laser";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //scan();
  init_com(ssid, password, "10.0.0.2");
  mqtt_subscribe(mqtt_topic);
}

void loop() {
  // put your main code here, to run repeatedly:
  comm_main();
}
