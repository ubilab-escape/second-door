
#include "MAX7221.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define MAX7221_CS 14

#define POTI_0 15
#define POTI_1 4
#define POTI_2 27
#define POTI_3 26

#define PIEZO_0 13

#define BUTTON_0 16
#define BUTTON_1 17

#define LED_0 2

#define LEDC_CHANNEL1 0
#define LEDC_RESOLUTION 8

#define REWIRE_0_1 25
#define REWIRE_0_2 33
#define REWIRE_0_3 32
#define REWIRE_0_4 35
#define REWIRE_0_5 34

typedef enum tPuzzleState {INIT, SOLVED, NOT_SOLVED};

tPuzzleState puzzleStateRewiring0;
tPuzzleState puzzleStatePotis;


MAX7221 max1 = MAX7221(MAX7221_CS, 4);


uint16_t evaluatePotentiometer(uint8_t p_poti);
void setDisplay(uint16_t* p_displayData);

void TaskPotentiometerReadout(void *pvParameters);
void TaskWiringReadout(void *pvParameters);
void TaskPiezoButtonReadout(void *pvParameters);

void TaskControlPuzzleState(void *pvParameters);

void setup() {

  // TODO: Clean Setup
  // TODO: Implement MQTT Connection

  puzzleStateRewiring0 = INIT;
  puzzleStatePotis = INIT;

  Serial.begin(115200);
  Serial.println("Setup started");
  
  
  pinMode(BUTTON_0, INPUT);
  pinMode(BUTTON_1, INPUT);

  pinMode(LED_0, OUTPUT);

  // Init PWM Signals for Buzzers
  ledcSetup(LEDC_CHANNEL1, 2000, LEDC_RESOLUTION);
  ledcAttachPin(PIEZO_0, LEDC_CHANNEL1);

  // Init MAX7221 on SPI Bus
  max1.initMAX();

  Serial.println("Setup finished");

  // Attach Tasks
  xTaskCreatePinnedToCore(TaskControlPuzzleState, "TaskControlPuzzleState", 1024, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskPotentiometerReadout, "TaskPotentiometerReadout", 1024, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(TaskWiringReadout, "TaskWiringReadout", 1024, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
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
      Serial.print("Poti ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(pValues[i]);
    }
    max1.transferData(0x01, pValues[2]);
    max1.transferData(0x02, pValues[3]);

    if (pValues[0] == 4 && pValues[1] == 3 && pValues[2] == 2 && pValues[3] == 1) {
      Serial.println("Potis Solved!"); 
      puzzleStatePotis = SOLVED;
    } else {
      Serial.println("Potis Not solved!");
      puzzleStatePotis = NOT_SOLVED;
    }
    
    vTaskDelay(500);
  }
}

void TaskWiringReadout(void *pvParameters) {
  // TODO: Implement Wiring Puzzle
  (void) pvParameters;
  for(;;) {
    uint16_t rewiringValues0[5];
    uint16_t rewiringPins0[] = {REWIRE_0_1, REWIRE_0_2, REWIRE_0_3, REWIRE_0_4, REWIRE_0_5};
    for (int i = 0; i <=4; i++) {
      rewiringValues0[i] = analogRead(rewiringPins0[i]);
      rewiringValues0[i] = map(rewiringValues0[i], 0, 4095, 0, 33);
      // Serial.print("Pin " + String(i) + " : ");
      Serial.print("Rewiring 0_");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(rewiringValues0[i]);
    }

    if (rewiringValues0[0] == 25 && rewiringValues0[1] == 18 && rewiringValues0[2] == 11 && rewiringValues0[3] == 33 && rewiringValues0[4] == 5) {
      Serial.println("Rewiring 0 solved!");
      puzzleStateRewiring0 = SOLVED;
    } else {
      Serial.println("Rewiring 0 not solved");
      puzzleStateRewiring0 = NOT_SOLVED;
    }

    vTaskDelay(500);
  }
}

void TaskControlPuzzleState(void *pvParameters) {
  (void) pvParameters;
  for(;;) {
    Serial.println("hey");
    if (puzzleStatePotis == SOLVED && puzzleStateRewiring0 == SOLVED) {
      Serial.println("All Puzzles Solved");
      digitalWrite(LED_0, HIGH);
    } else {
      digitalWrite(LED_0, LOW);
    }
    vTaskDelay(500);
  }
}

void TaskPiezoButtonReadout(void *pvParameters) {
  (void) pvParameters;
  for(;;) {
    // TODO: Debounce Buttons
    uint16_t buttonState1 = digitalRead(BUTTON_0);
    if(buttonState1) {
      float frequencies[] = {130.81, 164.81, 196.0, 246.94, 261.63, 246.94, 196.0, 164.81};
      for (int i = 0; i <= 7; i++) {
        ledcWriteTone(LEDC_CHANNEL1, frequencies[i]);
        vTaskDelay(250);
      } 
    } else {
      ledcWriteTone(LEDC_CHANNEL1, 0);
    }
    uint16_t buttonState2 = digitalRead(BUTTON_1);
    if(buttonState2) {
      // e 164.81
      // d# 155.56
      float frequencies[] = {164.81, 155.56, 164.81, 155.56, 123.47, 146.83, 130.81, 110};
      for (int i = 0; i <= 7; i++) {
        ledcWriteTone(LEDC_CHANNEL1, frequencies[i]);
        vTaskDelay(250);
      } 
    } else {
      ledcWriteTone(LEDC_CHANNEL1, 0);
    }
  }
}

uint16_t evaluatePotentiometer(uint8_t p_poti, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
  uint16_t value;
  value = analogRead(p_poti);
  value = map(value, in_min, in_max, out_min, out_max);
  return value;
}
