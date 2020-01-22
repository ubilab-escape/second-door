
#include <Arduino.h>
#include "wifi_secure.h"
#include "string.h"
#include "MqttBase.h"

void blink();
void on();
void off();
void solved();
void callback(const char* method1, const char* state, int daten);
int main();

#define LED 18
#define BUTTON 21 

const char* mqtt_topic = "7/robot";
const char* mqtt_server = "10.0.0.2";
const char* mqtt_client_name = "robotButton";

int status = 0;

MqttBase* mqtt_com;

void blink() {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
}

void on(){
    if (digitalRead(BUTTON)) {
      status = 3;
      mqtt_com->publish(mqtt_topic,"STATUS", "solved", 1);
    } else {
      digitalWrite(LED, HIGH);
    }
  
}

void off(){
    digitalWrite(LED, LOW);
}

void solved(){
    blink();
}

void callback(const char* method1, const char* state, int daten) {
  Serial.println("LOGIC CALLBACK");
  if (strcmp(method1, "TRIGGER") == 0) {
    if (strcmp(state, "off") == 0) {
      // configure LED PWM functionalitites
        status = 0;
        mqtt_com->publish(mqtt_topic,"STATUS", "inactive", 0);
      }
    if (strcmp(state, "on") == 0) {
      // configure LED PWM functionalitites
        status = 1;
        mqtt_com->publish(mqtt_topic,"STATUS", "active", 0);
      }
    }

    if (strcmp(method1, "STATUS") == 0) {
      if (strcmp(state, "solved") == 0) {
          status = 3;
          //mqtt_com->publish(mqtt_topic,"STATUS", "solved", 1);
        }
    }  
  }

int main(){

  switch ( status ){
        case 1:
          on();
          break;
        case 3:
          solved();
        default: off();
    }

    return 0;
}


void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLDOWN);
  // fill up vector with all topic names
  std::vector<std::shared_ptr<std::string>> mqtt_topics;
  std::string topic = "7/robot";
  mqtt_topics.push_back(std::make_shared<std::string>(topic));

  // fill up vector with all logic callback functions
  std::vector<std::function<void(const char*, const char*, int)>> logic_callbacks;
  logic_callbacks.push_back(callback);

  mqtt_com = new MqttBase(mqtt_server, mqtt_client_name, 1883);
  mqtt_com->init(ssid, password, mqtt_topics, logic_callbacks);
}

void loop() {

  mqtt_com->loop();
  main();
}