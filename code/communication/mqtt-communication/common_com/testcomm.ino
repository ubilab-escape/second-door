#include "communication.h"
#include "wifi_secure.h"

void test_callback(char* topic, byte* message, unsigned int length){
  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  
  
  }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //scan();
  init_com("Thomas", "kdkdkdkk", "10.0.0.2", &test_callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  comm_main();

}
