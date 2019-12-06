#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "MAX7221.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define MAX7221_CS 5
#define MAX7219_CS 12

#define POTI_0 15
#define POTI_1 2
#define POTI_2 0
#define POTI_3 4

#define PIEZO_0 13

#define BUTTON_0 16
#define BUTTON_1 17

#define LEDC_CHANNEL1 0
#define LEDC_RESOLUTION 8

#define REWIRE_0_1 25
#define REWIRE_0_2 33
#define REWIRE_0_3 32
#define REWIRE_0_4 35
#define REWIRE_0_5 34

#define LOCK_0 14

#define LED_RING 27

enum tPuzzleState { INIT, SOLVED, NOT_SOLVED };

tPuzzleState puzzleStateRewiring0;
tPuzzleState puzzleStatePotis;

MAX7221 seg1 = MAX7221(MAX7221_CS, 1, MAX7221::SEGMENT);
MAX7221 ledm1 = MAX7221(MAX7219_CS, 4, MAX7221::LEDMATRIX);


Adafruit_NeoPixel np(16, LED_RING, NEO_GRB + NEO_KHZ800);

void TaskPotentiometerReadout(void *pvParameters);
void TaskWiringReadout(void *pvParameters);
void TaskPiezoButtonReadout(void *pvParameters);
void TaskControlPuzzleState(void *pvParameters);
void TaskRefreshLedMatrix(void *pvParameters);

void setup() {

  // TODO: Clean Setup
  // TODO: Implement MQTT Connection

  puzzleStateRewiring0 = INIT;
  puzzleStatePotis = INIT;

  Serial.begin(115200);
  Serial.println("Setup started ...");

  pinMode(BUTTON_0, INPUT);
  pinMode(BUTTON_1, INPUT);

  // Init PWM Signals for Buzzers
  ledcSetup(LEDC_CHANNEL1, 2000, LEDC_RESOLUTION);
  ledcAttachPin(PIEZO_0, LEDC_CHANNEL1);

  // Init MAX7221 on SPI Bus
  seg1.initMAX();

  ledm1.initMAX();
  ledm1.clear();
  ledm1.commit();

  // Init LED ring
  np.begin();

  Serial.println("Setup finished");

  // Attach Tasks
  xTaskCreatePinnedToCore(TaskControlPuzzleState, "TaskControlPuzzleState",
                          2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskPotentiometerReadout, "TaskPotentiometerReadout",
                          2048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskWiringReadout, "TaskWiringReadout", 2048, NULL, 3,
                          NULL, 0);
  xTaskCreatePinnedToCore(TaskPiezoButtonReadout, "TaskPiezoButtonReadout",
                          2048, NULL, 4, NULL, 1);
  xTaskCreatePinnedToCore(TaskRefreshLedMatrix, "TaskRefreshLedMatrix", 2048,
                          NULL, 5, NULL, 0);
}

void loop() {
  // no code here!
  vTaskDelete(NULL);
}

void TaskPotentiometerReadout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    Serial.print("TaskPotentiometerReadout: ");
    uint16_t pValues[4];
    uint8_t potis[] = {POTI_0, POTI_1, POTI_2, POTI_3};
    for (int i = 0; i <= 3; i++) {

      uint16_t value;
      value = analogRead(potis[i]);
      value = map(value, 0, 4095, 0, 9);
      pValues[i] = value;

      // Serial.print("Poti ");
      // Serial.print(i);
      // Serial.print(": ");
      // Serial.println(pValues[i]);
    }
    seg1.transferData(0x01, pValues[0]);
    seg1.transferData(0x02, pValues[1]);
    seg1.transferData(0x03, pValues[2]);
    seg1.transferData(0x04, pValues[3]);

    if (pValues[0] == 1 && pValues[1] == 2 && pValues[2] == 3 &&
        pValues[3] == 4) {
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
  (void)pvParameters;
  for (;;) {
    Serial.print("TaskWiringReadout: ");
    uint16_t rewiringValues0[5];
    uint16_t rewiringPins0[] = {REWIRE_0_1, REWIRE_0_2, REWIRE_0_3, REWIRE_0_4,
                                REWIRE_0_5};
    for (int i = 0; i <= 4; i++) {
      rewiringValues0[i] = analogRead(rewiringPins0[i]);
      rewiringValues0[i] = map(rewiringValues0[i], 0, 4095, 0, 33);
      // Serial.print("Rewiring 0_");
      // Serial.print(i);
      // Serial.print(": ");
      // Serial.println(rewiringValues0[i]);
    }

    if ((rewiringValues0[0] >= 3 || rewiringValues0[0] <= 7) &&
        (rewiringValues0[1] >= 9 || rewiringValues0[1] <= 13) &&
        (rewiringValues0[2] >= 16 || rewiringValues0[2] <= 20) &&
        (rewiringValues0[3] >= 23 || rewiringValues0[3] <= 27) &&
        (rewiringValues0[4] >= 31 || rewiringValues0[4] <= 35)) {
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
  (void)pvParameters;
  for (;;) {
    Serial.print("TaskControlPuzzleState: ");
    if (puzzleStatePotis == SOLVED && puzzleStateRewiring0 == SOLVED) {
      Serial.println("All Puzzles Solved");
      ledm1.setColumn(0, 0b11100000);
      ledm1.setColumn(1, 0b10100000);
      ledm1.setColumn(2, 0b11100000);
      ledm1.commit();
    } else {
      Serial.println("Not all Puzzles Solved!");
      ledm1.setColumn(0, 0b10100000);
      ledm1.setColumn(1, 0b01000000);
      ledm1.setColumn(2, 0b10100000);
      ledm1.commit();
    }

    vTaskDelay(300);
  }
}

void TaskRefreshLedMatrix(void *pvParameters) {
  // define 10s delay
  for (;;) {
    Serial.print("TaskRefreshLedMatrix: ");
    ledm1.clear();
    ledm1.commit();
    Serial.println("Display cleared!");
    vTaskDelay(10000);
  }
}

void TaskPiezoButtonReadout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    Serial.print("TaskPiezoButtonReadout: ");
    // TODO: Debounce Buttons
    uint16_t buttonState1 = digitalRead(BUTTON_0);
    uint16_t buttonState2 = digitalRead(BUTTON_1);
    if (buttonState1) {
      Serial.println("Button 1 pressed");
      float frequencies[] = {130.81, 164.81, 196.0, 246.94,
                             261.63, 246.94, 196.0, 164.81};
      for (int i = 0; i <= 7; i++) {
        ledcWriteTone(LEDC_CHANNEL1, frequencies[i]);
        vTaskDelay(250);
      }
    } else if (buttonState2) {
      Serial.println("Button 2 pressed");
      // e 164.81
      // d# 155.56
      float frequencies[] = {164.81, 155.56, 164.81, 155.56,
                             123.47, 146.83, 130.81, 110};
      for (int i = 0; i <= 7; i++) {
        ledcWriteTone(LEDC_CHANNEL1, frequencies[i]);
        vTaskDelay(250);
      }
    } else {
      Serial.println("No Button pressed");
      ledcWriteTone(LEDC_CHANNEL1, 0);
    }
    vTaskDelay(500);
  }
}