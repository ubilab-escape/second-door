#include <PS2X_lib.h>

enum Color {none, red, green, blue, magenta };

class Robot {
 public:
  void init(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat, bool pressures, bool rumble, int lf,
            int lb, int rf, int rb);
  void controll();
  Color get_color();

  /**
   * is robot controllable?
   */
  bool robotOn;

 private:
  void stop_motors();
  void drive_forward();
  void drive_backward();
  void turn_right();
  void turn_left();
  bool controll_button_pressed();

  /**
   * PS2 controller handling variables
   */
  PS2X ps2x;
  int error = 0;
  byte type = 0;
  byte vibrate = 0;

  byte ps2Translation, ps2Rotation;

  Color color = none;
  int rememberButton;
  /**
   * Motor pins
   */
  int leftForward;
  int leftBackward;
  int rigthBackward;
  int rightForward;
};