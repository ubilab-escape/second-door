#include <Arduino.h>
#include <MqttBase.h>
#include <vector>
#include "wifi_secure.h"

#include <string>
#include <sstream>


#define ROBOT_ON 18

/// one pin measures the supply voltage the other the low voltage pin
#define BATTERY_PIN 35
#define BATTERY_LOW 34

float voltage = 0;

bool robot_on = false;

/// communication handling
const char *topic = "7/robot";
const char *mqtt_server = "10.0.0.2";
const char *mqtt_name = "robotClient";
MqttBase *mqtt_com;

/// FreeRTOS functions and handler
void TaskTalk2Robot(void *pvParameters);
void TaskCheckBattery(void *pvParameters);

TaskHandle_t xHandTalk2Robot;
TaskHandle_t xHandleCheckBattery;

/// to_string was not working -> 
template <typename T>
std::string to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

/// logic callback function for mqtt
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
  pinMode(BATTERY_LOW, INPUT);
  pinMode(ROBOT_ON, OUTPUT);
  /// keep robot off at begining
  digitalWrite(ROBOT_ON, LOW);

  /// communication intitialization
  std::vector<std::shared_ptr<std::string>> mqtt_topics;
  mqtt_topics.push_back(std::make_shared<std::string>(topic));
  std::vector<std::function<void(const char *, const char *, int)>> logic_callbacks;
  logic_callbacks.push_back(callback);
  mqtt_com = new MqttBase(mqtt_server, mqtt_name, 1883);
  mqtt_com->init(ssid, password, mqtt_topics, logic_callbacks);

  /// FreeRTOS intitialization
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
    /// suspend task to avoid reading interruption
    vTaskSuspend(xHandTalk2Robot);
    uint16_t pinVoltage = 0;
    float batLow = 0;
    for (uint8_t i = 0; i < 5; i++) {
      pinVoltage += analogRead(BATTERY_PIN);
    }
    pinVoltage = (int)pinVoltage / 5;
    voltage = (3.0f/2.0f) * 3.3 * ((float)pinVoltage/2048.0f);
    batLow = analogRead(BATTERY_LOW);
    batLow = 3.3 * ((float)batLow/2048.0f);
    if (batLow < 2.0) {
      std::string str = "robot battery too low : " + to_string(voltage);
      mqtt_com->publish(topic, "STATUS", "failed", str.c_str(), false);
    }
    vTaskResume(xHandTalk2Robot);
    vTaskDelay(3000);
  }
}