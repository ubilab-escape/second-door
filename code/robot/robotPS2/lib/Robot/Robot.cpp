#include <Robot.h>

void Robot::init(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat, bool pressures, bool rumble,
                 int lf, int lb, int rf, int rb) {
  // small delay for pairing time
  delay(300);
  error = ps2x.config_gamepad(clk, cmd, att, dat, pressures, rumble);
  delay(300);

  leftForward = lf;
  leftBackward = lb;
  rightForward = rf;
  rigthBackward = rb;

  robotOn = false;

  pinMode(leftForward, OUTPUT);
  pinMode(leftBackward, OUTPUT);
  pinMode(rightForward, OUTPUT);
  pinMode(rigthBackward, OUTPUT);

  stop_motors();
}

void Robot::controll() {
  ps2x.read_gamepad(false, vibrate);
  if (robotOn == true) {
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
    if (controll_button_pressed() == false) {
      stop_motors();
    }
    // for colors
    if (ps2x.Button(PSB_CIRCLE)) {
      color = red;
    }
    if (ps2x.Button(PSB_TRIANGLE)) {
      color = green;
    }
    if (ps2x.Button(PSB_SQUARE)) {
      color = magenta;
    }
    if (ps2x.Button(PSB_CROSS)) {
      color = blue;
    }
  }
  if(robotOn == false){
    color = none;
  }
}

void Robot::stop_motors() {
  analogWrite(leftForward, 0);
  analogWrite(rightForward, 0);
  analogWrite(leftBackward, 0);
  analogWrite(rigthBackward, 0);
}

void Robot::drive_forward() {
  analogWrite(leftForward, 64);
  analogWrite(rightForward, 64);
}

void Robot::drive_backward() {
  analogWrite(leftBackward, 64);
  analogWrite(rigthBackward, 64);
}

void Robot::turn_right() {
  analogWrite(leftForward, 64);
  analogWrite(rigthBackward, 64);
}

void Robot::turn_left() {
  analogWrite(leftBackward, 64);
  analogWrite(rightForward, 64);
}

bool Robot::controll_button_pressed() {
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

Color Robot::get_color() { return color; }