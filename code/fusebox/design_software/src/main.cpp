#include "MAX7221.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_TCS34725.h>

#include <Arduino.h>


#include <MD_MAX72xx.h>
#include <MD_MAXPanel.h>
#include "Fonts.h"

// #define DEBUG
// #define DEBUG_SERIAL

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

enum tPuzzleState { INIT, SOLVED, NOT_SOLVED };

tPuzzleState puzzleStateRewiring0;
tPuzzleState puzzleStatePotis;

MAX7221 seg1 = MAX7221(MAX7221_CS, 1, MAX7221::SEGMENT);
// MAX7221 ledm1 = MAX7221(MAX7219_CS, 8, MAX7221::LEDMATRIX);

MD_MAXPanel ledm1 = MD_MAXPanel(MD_MAX72XX::FC16_HW, MAX7219_CS, 3, 2);

TaskHandle_t xHandleLedRing;
TaskHandle_t xHandleControlPuzzleState;

#define SEQUENDE_SIZE 5
#define MAX_SEQUENZES 32

// RGB Ring
#define RGB_RING_PIN 27
#define NUM_PIXEL 16

// Laser dedector
#define detectorPin 26

Adafruit_NeoPixel pixels =
    Adafruit_NeoPixel(NUM_PIXEL, RGB_RING_PIN, NEO_GRB + NEO_KHZ800);
uint8_t sequence[6];
uint8_t numberOfSequences = 0;
int old_time_in_ms = millis();
bool puzzle_solved = false;

// Task Attachment
void TaskPotentiometerReadout(void *pvParameters);
void TaskWiringReadout(void *pvParameters);
void TaskPiezoButtonReadout(void *pvParameters);
void TaskControlPuzzleState(void *pvParameters);

void TaskLaserLock(void *pvParameters);

void blink_ring(uint8_t blinking_number, uint8_t frequency);
uint8_t analyse_sequence(uint8_t sequence[6], uint8_t target);

void setup() {

  // TODO: Clean Setup
  // TODO: Implement MQTT Connection

  puzzleStateRewiring0 = INIT;
  puzzleStatePotis = INIT;

  // disableCore0WDT();
  // disableCore1WDT();

  Serial.begin(115200);
  Serial.println("Setup started ...");

  pinMode(BUTTON_0, INPUT);
  pinMode(BUTTON_1, INPUT);

  pinMode(LOCK_0, OUTPUT);

  // Init PWM Signals for Buzzers
  ledcSetup(LEDC_CHANNEL1, 2000, LEDC_RESOLUTION);
  ledcAttachPin(PIEZO_0, LEDC_CHANNEL1);

  // Init MAX7221 on SPI Bus
  seg1.initMAX();

  ledm1.begin();
  ledm1.setFont(_Fixed_5x3);

  // ledm1.initMAX();
  // ledm1.clear();
  // ledm1.commit();

  // Init LED ring
  // set led ring to red
  pixels.begin();
  pixels.setBrightness(255); // die Helligkeit setzen 0 dunke -> 255 ganz hell
  pixels.show();             // Alle NeoPixel sind im Status "aus".

  for (int i = 0; i < NUM_PIXEL; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(
        i, pixels.Color(255, 0, 0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }

  pinMode(detectorPin, INPUT); // Laser Detector als Eingangssignal setzen
  Serial.println("Setup finished");

  // Attach Tasks
  xTaskCreatePinnedToCore(TaskControlPuzzleState, "TaskControlPuzzleState",
                          2048, NULL, 1, &xHandleControlPuzzleState, 0);

#ifndef DEBUG
  xTaskCreatePinnedToCore(TaskLaserLock, "TaskLaserLock", 2048, NULL, 2,
                          &xHandleLedRing, 1);
#else
  xTaskCreatePinnedToCore(TaskPotentiometerReadout, "TaskPotentiometerReadout",
                          2048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskWiringReadout, "TaskWiringReadout", 2048, NULL, 3,
                          NULL, 0);
  xTaskCreatePinnedToCore(TaskPiezoButtonReadout, "TaskPiezoButtonReadout",
                          2048, NULL, 2, NULL, 1);

#endif
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
#ifdef DEBUG_SERIAL
      Serial.print("Poti ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(pValues[i]);
#endif
    }
    seg1.transferData(0x01, pValues[0]);
    seg1.transferData(0x02, pValues[1]);
    seg1.transferData(0x03, pValues[2]);
    seg1.transferData(0x04, pValues[3]);

    if (pValues[0] == 1 && pValues[1] == 9 && pValues[2] == 9 &&
        pValues[3] == 5) {
      Serial.println("Potis Solved!");
      puzzleStatePotis = SOLVED;
    } else {
      Serial.println("Potis Not solved!");
      puzzleStatePotis = NOT_SOLVED;
    }

    vTaskDelay(500);
  }
}

void TaskLaserLock(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    static uint8_t byte_count = 0;

    sequence[byte_count] = digitalRead(detectorPin);

    if (byte_count == SEQUENDE_SIZE) {
      byte_count = 0;
      uint8_t zeros = analyse_sequence(sequence, 0);
      uint8_t ones = analyse_sequence(sequence, 1);

      // chekc if sequenz is correct
      if (zeros == ones && !puzzle_solved) {
        Serial.print("Sequence dedected ");
        Serial.print(zeros);
        Serial.print(" ");
        Serial.println(ones);
        numberOfSequences++;
        old_time_in_ms = millis();
        // old_time_in_ms = xTaskGetTickCount();
      }
    }

    // check if new additional LED shoulb set to green
    if ((numberOfSequences % 2 == 0) && (numberOfSequences > 0)) {
      uint8_t RGB_led = (uint8_t)numberOfSequences / 2;
      pixels.setPixelColor(
          RGB_led, pixels.Color(0, 255, 0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
    }

    // check if puzzle is solved
    if (numberOfSequences > MAX_SEQUENZES) {
      puzzle_solved = true;
      Serial.println("Fuse Box open");
      numberOfSequences = 0;
      blink_ring(5, 2);

      // Attach Tasks
      xTaskCreatePinnedToCore(TaskPotentiometerReadout,
                              "TaskPotentiometerReadout", 2048, NULL, 2, NULL,
                              1);
      xTaskCreatePinnedToCore(TaskWiringReadout, "TaskWiringReadout", 2048,
                              NULL, 3, NULL, 0);
      xTaskCreatePinnedToCore(TaskPiezoButtonReadout, "TaskPiezoButtonReadout",
                              2048, NULL, 2, NULL, 1);

      vTaskDelete(xHandleLedRing);
    }

    // int time_in_ms = xTaskGetTickCount() - old_time_in_ms;
    int time_in_ms = millis() - old_time_in_ms;

    if (time_in_ms >= 500) {
      if (numberOfSequences > 0) {
        if (numberOfSequences % 2 == 0) {
          uint8_t RGB_led = (uint8_t)numberOfSequences / 2;
          pixels.setPixelColor(RGB_led,
                               pixels.Color(255, 0, 0)); // set led to red
          pixels.show(); // This sends the updated pixel color to the hardware.
        }
        numberOfSequences--; // count down sequence if max time was reached
      }
      old_time_in_ms = millis();
      // old_time_in_ms = xTaskGetTickCount();
    }
    byte_count++;
    vTaskDelay(10);
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
#ifdef DEBUF_SERIAL
      Serial.print("Rewiring 0_");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(rewiringValues0[i]);
#endif
    }

    if ((rewiringValues0[0] >= 3 && rewiringValues0[0] <= 9) &&
        (rewiringValues0[1] >= 9 && rewiringValues0[1] <= 16) &&
        (rewiringValues0[2] >= 16 && rewiringValues0[2] <= 20) &&
        (rewiringValues0[3] >= 23 && rewiringValues0[3] <= 27) &&
        (rewiringValues0[4] >= 31 && rewiringValues0[4] <= 35)) {
      Serial.println("Rewiring 0 solved!");
      puzzleStateRewiring0 = SOLVED;
    } else {
      Serial.println("Rewiring 0 not solved");
      puzzleStateRewiring0 = NOT_SOLVED;
    }

    vTaskDelay(500);
  }
}

void blink_ring(uint8_t blinking_number, uint8_t frequency) {
  uint16_t period = 1 * 1000 / frequency;
  for (uint8_t k = 0; k < blinking_number; k++) {
    for (int i = 0; i < NUM_PIXEL; i++) {
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(
          i, pixels.Color(0, 0, 0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
    }
    delay(period / 2);
    for (int i = 0; i < NUM_PIXEL; i++) {
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(
          i, pixels.Color(0, 255, 0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
    }
    delay(period / 2);
  }
}

uint8_t analyse_sequence(uint8_t sequence[6], uint8_t target) {
  uint8_t target_number = 0;
  uint8_t len = sizeof(sequence);
  for (int i = 0; i < len; i++) {
    if (sequence[i] == target) {
      target_number++;
    }
  }
  return target_number + 1;
}

void TaskControlPuzzleState(void *pvParameters) {
  (void)pvParameters;
  for (;;) {

    // ledm1.displayAnimate();

    Serial.print("TaskControlPuzzleState: ");
    if (puzzleStatePotis == SOLVED && puzzleStateRewiring0 == SOLVED) {
#ifdef DEBUG_SERIAL
      Serial.println("All Puzzles Solved");
#endif
      ledm1.clear();
      ledm1.drawText(1, 14, "Solved!");

    } else {
      Serial.println("Not all Puzzles Solved!");
      ledm1.clear();
      ledm1.drawText(1, 14, "Not");
      ledm1.drawText(1, 6, "Solved!");

      // ledm1.commit();
    }

    vTaskDelay(300);
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
      vTaskSuspend(xHandleControlPuzzleState);
      Serial.println("Button 1 pressed");
      ledm1.clear();
      ledm1.setCharSpacing(0);

      ledm1.drawHLine(13, 0, 23);
      ledm1.drawText(0, 11, "STRANGER", MD_MAXPanel::ROT_0);
      ledm1.drawText(3, 5, "THINGS", MD_MAXPanel::ROT_0);
      ledm1.drawHLine(4, 0, 2);
      ledm1.drawHLine(4, 21, 23);
      ledm1.setCharSpacing(1);

      float frequencies[] = {130.81, 164.81, 196.0, 246.94,
                             261.63, 246.94, 196.0, 164.81};
      for (int i = 0; i <= 7; i++) {
        ledcWriteTone(LEDC_CHANNEL1, frequencies[i]);
        vTaskDelay(250);
      }

      ledm1.clear();
      vTaskResume(xHandleControlPuzzleState);

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