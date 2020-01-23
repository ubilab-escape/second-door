#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>

/* Example code for the Adafruit TCS34725 breakout library */

/* Connect SCL to analog 5
Connect SDA to analog 4
Connect VDD to 3.3V DC
Connect GROUND to common ground */

//Defines
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//RGB Ring
#define PIN       18
#define NUM_PIXEL 16

//RGB Sensor
#define RGB_R_VALUE_THRESH 2000
#define RGB_HITS_THRESH 20

//Globals
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXEL, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_TCS34725 rgb_sensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_1X);

int RGB_hits = 0;//counts the hits of RGB color sensor

int old_time_in_ms = millis();

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


// integration time defines 
//TCS34725_INTEGRATIONTIME_2_4MS  = 0xFF   #  2.4ms - 1 cycle    - Max Count: 1024
//TCS34725_INTEGRATIONTIME_24MS   = 0xF6   # 24ms  - 10 cycles  - Max Count: 10240
//TCS34725_INTEGRATIONTIME_50MS   = 0xEB   #  50ms  - 20 cycles  - Max Count: 20480
//TCS34725_INTEGRATIONTIME_101MS  = 0xD5   #  101ms - 42 cycles  - Max Count: 43008
//TCS34725_INTEGRATIONTIME_154MS  = 0xC0   #  154ms - 64 cycles  - Max Count: 65535
//TCS34725_INTEGRATIONTIME_700MS  = 0x00   #  700ms - 256 cycles - Max Count: 65535





void setup(void) {
  Serial.begin(9600);

  if (rgb_sensor.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  
  // set led ring to red
  pixels.begin();
  pixels.setBrightness(50); //die Helligkeit setzen 0 dunke -> 255 ganz hell
  pixels.show(); // Alle NeoPixel sind im Status "aus".
  
  for(int i=0;i<NUM_PIXEL;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(255,0,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void loop(void) {

  uint16_t r, g, b, c, colorTemp, lux;

  rgb_sensor.getRawData(&r, &g, &b, &c);
  colorTemp = rgb_sensor.calculateColorTemperature(r, g, b);
  lux = rgb_sensor.calculateLux(r, g, b);

  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");

  if (r >= RGB_R_VALUE_THRESH){
    pixels.setPixelColor(RGB_hits, pixels.Color(0,255,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    RGB_hits++;
    old_time_in_ms = millis();   
  }
  int time_in_ms = millis() - old_time_in_ms;

  if (time_in_ms >= 1000){
    if (RGB_hits > 0){
      RGB_hits--;
    }
    pixels.setPixelColor(RGB_hits, pixels.Color(255,0,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    old_time_in_ms = millis();
    }
}
