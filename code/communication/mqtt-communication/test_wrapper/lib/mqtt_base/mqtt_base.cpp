#include "mqtt_base.h"

void setup_wifi(const char* ssid, const char* password) {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


mqtt_base::mqtt_base() {


}

mqtt_base::~mqtt_base() {}

void mqtt_base::init(const char* ssid, const char* password, const char* topic) {
  mqtt_topic = topic;

  setup_wifi(ssid, password);
  
  client = new PubSubClient();
  espClient = new WiFiClient();
  client->setClient(* espClient);
  client->setServer(mqtt_server, 1883);
  client->setCallback([this](char* topic, byte* payload, unsigned int length) {
    this->callback(topic, payload, length);
  });
  client->subscribe(mqtt_topic);
  // reconnect();
  
}

void mqtt_base::reconnect() {
  
  
  while (!client->connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client->connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client->subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client->state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_base::loop() {
  if (!client->connected()) {
    reconnect();
  }
  client->loop();
}

void mqtt_base::callback(char* topic, byte* message, unsigned int length) {
  /*
  Serial.print("Message arrived on topic: ");
  Serial.print(mqtt_topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  deserializeJson(doc, message);
  const char* method1 = doc["method"];
  const char* state = doc["state"];
  int daten = doc["data"];

  Serial.print("Methode: ");
  Serial.println(method1);
  Serial.print("State: ");
  Serial.println(state);
  Serial.print("Daten: ");
  Serial.println(daten);
  */
}

void mqtt_base::publish(const char* methode, const char* state) {
  JSONencoder["method"] = methode;
  JSONencoder["state"] = state;
  JSONencoder["data"] = 0;

  char JSONmessageBuffer[100];

  serializeJson(doc, JSONmessageBuffer, 100);
  Serial.print("send message");
  Serial.println(JSONencoder);
  client->publish(mqtt_topic, JSONmessageBuffer);
}
