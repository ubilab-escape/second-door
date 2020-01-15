#include "MAX7221.h"
#include "Wire.h"
#include <Adafruit_ADS1015.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <MD_MAXPanel.h>
#include <pcf8574_esp.h>
#include "Fonts.h"

// #define DEBUG

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define MAX7221_CS 5
#define MAX7219_CS 12
#define PIEZO_0 13
#define BUTTON_0 16
#define BUTTON_1 17
#define LEDC_CHANNEL1 0
#define LEDC_RESOLUTION 8
#define LOCK_0 14
#define SEQUENDE_SIZE 5
#define MAX_SEQUENZES 32
#define RGB_RING_PIN 27
#define NUM_PIXEL 16
#define detectorPin 26
#define REWIRE_0_1 25
#define REWIRE_0_2 33
#define REWIRE_0_3 32
#define REWIRE_0_4 35
#define REWIRE_0_5 34

// Puzzle States
enum tPuzzleState { INIT, SOLVED, NOT_SOLVED };
tPuzzleState puzzleStateRewiring0;
tPuzzleState puzzleStateRewiring1;
tPuzzleState puzzleStatePotis;

// 7 Segment
MAX7221 seg1 = MAX7221(MAX7221_CS, 1, MAX7221::SEGMENT);

// LedMatrix
MD_MAXPanel ledm1 = MD_MAXPanel(MD_MAX72XX::FC16_HW, MAX7219_CS, 4, 2);

// Led Ring
Adafruit_NeoPixel pixels =
    Adafruit_NeoPixel(NUM_PIXEL, RGB_RING_PIN, NEO_GRB + NEO_KHZ800);
uint8_t sequence[6];
uint8_t numberOfSequences = 0;
int old_time_in_ms = millis();
bool puzzle_solved = false;

// ADC
Adafruit_ADS1015 ads;

// Port Expander
PCF857x portExpander0(0x20, &Wire);
PCF857x portExpander1(0x21, &Wire);

// Task Attachment
void TaskPotentiometerReadout(void *pvParameters);
void TaskWiring0Readout(void *pvParameters);
void TaskWiring1Readout(void *pvParameters);
void TaskPiezoButtonReadout(void *pvParameters);
void TaskControlPuzzleState(void *pvParameters);
void TaskLaserLock(void *pvParameters);
void TaskControlButtonsReadout(void *pvParameters); // TODO: Implement
TaskHandle_t xHandleLedRing;
TaskHandle_t xHandleControlPuzzleState;
TaskHandle_t xHandlePotentiometerReadout;
TaskHandle_t xHandleWiring0Readout;
TaskHandle_t xHandleWiring1Readout;

void blink_ring(uint8_t blinking_number, uint8_t frequency);
uint8_t analyse_sequence(uint8_t sequence[6], uint8_t target);

// Init Functions
void initPotentiometers(void);
void initLedRing(void);
void initPortExpander(void);
void initPiezoBuzzer(void);
void initLedMatrix(void);
void initSevenSegment(void);

void setup() {

  // TODO: Implement MQTT Connection

  puzzleStateRewiring0 = INIT;
  puzzleStatePotis = INIT;

  // disableCore0WDT();
  // disableCore1WDT();

  Serial.begin(115200);
  Serial.println("Setup started ...");

  initPotentiometers();
  initPiezoBuzzer();
  initLedRing();
  initPortExpander();
  initSevenSegment();
  initLedMatrix();

  // Lock
  pinMode(LOCK_0, OUTPUT);
  

  Serial.println("Setup finished");

  // Attach Tasks to Scheduler
  xTaskCreatePinnedToCore(TaskControlPuzzleState, "TaskControlPuzzleState",
                          2048, NULL, 1, &xHandleControlPuzzleState, 0);

#ifndef DEBUG
  xTaskCreatePinnedToCore(TaskLaserLock, "TaskLaserLock", 2048, NULL, 2,
                          &xHandleLedRing, 1);
#else
  xTaskCreatePinnedToCore(TaskPotentiometerReadout, "TaskPotentiometerReadout",
                          2048, NULL, 2, &xHandlePotentiometerReadout, 1);
  xTaskCreatePinnedToCore(TaskWiring0Readout, "TaskWiring0Readout", 2048, NULL,
                          3, &xHandleWiring0Readout, 0);
  xTaskCreatePinnedToCore(TaskWiring1Readout, "TaskWiring1Readout", 2048, NULL,
                          3, &xHandleWiring1Readout, 0);
  xTaskCreatePinnedToCore(TaskPiezoButtonReadout, "TaskPiezoButtonReadout",
                          2048, NULL, 2, NULL, 1);

#endif
}

void loop() {
  // no code here!
  vTaskDelete(NULL);
}

void initLedMatrix(void) {
  Serial.print("Setup LED Matrix ... ");
  ledm1.begin();
  ledm1.setFont(_Fixed_5x3);
  Serial.println("done!");
}

void initSevenSegment(void) {
  Serial.print("Setup Seven Segment Display ... ");
  seg1.initMAX();
  Serial.println("done!");
}

void initPotentiometers(void) {
  Serial.print("Setup Potentiometer ... ");
  ads.begin();
  Serial.println("done!");
}

void initPiezoBuzzer(void) {
  Serial.print("Setup Piezobuzzer ... ");
  pinMode(BUTTON_0, INPUT);
  pinMode(BUTTON_1, INPUT);
  // Init PWM Signals for Buzzers
  ledcSetup(LEDC_CHANNEL1, 2000, LEDC_RESOLUTION);
  ledcAttachPin(PIEZO_0, LEDC_CHANNEL1);
  Serial.println("done!");
}

void initPortExpander(void) {
  Serial.print("Setup Port Expander ... ");
  portExpander0.begin();
  portExpander1.begin();
  Serial.println("done!");
}

void initLedRing(void) {
  Serial.println("Setup LED Ring ... ");
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

  Serial.println("done!");
}

void TaskPotentiometerReadout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    Serial.print("TaskPotentiometerReadout: ");
    uint16_t pValues[4];
    uint16_t adcValues[4];

    adcValues[0] = ads.readADC_SingleEnded(0);
    adcValues[1] = ads.readADC_SingleEnded(1);
    adcValues[2] = ads.readADC_SingleEnded(2);
    adcValues[3] = ads.readADC_SingleEnded(3);

    for (uint8_t i = 0; i < 4; i++) {
      if (adcValues[i] == 0) {
        adcValues[i] = 1;
      }
    }

    pValues[0] = map(adcValues[0], 0, 1024, 0, 9);
    pValues[1] = map(adcValues[1], 0, 1024, 0, 9);
    pValues[2] = map(adcValues[2], 0, 1024, 0, 9);
    pValues[3] = map(adcValues[3], 0, 1025, 0, 9);

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
                              "TaskPotentiometerReadout", 2048,
                              &xHandlePotentiometerReadout, 2, NULL, 1);
      xTaskCreatePinnedToCore(TaskWiring0Readout, "TaskWiring0Readout", 2048,
                              &xHandleWiring0Readout, 3, NULL, 0);
      xTaskCreatePinnedToCore(TaskWiring1Readout, "TaskWiring0Readout", 2048,
                              &xHandleWiring1Readout, 3, NULL, 0);
      xTaskCreatePinnedToCore(TaskPiezoButtonReadout, "TaskPiezoButtonReadout",
                              2048, NULL, 2, NULL, 1);

      // open LOCK
      digitalWrite(LOCK_0, HIGH);
      vTaskDelay(1000);
      digitalWrite(LOCK_0, LOW);

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

void TaskWiring0Readout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    Serial.print("TaskWiring0Readout: ");

    uint16_t rewiringValues0[5];
    uint16_t rewiringPins0[] = {REWIRE_0_1, REWIRE_0_2, REWIRE_0_3, REWIRE_0_4,
                                REWIRE_0_5};
    for (int i = 0; i <= 4; i++) {
      rewiringValues0[i] = analogRead(rewiringPins0[i]);
      rewiringValues0[i] = map(rewiringValues0[i], 0, 4095, 0, 33);
    }
    //TODO: CHANGE ORDER
    // Measured Values: 33 25 19 13 7
    if ((rewiringValues0[0] >= 30 && rewiringValues0[0] <= 36) &&
        (rewiringValues0[1] >= 22 && rewiringValues0[1] <= 28) &&
        (rewiringValues0[2] >= 16 && rewiringValues0[2] <= 22) &&
        (rewiringValues0[3] >= 10 && rewiringValues0[3] <= 16) &&
        (rewiringValues0[4] >= 04 && rewiringValues0[4] <= 10)) {
      Serial.println("Rewiring 0 solved!");
      puzzleStateRewiring0 = SOLVED;
    } else {
      Serial.println("Rewiring 0 not solved!");
      puzzleStateRewiring0 = NOT_SOLVED;
    }

    vTaskDelay(500);
  }
}

void TaskWiring1Readout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    Serial.print("TaskWiring1Readout: ");

    uint8_t values = portExpander0.read8();
    // Serial.print("Port Expander Values: ");
    // Serial.println(values, BIN);

    // 0x58 + 0x42 = 0x9A = 0b10011010

    if (values == 0b10011010) {
      Serial.println("Rewiring 1 solved!");
      puzzleStateRewiring1 = SOLVED;
    } else {
      Serial.println("Rewering 1 not solved!");
      puzzleStateRewiring1 = NOT_SOLVED;
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
    Serial.print("TaskControlPuzzleState: ");

    if (puzzleStatePotis == SOLVED && puzzleStateRewiring0 == SOLVED &&
        puzzleStateRewiring1 == SOLVED) {
      Serial.println("All Puzzles Solved");
      ledm1.clear();
      ledm1.drawText(1, 14, "Solved!");

      // Remove Solved Tasks
      vTaskSuspend(xHandleWiring0Readout);
      vTaskSuspend(xHandleWiring1Readout);
      vTaskSuspend(xHandlePotentiometerReadout);

      // TODO: OPEN LOCK!!!
      // TODO: MQTT update

    } else {
      Serial.println("Not all Puzzles Solved!");

      ledm1.clear();
      if (puzzleStatePotis == SOLVED) {
        ledm1.drawCircle(4, 11, 2);
      } else {
        ledm1.drawRectangle(4, 11, 6, 13);
      }
      if (puzzleStateRewiring0 == SOLVED) {
        ledm1.drawCircle(12, 11, 2);
      } else {
        ledm1.drawRectangle(12, 11, 14, 13);
      }
      if (puzzleStateRewiring1 == SOLVED) {
        ledm1.drawCircle(20, 11, 2);
      } else {
        ledm1.drawRectangle(20, 11, 22, 13);
      }
    }

    vTaskDelay(300);
  }
}

void TaskPiezoButtonReadout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    Serial.print("TaskPiezoButtonReadout: ");
    uint16_t buttonState1 = digitalRead(BUTTON_0);
    uint16_t buttonState2 = digitalRead(BUTTON_1);
    if (buttonState1) {
      vTaskSuspend(xHandleControlPuzzleState);
      Serial.println("Button 1 pressed");
      ledm1.clear();
      // ledm1.setCharSpacing(0);

      // upper right corner => 0, 0

      ledm1.drawHLine(1, 30, 0);
      ledm1.drawText(30, 3, "STRANGER", MD_MAXPanel::ROT_180);
      ledm1.drawText(26, 9, "THINGS", MD_MAXPanel::ROT_180);
      // ledm1.drawHLine(4, 0, 2);
      // ledm1.drawHLine(4, 26, 28);
      // ledm1.setCharSpacing(1);

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