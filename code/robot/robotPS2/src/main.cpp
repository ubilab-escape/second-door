/******************************************************************
 * robot controll for UBI
 *
 * NOTES:
 *  CTRL_CLK has to be 4!
 *  reconfig of controller is done in read_gamepad()
 ******************************************************************/

#include <Adafruit_NeoPixel.h>
#include <Arduino_FreeRTOS.h>
#include <Robot.h>

#define PS2_DAT 12
#define PS2_CMD 11
#define PS2_SEL 10
#define PS2_CLK 13

// motor pins
#define LEFT_FORWARD 3    // A2
#define LEFT_BACKWARD 5   // A1
#define RIGHT_FORWARD 6   // B1
#define RIGHT_BACKWARD 9  // B2

// NeoPixel
#define PIXEL_PIN 7

#define NUMPIXELS 34

//#define pressures   true
#define pressures false
//#define rumble      true
#define rumble false

#define ROBOT_ON 2

void TaskControllRobot(void *pvParameters);
void TaskBlink(void *pvParameters);
void TaskCheckESP(void *pvParameters);

Robot robot;

Adafruit_NeoPixel pixels(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);


void drive_forward(byte val) {
  val = 255 - val;
  // val = map(val, 0, 1023, 0, 255);
  analogWrite(LEFT_FORWARD, val);
  analogWrite(RIGHT_FORWARD, val);
}

void setup() {
  Serial.begin(115200);

  pinMode(ROBOT_ON, INPUT);

  pixels.begin();
  pixels.clear();

  robot.init(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble, LEFT_FORWARD, LEFT_BACKWARD,
             RIGHT_FORWARD, RIGHT_BACKWARD);

  xTaskCreate(TaskControllRobot, "ControllRobot",
              128  // This stack size can be checked & adjusted by reading Highwater
              ,
              NULL, 1  // priority
              ,
              NULL);

  xTaskCreate(TaskBlink, "Blink"  // A name just for humans
              ,
              128  // Stack size
              ,
              NULL, 2  // priority
              ,
              NULL);

  xTaskCreate(TaskCheckESP, "CheckESP"  // A name just for humans
              ,
              128  // Stack size
              ,
              NULL, 2  // priority
              ,
              NULL);
}

void loop() {
}

void TaskControllRobot(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    robot.controll();
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskBlink(void *pvParameters) {
  (void)pvParameters;

  Color color = red;

  for (;;) {
    if (robot.allowColor == true) {
      color = robot.get_color();
      for (int i = 0; i < NUMPIXELS; i++) {
        if (color == red) {
          pixels.setPixelColor(i, pixels.Color(0, 255, 0));
        }
        if (color == blue) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 255));
        }
        if (color == green) {
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        }
        if (color == magenta) {
          pixels.setPixelColor(i, pixels.Color(255, 0, 255));
        }

        pixels.show();
      }
    }
    if (robot.allowColor == false) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
      pixels.show();
    }
    vTaskDelay(5);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskCheckESP(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    if (digitalRead(ROBOT_ON) == HIGH) {
      Serial.println("ON");
      robot.robotOn = true;
      robot.allowColor = true;
    }
    if (digitalRead(ROBOT_ON) == LOW) {
      Serial.println("OFF");
      robot.robotOn = false;
      robot.allowColor = false;
    }

    vTaskDelay(50);  // one tick delay (15ms) in between reads for stability
  }
}
