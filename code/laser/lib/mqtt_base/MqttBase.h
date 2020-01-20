#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define DEBUG

class MqttBase {
 private:
  const char* mqtt_topic_;
  const char* mqtt_server_;
  int mqtt_port_ = 1883;
  StaticJsonDocument<300> doc_;
  JsonObject JSONencoder_ = doc_.to<JsonObject>();
  // std::unique_ptr<PubSubClient> client;
  PubSubClient* pub_client_;
  WiFiClient* wifi_client_;

  std::function<void(const char*, const char*, int)> logic_callback_;

  void debug_print(const char* str);
  void debug_print(int i);
  void debug_println(const char* str);
  void debug_println(int i);

 public:
  MqttBase(const char* mqtt_server, uint16_t mqtt_port);
  ~MqttBase();

  void init(const char* ssid, const char* password, const char* topic,
            void (*f)(const char*, const char*, int));
  void reconnect();
  virtual void callback(char* topic, byte* message, unsigned int length);
  virtual void publish(const char* methode, const char* state);
  void loop();
  
};