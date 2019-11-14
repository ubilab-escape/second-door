#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif


#include <TM1637Display.h>

#define SEG_CLK 27
#define SEG_DIO 26
#define POTI_0 25

TM1637Display display(SEG_CLK, SEG_DIO);

void setup() {
  // put your setup code here, to run once:
  display.setBrightness(0x0a);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
  //
  data[1] = display.encodeDigit(1);
  data[2] = display.encodeDigit(2);
  data[3] = display.encodeDigit(3);

  //display.setSegments(data);

  uint16_t val = analogRead(POTI_0);
  Serial.println(val);
  val = map(val, 0, 4095, 0, 9);
  data[0] = display.encodeDigit(val);
  display.setSegments(data);

  delay(500);
  

}
