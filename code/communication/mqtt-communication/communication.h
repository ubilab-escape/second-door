#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_secure.h"


const char* mqtt_topic = "7/robot";

StaticJsonDocument<300> rxdoc;

const char* mqtt_server = "10.0.0.2";

WiFiClient espClient;
PubSubClient client(espClient);

//Functions
void scan();
void init_com();
void comm_main();
String translateEncryptionType(wifi_auth_mode_t encryptionType);

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void scan(){
  Serial.println("Start scan");
  int n = WiFi.scanNetworks();
 
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(") ");
      Serial.print(" [");
      Serial.print(WiFi.channel(i));
      Serial.print("] ");
      String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
      Serial.println(encryptionTypeDescription);
      delay(10);
    }
  }
  Serial.println("Scan done");
  Serial.println("");
  
  }

String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case (0):
      return "Open";
    case (1):
      return "WEP";
    case (2):
      return "WPA_PSK";
    case (3):
      return "WPA2_PSK";
    case (4):
      return "WPA_WPA2_PSK";
    case (5):
      return "WPA2_ENTERPRISE";
    default:
      return "UNKOWN";
    }
  }


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
  Serial.print("Methode: "); Serial.println(method1);
  Serial.print("State: "); Serial.println(state);
  Serial.print("Daten: "); Serial.println(daten);
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
//////////////////////////////////////////////////////////////////////////////  
void init_com(){
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////  
void comm_main(){
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
}
