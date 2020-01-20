
#include <Arduino.h>
#include <wrapper.h>
#include "wifi_secure.h"

//const char* ssid = "Thomass iPhone";
//const char* pw = "12345670";

mqtt_wrapper wrapper;


void setup() {
  Serial.begin(115200);
  wrapper.init(ssid, password, "7/laser");
}

void loop() {
  wrapper.loop();
  wrapper.publish("TRIGGER", "ON");
  delay(1000);

  // put your main code here, to run repeatedly:
}




/*

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

const char* ssid = "Thomass iPhone";
const char* password = "12345670";

const char* Topics[4] = {"7/fusebox", "7/robot", "7/laser", "7/buttonServer"};
#define topicNumber 0  // chose which topic should used
const char* mqtt_topic = Topics[topicNumber];

// Json Buffer

StaticJsonDocument<300> rxdoc;
// JsonObject JSONencoder = rxdoc.to<JsonObject>();

// Add your MQTT Broker IP address, example:
// const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "10.0.0.2";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void setup_wifi() {
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
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  deserializeJson(rxdoc, message);
  const char* method1 = rxdoc["method"];
  const char* state = rxdoc["state"];
  int daten = rxdoc["data"];
  Serial.print("Methode: ");
  Serial.println(method1);
  Serial.print("State: ");
  Serial.println(state);
  Serial.print("Daten: ");
  Serial.println(daten);

  // String str_topic = String(topic);
  // str_topic.remove(0,2);
  // Serial.println(str_topic);

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or
  // "off". Changes the output state according to the message
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}


*/