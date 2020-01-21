/*
 * uMQTTBroker demo for Arduino
 * 
 * Minimal Demo: the program simply starts a broker and waits for any client to connect.
 */

#include <ESP8266WiFi.h>
#include "uMQTTBroker.h"
#include <ArduinoJson.h>
#include "wifi_secure.h"

uMQTTBroker myBroker;

//Json
StaticJsonDocument<300> doc;
JsonObject JSONencoder = doc.to<JsonObject>();

/*
 * WiFi init stuff
 */
void startWiFiClient()
{
  Serial.println("Connecting to "+(String)ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void startWiFiAP()
{
  WiFi.softAP(ssid, password);
  Serial.println("AP started");
  Serial.println("IP address: " + WiFi.softAPIP().toString());
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // Connect to a WiFi network
  startWiFiClient();

  // Or start the ESP as AP
//startWiFiAP();

  // Start the broker
  Serial.println("Starting MQTT broker");
  myBroker.init();

  JSONencoder["method"] = "TRIGGER";
  JSONencoder["state"] = "on";
  JSONencoder["data"] = "2";
}

void loop()
{   
  // do anything here
  char JSONmessageBuffer[100];
  //JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  serializeJson(doc,JSONmessageBuffer, 100);
  Serial.print("send message");
  Serial.println(JSONencoder);
  myBroker.publish("7/fusebox", JSONmessageBuffer);
  delay(5000);
}
