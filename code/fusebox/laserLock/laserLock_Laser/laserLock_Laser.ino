
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#define LASER_PIN          21 
#define SEQUENDE_SIZE       5

//WIFI
const char* ssid     = "Thomass iPhone";
const char* password = "12345670";

//MQTT
const char* Topics[4] = {"7/fusebox", "7/robot", "7/laser", "7/buttonServer"};
#define topicNumber 2 //chose which topic should used
const char* mqtt_topic = Topics[topicNumber];


StaticJsonDocument<300> rxdoc;

// setting PWM properties
const int freq = 50;
const int ledChannel = 0;
const int resolution = 10; //Resolution 8, 10, 12, 15
const int duty_50 = 512;

//RGB Ring laser
#define RGB_RING_PIN       18
#define NUM_PIXEL          13
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXEL, RGB_RING_PIN, NEO_GRB + NEO_KHZ800);

//Globals
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


const char* mqtt_server = "172.20.10.9";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


//Setup
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  //setup_wifi();
  //client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);

    // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO2 to be controlled
  ledcAttachPin(LASER_PIN, ledChannel);
  ledcWrite(ledChannel,512);

   // set led ring to red
  pixels.begin();
  pixels.setBrightness(255); //die Helligkeit setzen 0 dunke -> 255 ganz hell
  pixels.show(); // Alle NeoPixel sind im Status "aus".
  
  for(int i=0;i<NUM_PIXEL;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,255,0)); // Moderately bright red color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}


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
  Serial.print("Methode: "); Serial.println(method1);
  Serial.print("State: "); Serial.println(state);
  Serial.print("Daten: "); Serial.println(daten);

  if (method1 == "TIGGER"){
    if (state == "on"){


        // configure LED PWM functionalitites
        ledcSetup(ledChannel, freq, resolution);
  
        // attach the channel to the GPIO2 to be controlled
        ledcAttachPin(LASER_PIN, ledChannel);
        ledcWrite(ledChannel,512);
        
        pixels.begin();
        pixels.setBrightness(255); //die Helligkeit setzen 0 dunke -> 255 ganz hell
        pixels.show(); // Alle NeoPixel sind im Status "aus".
    
        for(int i=0;i<NUM_PIXEL;i++){
          // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
          pixels.setPixelColor(i, pixels.Color(0,255,0)); // Moderately bright red color.
          pixels.show(); // This sends the updated pixel color to the hardware.
        }
        
      }
      if (state == "off"){


          pinMode(LASER_PIN, OUTPUT); 
 
         for(int i=0;i<NUM_PIXEL;i++){
          // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
          pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright red color.
          pixels.show(); // This sends the updated pixel color to the hardware.
        }
        
      }
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
//Loop
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
    //if (!client.connected()) {
    //reconnect();
    //}
    //client.loop();

}
