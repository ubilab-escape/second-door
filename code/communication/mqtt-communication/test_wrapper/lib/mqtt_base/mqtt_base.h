#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

class mqtt_base {
 private:
  const char* mqtt_topic;
  const char* mqtt_server = "10.0.0.2";
  StaticJsonDocument<300> doc;
  JsonObject JSONencoder = doc.to<JsonObject>();
  // std::unique_ptr<PubSubClient> client;
  PubSubClient* client;
  WiFiClient* espClient;

 public:
  mqtt_base();
  ~mqtt_base();

  void init(const char* ssid, const char* password, const char* topic);
  void reconnect();
  virtual void callback(char* topic, byte* message, unsigned int length);
  virtual void publish(const char* methode, const char* state);
  void loop();
};