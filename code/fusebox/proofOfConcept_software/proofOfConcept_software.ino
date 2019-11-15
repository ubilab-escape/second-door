#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif


#include <TM1637Display.h>

#define SEG_CLK 27
#define SEG_DIO 26

#define POTI_0 15
#define POTI_1 2
#define POTI_2 0
#define POTI_3 4

#define POTI_4 34

#define PIEZO_0 13

#define BUTTON_0 16
#define BUTTON_1 17

#define LEDC_CHANNEL 0
#define LEDC_RESOLUTION 8

TM1637Display display(SEG_CLK, SEG_DIO);

uint16_t evaluatePotentiometer(uint8_t p_poti);
void setDisplay(uint16_t* p_displayData);

void TaskPotentiometerReadout(void *pvParameters);
void TaskWiringReadout(void *pvParameters);
void TaskPiezoButtonReadout(void *pvParameters);


void setup() {

  // TODO: Clean Setup
  // TODO: Implement MQTT Connection

  Serial.begin(115200);
  Serial.println("Setup started");
  
  display.setBrightness(0x0a);
  
  Serial.println("Setup finished");

  pinMode(BUTTON_0, INPUT);
  pinMode(BUTTON_1, INPUT);

  ledcSetup(LEDC_CHANNEL, 2000, LEDC_RESOLUTION);
  ledcAttachPin(PIEZO_0, LEDC_CHANNEL);

  xTaskCreatePinnedToCore(TaskPotentiometerReadout, "TaskPotentiometerReadout", 1024, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  //xTaskCreatePinnedToCore(TaskWiringReadout, "TaskWiringReadout", 1024, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(TaskPiezoButtonReadout, "TaskPiezoButtonReadout", 1024, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
}

void loop() {
  // no code here!
  
}

void TaskPotentiometerReadout(void *pvParameters) {
  (void) pvParameters;
  for(;;) {
    uint16_t pValues[4];
    uint8_t potis[] = {POTI_0, POTI_1, POTI_2, POTI_3};
    for (int i = 0; i <= 3; i++) {
      pValues[i] = evaluatePotentiometer(potis[i], 0, 4095, 0, 9);
    }
    setDisplay(pValues);
    vTaskDelay(100);
  }
}

void TaskWiringReadout(void *pvParameters) {
  // TODO: Implement Wiring Puzzle
  (void) pvParameters;
  for(;;) {

  }
}

void TaskPiezoButtonReadout(void *pvParameters) {
  (void) pvParameters;
  for(;;) {
    // TODO: Debounce Buttons
    uint16_t buttonState1 = digitalRead(BUTTON_0);
    if(buttonState1) {
      uint16_t frequency = evaluatePotentiometer(POTI_4, 0, 4095, 100, 100000);
      ledcWriteTone(LEDC_CHANNEL, frequency);
      vTaskDelay(250);
    } else {
      ledcWriteTone(LEDC_CHANNEL, 0);
    }
    uint16_t buttonState2 = digitalRead(BUTTON_1);
    if(buttonState2) {
      float frequencies[] = {130.81, 164.81, 196.0, 246.94, 261.63, 246.94, 196.0, 164.81};
      for (int i = 0; i <= 7; i++) {
        ledcWriteTone(LEDC_CHANNEL, frequencies[i]);
        vTaskDelay(250);
      } 
    } else {
      ledcWriteTone(LEDC_CHANNEL, 0);
    }
  }
}

void setDisplay(uint16_t* p_displayData) {
  static uint8_t data[4];
  for (int i = 0; i < 4; i++) {
    data[i] = display.encodeDigit(p_displayData[i]);
  }
  display.setSegments(data);
}

uint16_t evaluatePotentiometer(uint8_t p_poti, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
  uint16_t value;
  value = analogRead(p_poti);
  value = map(value, in_min, in_max, out_min, out_max);
  return value;
}
