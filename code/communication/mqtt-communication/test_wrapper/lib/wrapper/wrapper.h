#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
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

class mqtt_wrapper {
 private:
  const char* mqtt_topic;
  const char* mqtt_server = "172.20.10.9";
  StaticJsonDocument<300> doc;
  JsonObject JSONencoder = doc.to<JsonObject>();
  PubSubClient* client;

 public:
  mqtt_wrapper();
  ~mqtt_wrapper();

  void init(const char* ssid, const char* password, const char* topic);
  virtual void callback(char* topic, byte* message, unsigned int length);
  void loop() { client->loop(); }
};

mqtt_wrapper::mqtt_wrapper() {}

mqtt_wrapper::~mqtt_wrapper() {}

void mqtt_wrapper::init(const char* ssid, const char* password, const char* topic) {
  mqtt_topic = topic;
  WiFiClient espClient;
  client = new PubSubClient(espClient);
  setup_wifi(ssid, password);
  client->setServer(mqtt_server, 1883);
  // client->setCallback(callback);
  client->setCallback([this](char* topic, byte* payload, unsigned int length) {
    this->callback(topic, payload, length);
  });
  client->subscribe(mqtt_topic);
}

void mqtt_wrapper::callback(char* topic, byte* message, unsigned int length) {
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
  /*
    if(strcmp(state, "<ON>") == 0){
      mqtt_publish("Status","active");
    }
    if(strcmp(state, "<OFF>") == 0){
      mqtt_publish("Status","inactive");
    }
    */
}
