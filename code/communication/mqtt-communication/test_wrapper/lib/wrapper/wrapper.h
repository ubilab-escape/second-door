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
  const char* mqtt_server = "10.0.0.2";
  StaticJsonDocument<300> doc;
  JsonObject JSONencoder = doc.to<JsonObject>();
  // std::unique_ptr<PubSubClient> client;
  PubSubClient* client;
  WiFiClient* espClient;

 public:
  mqtt_wrapper();
  ~mqtt_wrapper();

  void init(const char* ssid, const char* password, const char* topic);
  void reconnect();
  virtual void callback(char* topic, byte* message, unsigned int length);
  virtual void publish(const char* methode, const char* state);
  void loop();
};

mqtt_wrapper::mqtt_wrapper() {


}

mqtt_wrapper::~mqtt_wrapper() {}

void mqtt_wrapper::init(const char* ssid, const char* password, const char* topic) {
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

void mqtt_wrapper::reconnect() {
  
  
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

void mqtt_wrapper::loop() {
  if (!client->connected()) {
    reconnect();
  }
  client->loop();
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

void mqtt_wrapper::publish(const char* methode, const char* state) {
  JSONencoder["method"] = methode;
  JSONencoder["state"] = state;
  JSONencoder["data"] = 0;

  char JSONmessageBuffer[100];

  serializeJson(doc, JSONmessageBuffer, 100);
  Serial.print("send message");
  Serial.println(JSONencoder);
  client->publish(mqtt_topic, JSONmessageBuffer);
}
