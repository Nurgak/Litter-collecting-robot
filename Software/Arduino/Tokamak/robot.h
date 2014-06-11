/***************************************************************************************
 *
 * Title:      STI competition Arduino code for the Tokamak robot
 * Date:       2014-06-06
 *
 ***************************************************************************************/
#ifndef _PRISMINO_MASTER
#define _PRISMINO_MASTER

// comment this line when testing without the motor controller
#define ENABLE_CONTROLLER
// comment this line to disable all bluetooth functionalities
#define ENABLE_BLUETOOTH
// comment this line to turn off sounds
#define ENABLE_SOUND
// comment this line to disable compass
#define ENABLE_COMPASS

#define Bluetooth Serial1

#define I2C_MOTOR_CONTROLLER_ADDRESS 0x04
#define I2C_COMPASS_ADDRESS 0x1E

#define PIN_LIGHTS 9

#define SENSOR_IR_LEFT 0
#define SENSOR_IR_CENTER_LEFT 1
#define SENSOR_IR_CENTER_RIGHT 2
#define SENSOR_IR_RIGHT 3

#define SERVO_LEFT_UP 10
// set up position to 5 when the mast is installed
//#define SERVO_LEFT_UP 50
#define SERVO_LEFT_DOWN 170
#define SERVO_RIGHT_UP SERVO_LEFT_DOWN
#define SERVO_RIGHT_DOWN SERVO_LEFT_UP

#define TONE_I2C_ERROR 3300
#define TONE_BEEP 440
#define TONE_BATTERY 1100

#define TIME_CHECK_BATTERY 1000
#define TIME_ANTI_LOOP 3000
#define TIME_ANTI_LOOP_TIMEOUT_TURN 1000
// if a bottle hasn't been detected during this time consider it a false positive
#define TIME_BOTTLE_SEEN_TIMEOUT 2000
// after a false positive the robot won't check for a bottle for this amount of time
#define TIME_NEXT_BOTTLE_CHECK 1000
// when an obstacle has been seen instead of a wall in the STATE_FIND_WALL state do not check for a wall for this amount of time
#define TIME_WALL_RECHECK 4000
#define TIME_TURN_FOLLOW_WALL 1500
#define TIME_TURN_AROUND 1000

#define TIME_WALL_CHECK_TURN 700

#define TIME_GRAB_LOST_BOTTLE 500

// in percent the maximum allowed setting speed
#define CONST_SPEED_MAX 100
// in percent the maximum speed while avoiding obstacles
#define CONST_SPEED_OBSTACLE 100
// in percent the speed of approach when a bottle was seen, 60 is not enough with low-power 172:1 and 7.2V battery
#define CONST_SPEED_BOTTLE 80
// after every setSpeed() wait this long before continuing
#define CONST_SPEED_SET_DELAY 50

// this is the threshold when to lower the cage, the constant indicated the minimum box size the bottle must be detected in
#define CONST_BOTTLE_SIZE_LOWER_CAGE 90
// acceptable error when heading towards a direction (goes straight forward)
#define CONST_DEVIATION_OK 15
#define CONST_DEVIATION_OK_STRICT 5
// number of times the robot must change direction without going forwards for it to be considered stuck
#define CONST_LOOP_TURN_TIMES 3

// in milliseconds, the pause time between 1 degree change
#define CONST_MAX_SERVO_SPEED 10
// calibrated value, about 7.1V
#define CONST_BATTERY_LOW 470

// IR sensor value under which an obstacle is detected
#define CONST_IR_OBSTACLE_SIDE 300
#define CONST_IR_OBSTACLE_CENTER 310
// IR sensor values to use when the cage is deployed
#define CONST_IR_OBSTACLE_SIDE_CAGE 160
#define CONST_IR_OBSTACLE_CENTER_CAGE 140
// value at which there an obstacle far away
#define CONST_IR_NO_OBSTACLE 100
#define CONST_IR_TOO_CLOSE 250

// when following a wall the sensor should remain between these values
#define CONST_IR_WALL_FOLLOW_MIN 160
#define CONST_IR_WALL_FOLLOW_MAX 200

#define CONST_IR_OBSTACLE_FAR 100

// direction towards which to go when going home
#define CONST_HEADING_HOME 50

enum comm_methods
{
  USB,
  BLUETOOTH
};
  
enum cage_positions
{
  CAGE_UP,
  CAGE_DOWN
};

enum state_codes
{
  STATE_SEARCHING,
  STATE_FETCHING_BOTTLE,
  STATE_LOWER_CAGE,
  STATE_GO_HOME,
  STATE_RAISE_CAGE
};

enum return_codes
{
  OK,
  FAIL,
  REPEAT
};

enum sounds
{
  COIN,
  POWERUP,
  ONEUP,
  FLAGPOLE
};

enum sides
{
  LEFT,
  RIGHT
};

enum state_wall
{
  NO_WALL,
  WALL_LEFT,
  WALL_RIGHT
};

//#define ENTRY_STATE STATE_SEARCHING
#define ENTRY_STATE STATE_SEARCHING

class Tokamak
{
  public: 
  struct Flags
  {
    uint8_t enableFrontLeds :1;
    uint8_t cagePosition :1;
    uint8_t running: 1;
    uint8_t wall: 2;
  };
  
  Servo servoLeft;
  Servo servoRight;
  uint16_t batteryVoltage;
  uint16_t currentLeft;
  uint16_t currentRight;
  
  char readInput(comm_methods);
  void sendOutput(comm_methods, const char*);
  
  Flags flags;
  
  Tokamak();
  void setCagePosition(cage_positions);
  void setSpeed(int8_t, int8_t);
  void turn(int16_t, int8_t);
  void stop(void);
  void setLights(boolean);
  void checkBattery(void);
  int16_t getHeading();
  // default speed is the maximum
  void headTo(int16_t, int8_t = CONST_SPEED_MAX);
  void readIrSensors(uint16_t*, uint16_t*, uint16_t*, uint16_t*);
  void playSound(sounds);
  
  void followWall(uint16_t*, sides, int8_t = CONST_SPEED_MAX);
  void navigation_avoidance(int16_t);
  void gotoHeading(int16_t direction_home = CONST_HEADING_HOME);
  void resetReturnBase(void);
  return_codes goHome(int16_t = CONST_HEADING_HOME);
  uint8_t detectWall(void);
};

#endif

