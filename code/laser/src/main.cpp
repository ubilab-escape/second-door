#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include "wifi_secure.h"

#include "MqttBase.h"

const char* mqtt_topic = "7/laser";
const char* mqtt_server = "10.0.0.2";

#define LASER_PIN 21
#define SEQUENDE_SIZE 5

// setting PWM properties
const int freq = 50;
const int ledChannel = 0;
const int resolution = 10;  // Resolution 8, 10, 12, 15
const int duty_50 = 512;

// RGB Ring laser
#define RGB_RING_PIN 18
#define NUM_PIXEL 13
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXEL, RGB_RING_PIN, NEO_GRB + NEO_KHZ800);

MqttBase* mqtt_com;

void callback(const char* method1, const char* state, int daten) {
  Serial.println("LOGIC CALLBACK");
  if (strcmp(method1, "TRIGGER") == 0) {
    Serial.println("HUI");
    if (strcmp(state, "ON") == 0) {
      // configure LED PWM functionalitites
      ledcSetup(ledChannel, freq, resolution);

      // attach the channel to the GPIO2 to be controlled
      ledcAttachPin(LASER_PIN, ledChannel);
      ledcWrite(ledChannel, 512);

      pixels.begin();
      pixels.setBrightness(255);  // die Helligkeit setzen 0 dunke -> 255 ganz hell
      pixels.show();              // Alle NeoPixel sind im Status "aus".

      for (int i = 0; i < NUM_PIXEL; i++) {
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(0, 255, 0));  // Moderately bright red color.
        pixels.show();  // This sends the updated pixel color to the hardware.
      }
    }
    if (strcmp(state, "OFF") == 0) {
      pinMode(LASER_PIN, OUTPUT);

      for (int i = 0; i < NUM_PIXEL; i++) {
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));  // Moderately bright red color.
        pixels.show();  // This sends the updated pixel color to the hardware.
      }
    }
  }
}

// Setup
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  mqtt_com = new MqttBase("10.0.0.2", 1883);
  mqtt_com->init(ssid, password, "7/laser", callback);
  // init_com(ssid, password, mqtt_server, callback);
}

// Loop
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  mqtt_com->loop();
  delay(2000);
  //mqtt_com->publish("STATUS", "solved");
}