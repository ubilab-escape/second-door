/*
 * This
 */
#include <Arduino.h>
#include <MqttBase.h>
#include <vector>
#include "wifi_secure.h"

#include <string>
#include <sstream>

/* Pin needs at least 2.5V (pinVoltage = 1500)!
 * If voltage at pin is lower the robot will not be controllable anymore.
 * For safty the robot will publish low voltage in case of pinVoltage < 1600
 */

#define ROBOT_ON 18

#define BATTERY_PIN 35

#define DEBUG

const char *topic = "7/robot";
const char *mqtt_server = "10.0.0.2";
const char *mqtt_name = "robotClient";

float voltage = 0;

MqttBase *mqtt_com;

bool robot_on = false;

void TaskTalk2Robot(void *pvParameters);
void TaskCheckBattery(void *pvParameters);

TaskHandle_t xHandTalk2Robot;
TaskHandle_t xHandleCheckBattery;

template <typename T>
std::string to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

void callback(const char *method1, const char *state, int daten) {
  if (caseInSensStringCompare(method1, "TRIGGER")) {
    if (caseInSensStringCompare(state, "on")) {
      robot_on = true;
      std::string str = "robot "+ to_string(voltage);
      mqtt_com->publish(topic, "STATUS", "active", str.c_str(), false);
    }
    if (caseInSensStringCompare(state, "off")) {
      robot_on = false;
      mqtt_com->publish(topic, "STATUS", "inactive", "robot", false);
    }
  }
}

void setup() {
  Serial.begin(115200);

  analogReadResolution(11);
  pinMode(BATTERY_PIN, INPUT);
  pinMode(ROBOT_ON, OUTPUT);

  std::vector<std::shared_ptr<std::string>> mqtt_topics;
  mqtt_topics.push_back(std::make_shared<std::string>(topic));
  std::vector<std::function<void(const char *, const char *, int)>> logic_callbacks;
  logic_callbacks.push_back(callback);

  mqtt_com = new MqttBase(mqtt_server, mqtt_name, 1883);
  mqtt_com->init(ssid, password, mqtt_topics, logic_callbacks);

  xTaskCreatePinnedToCore(TaskTalk2Robot, "Talk2Robot", 8192, NULL, 3, &xHandTalk2Robot, 0);
  xTaskCreatePinnedToCore(TaskCheckBattery, "CheckBattery", 8192, NULL, 2, &xHandleCheckBattery, 1);
}

void loop() {
  mqtt_com->loop();
  vTaskDelay(500);
}

void TaskTalk2Robot(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    /// caused by mosfet inverted logic
    if (robot_on == true) {
      digitalWrite(ROBOT_ON, LOW);
    }
    if (robot_on == false) {
      digitalWrite(ROBOT_ON, HIGH);
    }
    vTaskDelay(500);
  }
}

void TaskCheckBattery(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    vTaskSuspend(xHandTalk2Robot);
    uint16_t pinVoltage = 0;
    for (uint8_t i = 0; i < 5; i++) {
      pinVoltage += analogRead(BATTERY_PIN);
    }
    pinVoltage = (int)pinVoltage / 5;
    voltage = (3.0f/2.0f) * 3.3 * ((float)pinVoltage/2048.0f);
    Serial.println(voltage);
    if (voltage < 4.0) {
      std::string str = "robot battery too low : "+ to_string(voltage);

      mqtt_com->publish(topic, "STATUS", "failed", str.c_str(), false);
    }
    vTaskResume(xHandTalk2Robot);
    vTaskDelay(3000);
  }
}