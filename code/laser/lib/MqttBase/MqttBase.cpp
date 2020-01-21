#include "MqttBase.h"

MqttBase::MqttBase(const char* mqtt_server, uint16_t mqtt_port) {
  mqtt_server_ = mqtt_server;
  mqtt_port_ = mqtt_port;
}

MqttBase::~MqttBase() {}

void MqttBase::init(const char* ssid, const char* password, const char* topic,
                    void (*logic_callback)(const char*, const char*, int)) {
  mqtt_topic_ = topic;
  Serial.print("CONNECT TO WIFI");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debug_print(".");
  }
  debug_println("CONNECTED");

  pub_client_ = new PubSubClient();
  wifi_client_ = new WiFiClient();
  pub_client_->setClient(*wifi_client_);
  pub_client_->setServer(mqtt_server_, mqtt_port_);
  pub_client_->setCallback([this](char* topic, byte* payload, unsigned int length) {
    this->callback(topic, payload, length);
  });
  pub_client_->subscribe(mqtt_topic_);
  this->logic_callback_ = logic_callback;
}

void MqttBase::reconnect() {
  while (!pub_client_->connected()) {
    debug_print("Attempting MQTT connection...");
    // Attempt to connect
    if (pub_client_->connect("ESP32Client")) {
      debug_println("connected");
      // Subscribe
      pub_client_->subscribe(mqtt_topic_);
    } else {
      debug_print("failed, rc=");
      debug_print(pub_client_->state());
      debug_println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void MqttBase::loop() {
  if (!pub_client_->connected()) {
    reconnect();
  }
  pub_client_->loop();
}

void MqttBase::callback(char* topic, byte* message, unsigned int length) {
  debug_print("Message arrived on topic: ");
  debug_print(mqtt_topic_);
  debug_print(". Message: ");
  String messageTemp;
/*
  for (int i = 0; i < length; i++) {
    debug_print((char)message[i]);
    messageTemp += (char)message[i];
  }
  debug_println("");
*/
  deserializeJson(doc_, message);
  const char* method1 = doc_["method"];
  const char* state = doc_["state"];
  int daten = doc_["data"];

  debug_print("Methode: ");
  debug_println(method1);
  debug_print("State: ");
  debug_println(state);
  debug_print("Daten: ");
  debug_println(daten);

  logic_callback_(method1, state, daten);
}

void MqttBase::publish(const char* methode, const char* state) {
  JSONencoder_["method"] = methode;
  JSONencoder_["state"] = state;
  JSONencoder_["data"] = 0;

  char JSONmessageBuffer[100];

  serializeJson(doc_, JSONmessageBuffer, 100);
  debug_print("send message");
  debug_println(JSONencoder_);
  pub_client_->publish(mqtt_topic_, JSONmessageBuffer);
}

void MqttBase::debug_print(const char* str) {
#ifdef DEBUG
  Serial.print(str);
#endif
}

void MqttBase::debug_print(int i) {
#ifdef DEBUG
  Serial.print(i);
#endif
}

void MqttBase::debug_println(const char* str) {
#ifdef DEBUG
  Serial.println(str);
#endif
}

void MqttBase::debug_println(int i) {
#ifdef DEBUG
  Serial.println(i);
#endif
}