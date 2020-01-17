

#include <Adafruit_NeoPixel.h>

#define MAX_SEQUENZES 32

#define LASER_PIN          21 
#define SEQUENDE_SIZE       5

//RGB Ring
#define RGB_RING_PIN       18
#define NUM_PIXEL          16

// setting PWM properties
const int freq = 50;
const int ledChannel = 0;
const int resolution = 10; //Resolution 8, 10, 12, 15
const int duty_50 = 512;

//Laser dedector
int detectorPin = 19;

//Globals
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXEL, RGB_RING_PIN, NEO_GRB + NEO_KHZ800);

uint8_t sequence[6];

uint8_t numberOfSequences = 0;

int old_time_in_ms = millis();

bool puzzle_solved = false;

//Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t  analyse_sequence(uint8_t sequence[6], uint8_t target){
  uint8_t target_number = 0;
  uint8_t len = sizeof(sequence);
  for (int i=0; i<len; i++){
    if (sequence[i] == target){
      target_number++;
    }
  }
  return target_number + 1;
}

void blink_ring(uint8_t blinking_number, uint8_t frequency){
  uint16_t period = 1 * 1000 / frequency;
  for (uint8_t k=0; k < blinking_number; k++){
    for(int i=0;i<NUM_PIXEL;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    }
    delay(period/2);
    for(int i=0;i<NUM_PIXEL;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,255,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    }
    delay(period/2);
    }
  }


//Setup
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  //Beginn der seriellen Kommunikation
  Serial.begin(9600);

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
    pixels.setPixelColor(i, pixels.Color(255,0,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
  
  pinMode(detectorPin, INPUT);  //Laser Detector als Eingangssignal setzen

}
//Loop
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  
  static uint8_t byte_count = 0;
  
  sequence[byte_count] = digitalRead(detectorPin);
  
  if (byte_count == SEQUENDE_SIZE){
    byte_count = 0;
    uint8_t zeros = analyse_sequence(sequence, 0); 
    uint8_t ones = analyse_sequence(sequence, 1);
    
    //chekc if sequenz is correct
    if (zeros == ones && !puzzle_solved){
      Serial.print("Sequence dedected ");
      Serial.print(zeros);Serial.print(" ");Serial.println(ones);
      numberOfSequences++;
      old_time_in_ms = millis();
      }
     }
     
    // check if new additional LED shoulb set to green
    if ((numberOfSequences % 2 == 0) && (numberOfSequences > 0)){
          uint8_t RGB_led = (uint8_t)numberOfSequences/2;
          pixels.setPixelColor(RGB_led, pixels.Color(0,255,0)); // Moderately bright green color.
          pixels.show(); // This sends the updated pixel color to the hardware.
      }
      
    //check if puzzle is solved
    if (numberOfSequences > MAX_SEQUENZES){
      puzzle_solved = true;
      Serial.println("Fuse Box open");
      numberOfSequences = 0;
      blink_ring(5, 2);
    }
    
    
    int time_in_ms = millis() - old_time_in_ms;

    if (time_in_ms >= 500){
      if (numberOfSequences > 0){
        if(numberOfSequences % 2 == 0){
          uint8_t RGB_led = (uint8_t)numberOfSequences/2;
          pixels.setPixelColor(RGB_led, pixels.Color(255,0,0)); // set led to red
          pixels.show(); // This sends the updated pixel color to the hardware.     
          }         
        numberOfSequences--;   // count down sequence if max time was reached  
    }
    old_time_in_ms = millis();    
  }
byte_count++;
}
