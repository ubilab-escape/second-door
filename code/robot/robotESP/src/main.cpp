/*
 * This
 */
#include <Arduino.h>
#include <MqttBase.h>
#include <vector>
#include "wifi_secure.h"

#define ROBOT_ON 18 

std::string topic = "7/robot";
const char *mqtt_server = "10.0.0.2";
const char *mqtt_name = "robotClient";

MqttBase *mqtt_com;

bool robot_on = false;

void TaskTalk2Robot(void *pvParameters);

void callback(const char *method1, const char *state, int daten) {
  if (strcmp(method1, "TRIGGER") == 0) {
    if (strcmp(state, "on") == 0) {
      robot_on = true;
    }
    if (strcmp(state, "off") == 0) {
      robot_on = false;
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ROBOT_ON, OUTPUT);

  xTaskCreate(TaskTalk2Robot, "Talk2Robot", 1024, NULL, 2, NULL);

  std::vector<std::shared_ptr<std::string>> mqtt_topics;
  mqtt_topics.push_back(std::make_shared<std::string>(topic));
  std::vector<std::function<void(const char *, const char *, int)>> logic_callbacks;
  logic_callbacks.push_back(callback);

  mqtt_com = new MqttBase(mqtt_server, mqtt_name, 1883);
  mqtt_com->init(ssid, password, mqtt_topics, logic_callbacks);
}

void loop() {
  mqtt_com->loop();
  vTaskDelay(500);
}

void TaskTalk2Robot(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    if (robot_on == true) {
      digitalWrite(ROBOT_ON, HIGH);
    }
    if (robot_on == false) {
      digitalWrite(ROBOT_ON, LOW);
    }
    vTaskDelay(500);
  }
}