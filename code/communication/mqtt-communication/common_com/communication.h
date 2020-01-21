#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>



const char* mqtt_topic = "7/robot";
const char* mqtt_server = "10.0.0.2";

StaticJsonDocument<300> doc;
JsonObject JSONencoder = doc.to<JsonObject>();
WiFiClient espClient;
PubSubClient client(espClient);

//Functions
void init_com(const char* ssid, const char* password, const char* ip_mqtt_server, void *callback());
void comm_main();
void mqtt_subscribe(const char* topic);
void mqtt_publish(const char* methode, const char* state);

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
  
  deserializeJson(doc, message);
  const char* method1 = doc["method"];
  const char* state = doc["state"];
  int daten = doc["data"];
  
  Serial.print("Methode: "); Serial.println(method1);
  Serial.print("State: "); Serial.println(state);
  Serial.print("Daten: "); Serial.println(daten);

  if(strcmp(state, "<ON>") == 0){
    mqtt_publish("Status","active");
  }
  if(strcmp(state, "<OFF>") == 0){
    mqtt_publish("Status","inactive");
  }
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
//////////////////////////////////////////////////////////////////////////////////////  
void init_com(const char* ssid, const char* password, const char* ip_mqtt_server, void* callback){
  setup_wifi(ssid, password);
  client.setServer(ip_mqtt_server, 1883);
  client.setCallback(callback);
}

///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////  
void mqtt_subscribe(const char* topic){
  client.subscribe(topic);
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////  
void mqtt_publish(const char* methode, const char* state){

  JSONencoder["method"] = methode;
  JSONencoder["state"] = state;
  JSONencoder["data"] = 0;

  char JSONmessageBuffer[100];

  serializeJson(doc,JSONmessageBuffer, 100);
  Serial.print("send message");
  Serial.println(JSONencoder);
  client.publish(mqtt_topic, JSONmessageBuffer);

  
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////  
void comm_main(){
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
}
