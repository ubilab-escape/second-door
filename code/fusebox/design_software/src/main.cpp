// define this to get serial debug messages
#define SERIAL_DEBUG

#include "Fonts.h"
#include "MAX7221.h"
#include "Wire.h"
#include "serial_debug.h"
#include "wifi_secure.h"
#include <Adafruit_ADS1015.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <MD_MAXPanel.h>
#include <MqttBase.h>
#include <pcf8574_esp.h>
#include <vector>

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
enum tPuzzleState { INACTIVE, ACTIVE, SOLVED, UNSOLVED };
tPuzzleState puzzleStateRewiring0 = UNSOLVED;
tPuzzleState puzzleStateRewiring1 = UNSOLVED;
tPuzzleState puzzleStatePotis = UNSOLVED;
tPuzzleState puzzleStateLaserLock = UNSOLVED;

bool potentiometerTaskDeleted = false;
bool rewiring0TaskDeleted = false;
bool rewiring1TaskDeleted = false;
bool laserDetectionTaskDeleted = false;

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
void blink_ring(uint8_t blinking_number, uint8_t frequency);
uint8_t analyse_sequence(uint8_t sequence[6], uint8_t target);

// ADC
Adafruit_ADS1015 ads;

// Port Expander
PCF857x portExpander0(0x20, &Wire);
PCF857x portExpander1(0x21, &Wire);

// MQTT stuff
MqttBase *mqttCommunication;
void callbackLaserDetection(const char *method1, const char *state, int daten);
void callbackRewiring0(const char *method1, const char *state, int daten);
void callbackRewiring1(const char *method1, const char *state, int daten);
void callbackPotentiometer(const char *method1, const char *state, int daten);

// Task Attachment
void TaskPotentiometerReadout(void *pvParameters);
void TaskWiring0Readout(void *pvParameters);
void TaskWiring1Readout(void *pvParameters);
void TaskPiezoButtonReadout(void *pvParameters);
void TaskControlPuzzleState(void *pvParameters);
void TaskLaserLock(void *pvParameters);
void TaskControlButtonsReadout(void *pvParameters); // TODO: Implement
void TaskMqttLoop(void *pvParameters);
void TaskMqttPublish(void *pvParameters);

// Task Handles
TaskHandle_t xHandleLedRing;
TaskHandle_t xHandleControlPuzzleState;
TaskHandle_t xHandlePotentiometerReadout;
TaskHandle_t xHandleWiring0Readout;
TaskHandle_t xHandleWiring1Readout;
TaskHandle_t xHandleMqttLoop;
TaskHandle_t xHandleMqttPublish;

// Init Functions
void initPotentiometers(void);
void initLedRing(void);
void initPortExpander(void);
void initPiezoBuzzer(void);
void initLedMatrix(void);
void initSevenSegment(void);
void initMqtt(void);

void setup() {

  Serial.begin(115200);
  Serial.println("Setup started ...");

  // disableCore0WDT();
  // disableCore1WDT();

  initMqtt();
  initPotentiometers();
  initPiezoBuzzer();
  initLedRing();
  initPortExpander();
  initSevenSegment();
  initLedMatrix();

  Serial.println("Setup finished");

  // Attach Tasks to Scheduler
  xTaskCreatePinnedToCore(TaskControlPuzzleState, "TaskControlPuzzleState",
                          4096, NULL, 1, &xHandleControlPuzzleState, 0);

  // Mqtt Tasks
  xTaskCreatePinnedToCore(TaskMqttLoop, "TaskMqttLoop", 4096, NULL, 1,
                          &xHandleMqttLoop, 0);
  xTaskCreatePinnedToCore(TaskMqttPublish, "TaskMqttPublish", 4096, NULL, 2,
                          &xHandleMqttPublish, 0);

  // Riddle Tasks

  xTaskCreatePinnedToCore(TaskPotentiometerReadout, "TaskPotentiometerReadout",
                          2048, NULL, 2, &xHandlePotentiometerReadout, 1);

  xTaskCreatePinnedToCore(TaskWiring0Readout, "TaskWiring0Readout", 2048, NULL,
                          3, &xHandleWiring0Readout, 1);
  
  xTaskCreatePinnedToCore(TaskWiring1Readout, "TaskWiring1Readout", 2048, NULL,
                          3, &xHandleWiring1Readout, 1);

  xTaskCreatePinnedToCore(TaskLaserLock, "TaskLaserLock", 2048, NULL, 2,
                          &xHandleLedRing, 1);

  // Fake Riddle Tasks
  xTaskCreatePinnedToCore(TaskPiezoButtonReadout, "TaskPiezoButtonReadout",
                          2048, NULL, 2, NULL, 1);
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
  Serial.print("Setup LED Ring ... ");
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
  pinMode(LOCK_0, OUTPUT);     // Lock als Ausgang setzen

  Serial.println("done!");
}

void initMqtt(void) {
  Serial.print("Setup MQTT ... ");

  mqttCommunication = new MqttBase("10.0.0.2", "test187", 1883);

  std::vector<std::shared_ptr<std::string>> mqttTopics;
  mqttTopics.push_back(
      std::make_shared<std::string>("7/fusebox/laserDetection"));
  mqttTopics.push_back(std::make_shared<std::string>("7/fusebox/rewiring0"));
  mqttTopics.push_back(std::make_shared<std::string>("7/fusebox/rewiring1"));
  mqttTopics.push_back(
      std::make_shared<std::string>("7/fusebox/potentiometer"));

  std::vector<std::function<void(const char *, const char *, int)>>
      logicCallbacks;
  logicCallbacks.push_back(callbackLaserDetection);
  logicCallbacks.push_back(callbackRewiring0);
  logicCallbacks.push_back(callbackRewiring1);
  logicCallbacks.push_back(callbackPotentiometer);

  mqttCommunication->init(ssid, password, mqttTopics, logicCallbacks);

  Serial.println("done!");
}

void TaskPotentiometerReadout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    SERIALPRINTS("TaskPotentiometerReadout: \t")

    uint16_t pValues[4];
    float adcValues[4];

    for (uint8_t i = 0; i < 4; i++) {
      adcValues[i] = ads.readADC_SingleEnded(i);
      adcValues[i] = adcValues[i] * 9 / 1024;
      pValues[i] = adcValues[i];
      SERIALPRINT("", i);
      SERIALPRINTS(": ");
      SERIALPRINT("", pValues[i]);
      SERIALPRINTS(" ");
    }

    seg1.transferData(0x01, pValues[0]);
    seg1.transferData(0x02, pValues[1]);
    seg1.transferData(0x03, pValues[2]);
    seg1.transferData(0x04, pValues[3]);

    if (pValues[0] == 1 && pValues[1] == 9 && pValues[2] == 9 &&
        pValues[3] == 5) {
      SERIALPRINTS("Potis Solved!\n");
      puzzleStatePotis = SOLVED;

    } else {
      SERIALPRINTS("Potis not solved!\n");
      puzzleStatePotis = UNSOLVED;
    }

    vTaskDelay(500);
  }
}

void TaskLaserLock(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    // SERIALPRINTS("TaskLaserLock: \t\t");
    static uint8_t byte_count = 0;

    sequence[byte_count] = digitalRead(detectorPin);

    if (byte_count == SEQUENDE_SIZE) {
      byte_count = 0;
      uint8_t zeros = analyse_sequence(sequence, 0);
      uint8_t ones = analyse_sequence(sequence, 1);

      // chekc if sequenz is correct
      if (zeros == ones && puzzleStateLaserLock == UNSOLVED) {
        SERIALPRINTS("Sequence detected ");
        SERIALPRINT("", zeros);
        SERIALPRINTS(" ");
        SERIALPRINT("", ones);
        SERIALPRINTS("\n");
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
      puzzleStateLaserLock = SOLVED;
      SERIALPRINTS("Fuse Box open \n");
      numberOfSequences = 0;
      blink_ring(5, 2);

      // open LOCK
      digitalWrite(LOCK_0, HIGH);
      vTaskDelay(1000);
      digitalWrite(LOCK_0, LOW);
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
    // SERIALPRINTS(" not solved yet!\n");
    vTaskDelay(10);
  }
}

void TaskWiring0Readout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    SERIALPRINTS("TaskWiring0Readout: \t\t");

    float rewiringValues0[5];
    uint16_t rewiringPins0[] = {REWIRE_0_1, REWIRE_0_2, REWIRE_0_3, REWIRE_0_4,
                                REWIRE_0_5};
    for (int i = 0; i <= 4; i++) {
      rewiringValues0[i] = analogRead(rewiringPins0[i]);
      rewiringValues0[i] = rewiringValues0[i] / 4095 * 33;
      SERIALPRINT("", i);
      SERIALPRINTS(": ");
      SERIALPRINT("", rewiringValues0[i]);
      SERIALPRINTS(" ");
    }

    // Measured Values: 33 25 19 13 7
    // TODO: Problem with [0]: WiFi and ADC not working together,
    // TODO: replaced 13 by 33
    if ((rewiringValues0[3] >= 30 && rewiringValues0[3] <= 36) &&
        (rewiringValues0[4] >= 22 && rewiringValues0[4] <= 28) &&
        (rewiringValues0[1] >= 16 && rewiringValues0[1] <= 22) &&
        (rewiringValues0[0] >= 30 && rewiringValues0[0] <= 36) &&
        (rewiringValues0[2] >= 4 && rewiringValues0[2] <= 10)) {
      SERIALPRINTS("Rewiring 0 solved!\n");
      puzzleStateRewiring0 = SOLVED;
    } else {
      SERIALPRINTS("Rewiring 0 not solved!\n");
      puzzleStateRewiring0 = UNSOLVED;
    }

    vTaskDelay(2000);
  }
}

void TaskWiring1Readout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    SERIALPRINTS("TaskWiring1Readout: \t\t");

    uint8_t values = portExpander0.read8();
    // 0x58 + 0x42 = 0x9A = 0b10011010
    if (values == 0b10011010) {
      SERIALPRINTS("Rewiring 1 solved!\n")
      puzzleStateRewiring1 = SOLVED;
    } else {
      SERIALPRINTS("Rewiring 1 not solved!\n");
      puzzleStateRewiring1 = UNSOLVED;
    }
    vTaskDelay(2000);
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
  uint8_t len = 6;
  for (int i = 0; i < len; i++) {
    if (sequence[i] == target) {
      target_number++;
    }
  }
  return target_number + 1;
}

String getStateDecription(eTaskState state) {
  if (state == eReady) {
    return "eReady";
  } else if (state == eRunning) {
    return "eRunning";
  } else if (state == eBlocked) {
    return "eBlocked";
  } else if (state == eSuspended) {
    return "eSuspended";
  } else if (state == eDeleted) {
    return "eDeleted";
  } else {
    return "smth else";
  }
}

void TaskControlPuzzleState(void *pvParameters) {
  (void)pvParameters;
  for (;;) {

    if (puzzleStatePotis == SOLVED && puzzleStateRewiring0 == SOLVED &&
        puzzleStateRewiring1 == SOLVED && puzzleStateLaserLock == SOLVED) {
      ledm1.clear();
      ledm1.drawText(25, 7, "Solved!", MD_MAXPanel::ROT_180);

    } else {

      ledm1.clear();
      if (puzzleStatePotis == SOLVED) {
        ledm1.drawCircle(4, 11, 2);
        // vTaskSuspend(xHandlePotentiometerReadout);
      } else {
        ledm1.drawRectangle(4, 11, 6, 13);
      }
      if (puzzleStateRewiring0 == SOLVED) {
        ledm1.drawCircle(12, 11, 2);
        // vTaskSuspend(xHandleWiring0Readout);
      } else {
        ledm1.drawRectangle(12, 11, 14, 13);
      }
      if (puzzleStateRewiring1 == SOLVED) {
        ledm1.drawCircle(20, 11, 2);
        // vTaskSuspend(xHandleWiring1Readout);
      } else {
        ledm1.drawRectangle(20, 11, 22, 13);
      }
      if (puzzleStateLaserLock == SOLVED) {
        ledm1.drawCircle(28, 11, 2);
        // vTaskSuspend(xHandleLedRing);
      } else {
        ledm1.drawRectangle(28, 11, 30, 13);
      }
    }

    vTaskDelay(300);
  }
}

void TaskPiezoButtonReadout(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    SERIALPRINTS("TaskPiezoButtonReadout: \t");
    uint16_t buttonState1 = digitalRead(BUTTON_0);
    uint16_t buttonState2 = digitalRead(BUTTON_1);
    if (buttonState1) {
      vTaskSuspend(xHandleControlPuzzleState);
      SERIALPRINTS("Button 1 pressed\n");
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
      SERIALPRINTS("Button 2 pressed\n");
      // e 164.81
      // d# 155.56
      float frequencies[] = {164.81, 155.56, 164.81, 155.56,
                             123.47, 146.83, 130.81, 110};
      for (int i = 0; i <= 7; i++) {
        ledcWriteTone(LEDC_CHANNEL1, frequencies[i]);
        vTaskDelay(250);
      }
    } else {
      SERIALPRINTS("No Button pressed\n");
      ledcWriteTone(LEDC_CHANNEL1, 0);
    }
    vTaskDelay(1000);
  }
}

void TaskMqttLoop(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    mqttCommunication->loop();
    vTaskDelay(500);
  }
}

void TaskMqttPublish(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    if (puzzleStatePotis == SOLVED) {
      mqttCommunication->publish("7/fusebox/potentiometer", "STATUS", "solved",
                                 true);
    } else {
      mqttCommunication->publish("7/fusebox/potentiometer", "STATUS",
                                 "unsolved", false);
    }
    vTaskDelay(1000);
    if (puzzleStateRewiring0 == SOLVED) {
      mqttCommunication->publish("7/fusebox/rewiring0", "STATUS", "solved",
                                 true);
    } else {
      mqttCommunication->publish("7/fusebox/rewiring0", "STATUS", "unsolved",
                                 false);
    }
    vTaskDelay(1000);
    if (puzzleStateRewiring1 == SOLVED) {
      mqttCommunication->publish("7/fusebox/rewiring1", "STATUS", "solved",
                                 true);
    } else {
      mqttCommunication->publish("7/fusebox/rewiring1", "STATUS", "unsolved",
                                 false);
    }
    vTaskDelay(1000);
    if (puzzleStateLaserLock == SOLVED) {
      mqttCommunication->publish("7/fusebox/laserDetection", "STATUS", "solved",
                                 true);
    } else {
      mqttCommunication->publish("7/fusebox/laserDetection", "STATUS",
                                 "unsolved", false);
    }
    vTaskDelay(1000);
  }
}

void callbackLaserDetection(const char *method1, const char *state, int daten) {
  vTaskSuspend(xHandleMqttPublish);
  if (strcmp(state, "solved") == 0) {
    puzzleStateLaserLock = SOLVED;
    if (laserDetectionTaskDeleted != true) {
      Serial.println("Delete Laser Detection Task!");
      vTaskDelete(xHandleLedRing);
      laserDetectionTaskDeleted = true;
    }
  } else if (strcmp(state, "unsolved") == 0) {
    puzzleStateLaserLock = UNSOLVED;
  }
  vTaskResume(xHandleMqttPublish);
}
void callbackRewiring0(const char *method1, const char *state, int daten) {
  vTaskSuspend(xHandleMqttPublish);
  if (strcmp(state, "solved") == 0) {
    puzzleStateRewiring0 = SOLVED;
    if (rewiring0TaskDeleted != true) {
      Serial.println("Delete Rewiring 0 Task!");
      vTaskDelete(xHandleWiring0Readout);
      rewiring0TaskDeleted = true;
    }
  } else if (strcmp(state, "unsolved") == 0) {
    puzzleStateRewiring0 = UNSOLVED;
  }
  vTaskResume(xHandleMqttPublish);
}
void callbackRewiring1(const char *method1, const char *state, int daten) {
  vTaskSuspend(xHandleMqttPublish);
  if (strcmp(state, "solved") == 0) {
    puzzleStateRewiring1 = SOLVED;
    if (rewiring1TaskDeleted != true) {
      Serial.println("Delete Rewiring 1 Task!");
      vTaskDelete(xHandleWiring1Readout);
      rewiring1TaskDeleted = true;
    }
  } else if (strcmp(state, "unsolved") == 0) {
    puzzleStateRewiring1 = UNSOLVED;
  }
  vTaskResume(xHandleMqttPublish);
}
void callbackPotentiometer(const char *method1, const char *state, int daten) {
  vTaskSuspend(xHandleMqttPublish);
  if (strcmp(state, "solved") == 0) {
    puzzleStatePotis = SOLVED;
    if (potentiometerTaskDeleted != true) {
      Serial.println("Delete Poti Task!");
      vTaskDelete(xHandlePotentiometerReadout);
      potentiometerTaskDeleted = true;
    }  
  } else if (strcmp(state, "unsolved") == 0) {
    puzzleStatePotis = UNSOLVED;
  }
  vTaskResume(xHandleMqttPublish);
}