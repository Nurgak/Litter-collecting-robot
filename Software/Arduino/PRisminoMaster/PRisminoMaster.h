/***************************************************************************************
 *
 * Title:      PRismino I2C master sketch communicating with the Raspberry Pi via
 *             serial and the motor controller via I2C.
 * Date:       2014-06-01
 *
 ***************************************************************************************/
#ifndef _PRISMINO_MASTER
#define _PRISMINO_MASTER

#define I2C_MOTOR_CONTROLLER_ADDRESS 0x04
#define I2C_COMPASS_ADDRESS 0x1E //0011110b, I2C 7bit address of HMC5883

#define PIN_LIGHTS 9

#define IR_FAR_LEFT 0
#define IR_LEFT 1
#define IR_RIGHT 2
#define IR_FAR_RIGHT 3

#define SERVO_LEFT_UP 10
#define SERVO_LEFT_DOWN 170
#define SERVO_RIGHT_UP 170
#define SERVO_RIGHT_DOWN 10

#define TONE_I2C_ERROR 330
#define TONE_BEEP 440
#define TONE_BATTERY 1100

#define TIME_CHECK_BATTERY 1000
#define TIME_ANTI_LOOP 3000
#define TIME_COMPASS_CALIBRATION 13000
#define TIME_WALL_CHECK_TURN 700
#define TIME_WALL_RECHECK 2000 //do not recheck for walls if a falls positive was found

// in percent the maximum allowed setting speed
#define CONST_SPEED_MAX 100
// in percent the maximum speed while avoiding obstacles
#define CONST_SPEED_OBSTACLE 100

// in milliseconds, the pause time between 1 degree change
#define CONST_MAX_SERVO_SPEED 10
// calibrated value, about 7.1V
#define CONST_BATTERY_LOW 470

// IR sensor value under which an obstacle is detected
#define CONST_IR_OBSTACLE_SIDE 300
#define CONST_IR_OBSTACLE_CENTER 310
#define CONST_IR_OBSTACLE_SIDE_CAGE_DOWN 160
#define CONST_IR_OBSTACLE_CENTER_CAGE_DOWN 140

#define CONST_KP 10


enum items
{
  FREE,
  OBSTACLE
};

enum cage_positions
{
  CAGE_UP,
  CAGE_DOWN
};

#endif

