/*
 * File:          plan_b.c
 * Date:          
 * Description:   
 * Author:        
 * Modifications: 
 */

/*
 * You may need to add include files like <webots/distance_sensor.h> or
 * <webots/differential_wheels.h>, etc.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/compass.h>
#include <webots/distance_sensor.h>

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define CAGE 4
#define SLOW 5

#define KEY_W 87
#define KEY_A 65
#define KEY_S 83
#define KEY_D 68
#define KEY_F 70
#define KEY_SPACE 32

#define CAGE_RETRACT 0
#define CAGE_DEPLOY 1

/*
 * You may want to add macros here.
 */
#define TIME_STEP 64

const double speed = 10; // [rad/s]
const double acceleration = .3 * 1000 / TIME_STEP; // [rad/s^2]

double speedLeft, speedRight;
double speedLeftCurrent = 0, speedRightCurrent = 0;
int key, cageState = CAGE_RETRACT;
int controlKeys[6];

double robotPosition[2] = {0, 0};
double robotRotation = 0;
double robotSpeed[2] = {0, 0};
int cameraObstacleSensor[4] = {0, 0, 0 ,0};
double compassNorth;

int speedDeltaEnable = 0;
int speedDeltaIndex = 0;
const int speedDeltaMax = 20;
const int speedDeltaThreshold = 5;
double speedDelta[speedDeltaMax][2];

WbDeviceTag compass;
WbDeviceTag distanceSensors[5];

void cageDeploy(int newState)
{
  if(newState == cageState)
  {
    return;
  }
  
  WbDeviceTag cage = wb_robot_get_device("cage");
  
  if(newState == CAGE_DEPLOY)
  {
    wb_motor_set_position(cage, 3.14);
    printf("Deploying cage\n");
    cageState = CAGE_DEPLOY;
  }
  else
  
  {
    wb_motor_set_position(cage, 0);
    printf("Retracting cage\n");
    cageState = CAGE_RETRACT;
  }
}

double get_bearing_in_degrees()
{
  const double *north = wb_compass_get_values(compass);
  double rad = atan2(north[0], north[2]);
  double bearing = (rad - 1.5708) / M_PI * 180.0;
  if(bearing < 0.0)
  {
    bearing = bearing + 360.0;
  }
  return bearing;
}

/*
 * This is the main program.
 * The arguments of the main function can be specified by the
 * "controllerArgs" field of the Robot node
 */
int main(int argc, char **argv)
{
  wb_robot_init();
  
  /* necessary to initialize webots stuff */
  compass = wb_robot_get_device("compass");
  wb_compass_enable(compass, TIME_STEP); 
  
  distanceSensors[0] = wb_robot_get_device("distanceSensor1");
  distanceSensors[1] = wb_robot_get_device("distanceSensor2");
  distanceSensors[2] = wb_robot_get_device("distanceSensor3");
  distanceSensors[3] = wb_robot_get_device("distanceSensor4");
  distanceSensors[4] = wb_robot_get_device("distanceSensor5");
  wb_distance_sensor_enable(distanceSensors[0], TIME_STEP);
  wb_distance_sensor_enable(distanceSensors[1], TIME_STEP);
  wb_distance_sensor_enable(distanceSensors[2], TIME_STEP);
  wb_distance_sensor_enable(distanceSensors[3], TIME_STEP);
  wb_distance_sensor_enable(distanceSensors[4], TIME_STEP);
  
  //accelerometer = wb_robot_get_device("accelerometer");
  //wb_accelerometer_enable(accelerometer, TIME_STEP);
  
  //gyro = wb_robot_get_device("gyro");
  //wb_gyro_enable(gyro, TIME_STEP);
  
  // Enable keyboard interface
  wb_robot_keyboard_enable(TIME_STEP);
  
  /*
   * You should declare here WbDeviceTag variables for storing
   * robot devices like this:
   *  WbDeviceTag my_sensor = wb_robot_get_device("my_sensor");
   *  WbDeviceTag my_actuator = wb_robot_get_device("my_actuator");
   */
   
   // initialize motors
  WbDeviceTag wheels[4];
  char wheels_names[4][8] =
  {
    "wheelFL", "wheelFR", "wheelBL", "wheelBR"
  };
  unsigned int i;
  for(i = 0; i < 4; i++)
  {
    wheels[i] = wb_robot_get_device(wheels_names[i]);
    
    // For velocity control the position must be set to INFINITY
    wb_motor_set_position(wheels[i], INFINITY);
  }
  
  /* main loop
   * Perform simulation steps of TIME_STEP milliseconds
   * and leave the loop when the simulation is over
   */
  while(wb_robot_step(TIME_STEP) != -1)
  {
    
    /* 
     * Read the sensors :
     * Enter here functions to read sensor data, like:
     *  double val = wb_distance_sensor_get_value(my_sensor);
     */
    
    /* Process sensor data here */
    
    // Update compas values
    compassNorth = get_bearing_in_degrees();
    //printf("North: %f\n", compassNorth);
    
    // Get pressed keys and move accordingly
    memset(&controlKeys[0], 0, sizeof(controlKeys));
    while((key = wb_robot_keyboard_get_key()))
    {
      //printf("Key press: %d\n", key);
      if(key == WB_ROBOT_KEYBOARD_UP || key == KEY_W)
      {
        controlKeys[UP] = 1;
      }
      if(key == WB_ROBOT_KEYBOARD_DOWN || key == KEY_S)
      {
        controlKeys[DOWN] = 1;
      }
      if(key == WB_ROBOT_KEYBOARD_LEFT || key == KEY_A)
      {
        controlKeys[LEFT] = 1;
      }
      if(key == WB_ROBOT_KEYBOARD_RIGHT || key == KEY_D)
      {
        controlKeys[RIGHT] = 1;
      }
      if(key == KEY_SPACE)
      {
        controlKeys[CAGE] = 1;
      }
      if(key == KEY_F)
      {
        controlKeys[SLOW] = 1;
      }
    }
    
    if(controlKeys[CAGE])
    {
      // Deploy/retract cage
      int newCageState = cageState == CAGE_RETRACT ? CAGE_DEPLOY : CAGE_RETRACT;
      cageDeploy(newCageState);
    }
    
    double irFarLeft = wb_distance_sensor_get_value(distanceSensors[0]);
    double irLeft = wb_distance_sensor_get_value(distanceSensors[1]);
    double irMiddle = wb_distance_sensor_get_value(distanceSensors[2]);
    double irRight = wb_distance_sensor_get_value(distanceSensors[3]);
    double irFarRight = wb_distance_sensor_get_value(distanceSensors[4]);
    //printf("IR: %f, %f, %f, %f, %f\n", irFarLeft, irLeft, irMiddle, irRight, irFarRight);
    
    // Read "camera sensor" values from 0 to 3 and try to avoid obstacles
    double obstacleThreshold = 0.6;
    if(irFarLeft > obstacleThreshold)
    {
      speedLeft = 0;
      speedRight = -speed;
      //printf("Obstacle left\n");
    }
    else if(irFarRight > obstacleThreshold)
    {
      speedLeft = -speed;
      speedRight = 0;
      //printf("Obstacle right\n");
    }
    else if(irLeft > obstacleThreshold)
    {
      speedLeft = speed;
      speedRight = -speed;
      //printf("Obstacle in front left\n");
    }
    else if(irRight > obstacleThreshold)
    {
      speedLeft = -speed;
      speedRight = speed;
      //printf("Obstacle in front right\n");
    }
    else
    {
      // Simply go forwards
      speedLeft = speed;
      speedRight = speed;
    }
      
    // Detect if robot is stuck
    speedDelta[speedDeltaIndex][0] = speedLeft;
    speedDelta[speedDeltaIndex][1] = speedRight;
    speedDeltaIndex = (speedDeltaIndex + 1) % speedDeltaMax;
    
    // Allow to load the array with values at start
    if(!speedDeltaEnable && speedDeltaIndex == speedDeltaMax - 1)
    {
      speedDeltaEnable = 1;
    }
    
    // Check that the delta isn't around zero with a running average
    int i;
    double averageLeft = 0, averageRight = 0;
    for(i = 0; i < speedDeltaMax; i++)
    {
      averageLeft = (i * averageLeft + speedDelta[i][0]) / (i + 1);
      averageRight = (i * averageRight + speedDelta[i][1]) / (i + 1);
    }
    
    //printf("Delta: %f\n", (averageLeft + averageRight));
    
    if(speedDeltaEnable && averageLeft + averageRight < speedDeltaThreshold)
    {
      //printf("Infinite loop detected\n");
      if(cameraObstacleSensor[0] >= 70 && cameraObstacleSensor[1] >= 70 && cameraObstacleSensor[2] >= 70 && cameraObstacleSensor[3] >= 70 && irLeft < 1.25 && irRight < 1.25)
      {
        speedDeltaIndex = 0;
        speedDeltaEnable = 0;
      }
      else
      {
        speedLeft = speed;
        speedRight = -speed;
      }
    }
    
    // Normalise speed
    /*if(fabs(speedLeft) > fabs(speed))
    {
      speedLeft = speed;
      speedRight = speedRight * (speed / speedLeft);
    }
    else if(fabs(speedRight) > fabs(speed))
    {
      speedRight = speed;
      speedLeft = speedLeft * (speed / speedRight);
    }*/
    
    // Manual control
    if(controlKeys[UP])
    {
      speedLeft = speed;
      speedRight = speed;
    }
    if(controlKeys[DOWN])
    {
      speedLeft = -speed;
      speedRight = -speed;
    }
    if(controlKeys[LEFT])
    {
      speedLeft = -speed;
      speedRight = speed;
    }
    if(controlKeys[RIGHT])
    {
      speedLeft = speed;
      speedRight = -speed;
    }
    
    //speedLeftCurrent = speedLeft;
    //speedRightCurrent = speedRight;
    
    // Wheel speed control with acceleration and deceleration
    if(speedLeft > 0 && speedLeftCurrent < speedLeft && speedLeftCurrent + acceleration <= speedLeft)
    {
      // Left wheels setpoint speed is not yet reached so add a delta acceleration to it
      speedLeftCurrent += acceleration;
    }
    else if(speedLeft > 0 && speedLeftCurrent > speedLeft && speedLeftCurrent - acceleration >= speedLeft)
    {
      // Left wheels setpoint speed is not yet reached so add a delta acceleration to it
      speedLeftCurrent -= acceleration;
    }
    else if(speedLeft < 0 && speedLeftCurrent > speedLeft && speedLeftCurrent - acceleration <= speedLeft)
    {
      // Left wheels setpoint speed is not yet reached so add a delta acceleration to it
      speedLeftCurrent -= acceleration;
    }
    else if(speedLeft < 0 && speedLeftCurrent < speedLeft && speedLeftCurrent + acceleration >= speedLeft)
    {
      // Left wheels setpoint speed is not yet reached so add a delta acceleration to it
      speedLeftCurrent += acceleration;
    }
    else
    {
      speedLeftCurrent = speedLeft;
    }
    
    if(speedRight > 0 && speedRightCurrent < speedRight && speedRightCurrent + acceleration <= speedRight)
    {
      // Left wheels setpoint speed is not yet reached so add a delta acceleration to it
      speedRightCurrent += acceleration;
    }
    else if(speedRight > 0 && speedRightCurrent > speedRight && speedRightCurrent - acceleration >= speedRight)
    {
      // Left wheels setpoint speed is not yet reached so add a delta acceleration to it
      speedRightCurrent -= acceleration;
    }
    else if(speedRight < 0 && speedRightCurrent > speedRight && speedRightCurrent - acceleration <= speedRight)
    {
      // Left wheels setpoint speed is not yet reached so add a delta acceleration to it
      speedRightCurrent -= acceleration;
    }
    else if(speedRight < 0 && speedRightCurrent < speedRight && speedRightCurrent + acceleration >= speedRight)
    {
      // Left wheels setpoint speed is not yet reached so add a delta acceleration to it
      speedRightCurrent += acceleration;
    }
    else
    {
      speedRightCurrent = speedRight;
    }
    
    // Half speed mode
    if(controlKeys[SLOW])
    {
      speedLeftCurrent = speedLeftCurrent / 2;
      speedRightCurrent = speedRightCurrent / 2;
    }
    
    //printf("Speed: left: %f, right: %f\n", speedLeftCurrent, speedRightCurrent);
    
    // Front left
    wb_motor_set_velocity(wheels[0], speedLeftCurrent);
    // Front right
    wb_motor_set_velocity(wheels[1], speedRightCurrent);
    // Back left
    wb_motor_set_velocity(wheels[2], speedLeftCurrent);
    // Back right
    wb_motor_set_velocity(wheels[3], speedRightCurrent);
  };
  
  /* Enter your cleanup code here */
  
  /* This is necessary to cleanup webots resources */
  wb_robot_cleanup();
  
  return 0;
}
