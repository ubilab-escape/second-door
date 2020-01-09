/******************************************************************
 * robot controll for UBI
 *
 * NOTES:
 *  CTRL_CLK has to be 4!
 *  reconfig of controller is done in read_gamepad()
 ******************************************************************/

#include <PS2X_lib.h>
#include <Arduino_FreeRTOS.h>

#define PS2_DAT 12
#define PS2_CMD 11
#define PS2_SEL 10
#define PS2_CLK 13

// motor pins
#define LEFT_FORWARD 3   // A2
#define LEFT_BACKWARD 5  // A1

#define RIGHT_FORWARD 6   // B1
#define RIGHT_BACKWARD 9  // B2

//#define pressures   true
#define pressures false
//#define rumble      true
#define rumble false

PS2X ps2x;

int error = 0;
byte type = 0;
byte vibrate = 0;

byte ps2_translation, ps2_rotation;

class Timer {
 public:
  void init() {
    now = millis();
    remember = millis();
  }
  void set_time() { now = millis(); }
  void remember_me() { remember = millis(); }
  unsigned long get_diff(bool rem = false) {
    unsigned long diff = now - remember;
    if (rem == true) {
      remember_me();
    }
    return diff;
  }

 private:
  unsigned long now;
  unsigned long remember;
};

Timer time;

void stop_motors() {
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
}

void drive_forward(byte val) {
  val = 255 - val;
  // val = map(val, 0, 1023, 0, 255);
  analogWrite(LEFT_FORWARD, val);
  analogWrite(RIGHT_FORWARD, val);
}

void drive_forward() {
  analogWrite(LEFT_FORWARD, 64);
  analogWrite(RIGHT_FORWARD, 64);
}

void drive_backward(byte val) {
  analogWrite(LEFT_BACKWARD, val);
  analogWrite(RIGHT_BACKWARD, val);
}

void drive_backward() {
  analogWrite(LEFT_BACKWARD, 64);
  analogWrite(RIGHT_BACKWARD, 64);
}

void turn_right() {
  analogWrite(LEFT_FORWARD, 64);
  analogWrite(RIGHT_BACKWARD, 64);
}
void turn_left() {
  analogWrite(LEFT_BACKWARD, 64);
  analogWrite(RIGHT_FORWARD, 64);
}

bool controll_button_pressed() {
  if (ps2x.Button(PSB_PAD_UP)) {
    return true;
  }
  if (ps2x.Button(PSB_PAD_DOWN)) {
    return true;
  }
  if (ps2x.Button(PSB_PAD_RIGHT)) {
    return true;
  }
  if (ps2x.Button(PSB_PAD_LEFT)) {
    return true;
  }
  if (ps2x.Analog(PSS_LY) <= 126 && ps2x.Analog(PSS_LX) >= 130) {
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);

  delay(300);
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);

  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);

  // small delay to give controller pairing
  delay(2000);
  stop_motors();
  time.init();

  // init_PS2();
}

void loop() {
  ps2x.read_gamepad(false, vibrate);

  // timing for reseting motor pins in case of keeping different button pressed
  time.set_time();

  /*
  while (error == 1) {
    stop_motors();
    Serial.println("Lost connection to controller!");
    error = ps2x.read_gamepad(false, vibrate);
    delay(1000);
  }
  */
  /*
  // Joystick
  ps2_translation = ps2x.Analog(PSS_LY);
  ps2_rotation = ps2x.Analog(PSS_LX);

  if (ps2_translation < 126) {
    drive_forward(ps2_translation);
  }
  if (ps2_translation > 130) {
    drive_backward(ps2_translation);
  }
  */
  /*
  if (ps2_translation >= 126 && ps2_translation <= 130) {
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, 0);
    analogWrite(RIGHT_BACKWARD, 0);
  }
  */
  /*
  if(ps2_rotation > 128){
    turn_right(ps2_rotation);
  }
  if(ps2_rotation < 128){
    turn_left(ps2_rotation);
  }
  */

  if (ps2x.Button(PSB_PAD_UP)) {
    drive_forward();
  }
  if (ps2x.Button(PSB_PAD_DOWN)) {
    drive_backward();
  }
  if (ps2x.Button(PSB_PAD_RIGHT)) {
    turn_right();
  }
  if (ps2x.Button(PSB_PAD_LEFT)) {
    turn_left();
  }

  if (controll_button_pressed() == false || time.get_diff() > 500) {
    Serial.println("STOP");
    stop_motors();
    time.remember_me();
  }


  Serial.print(ps2_translation);  // Left stick, Y axis. Other options: LX, RY, RX
  Serial.print(",");
  Serial.print(ps2_rotation);
  Serial.println(" ");
  delay(10);
}