/*
 * File:          pix.c
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
#include <webots/camera.h>
#include <webots/display.h>
#include <webots/accelerometer.h>
#include <webots/gyro.h>
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

int bottle[4] = {-1, -1, -1, -1};
int bottlePosition;

WbDeviceTag camera;
WbDeviceTag display;
WbDeviceTag accelerometer;
WbDeviceTag gyro;
WbDeviceTag compass;
WbDeviceTag distanceSensors[4];

unsigned int displayData[128][64];

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

void processCameraImage(WbDeviceTag cam)
{
  const unsigned char *image = wb_camera_get_image(cam);
  
  //int cameraWidth = wb_camera_get_width(cam);
  //int cameraHeight = wb_camera_get_height(cam);
  
  // get center pixel color levels
  
  /*const unsigned char *center_pixel = image + (32 * 128 + 64) * 4;
   
  unsigned char r = (*(center_pixel + 2)) & 0xff;
  unsigned char g = (*(center_pixel + 1)) & 0xff;
  unsigned char b = (*(center_pixel + 0)) & 0xff;
  
  printf("R: %d, G: %d, B: %d\n", r, g, b);
  
  unsigned long color = (r << 16) | (g << 8) | b;
  wb_display_set_color(display, color);
  
  printf("Color: %lx\n", color);
  
  for(int x = 0; x < 128; x++)
  {
    for(int y = 0; y < 64; y++)
    {
      wb_display_draw_pixel(display, x, y);
    }
  }*/
  
  /*const unsigned char *openTerrainPixel = image + (32 * 128 + 64) * 4;
  
  unsigned char ot_r = *(openTerrainPixel + 2);
  unsigned char ot_g = *(openTerrainPixel + 1);
  unsigned char ot_b = *(openTerrainPixel + 0);
  
  unsigned long frontPixel = (ot_r << 16) | (ot_g << 8) | ot_b;
  printf("Center pixel: %lx\n", frontPixel);*/
  
  const unsigned int colorMargin = 30;
  
  const unsigned char *pixel;
  unsigned char r, g, b;
  for(int x = 0; x < 128; x++)
  {
    for(int y = 0; y < 64; y++)
    {
      pixel = image + (((y << 7) + x) << 2);
      
      r = *(pixel + 2);
      g = *(pixel + 1);
      b = *(pixel + 0);
      
      if(r == 0xff && g == 0xff && b == 0)
      {
        // Detect yellow beacon
        //wb_display_set_color(display, 0xffff00);
        //wb_display_draw_pixel(display, x, y);
        displayData[x][y] = 0xffff00;
      }
      else if(r == g && r == b)
      {
        // Gray is an obstacle by default (r = b = b)
        //wb_display_set_color(display, 0xff0000);
        //wb_display_draw_pixel(display, x, y);
        displayData[x][y] = 0xff0000;
      }
      else if(
        r > 0xb4 - colorMargin && r < 0xb4 + colorMargin &&
        g > 0x6f - colorMargin && g < 0x6f + colorMargin &&
        b > 0x4b - colorMargin && b < 0x4b + colorMargin
      )
      {
        // Detect walls
        //wb_display_set_color(display, 0xff0000);
        //wb_display_draw_pixel(display, x, y);
        //displayData[x][y] = 0xff0000;
        displayData[x][y] = 0x00ff00;
      }
      else if(
        r > 0x9b - colorMargin && r < 0x9b + colorMargin &&
        g > 0x86 - colorMargin && g < 0x86 + colorMargin &&
        b > 0x70 - colorMargin && b < 0x70 + colorMargin
      )
      {
        // General center area (0x9b8670)
        //wb_display_set_color(display, 0x00ff00);
        //wb_display_draw_pixel(display, x, y);
        displayData[x][y] = 0x00ff00;
      }
      else if(
        r > 0x62 - colorMargin && r < 0x62 + colorMargin &&
        g > 0x93 - colorMargin && g < 0x93 + colorMargin &&
        b > 0x4a - colorMargin && b < 0xa4 + colorMargin
      )
      {
        // 50% point area (0x62934a)
        //wb_display_set_color(display, 0xff9933);
        //wb_display_draw_pixel(display, x, y);
        displayData[x][y] = 0xff9933;
      }
      else if(
        r > 0x97 - colorMargin && r < 0x97 + colorMargin &&
        g > 0x8f - colorMargin && g < 0x8f + colorMargin &&
        b > 0x3c - colorMargin && b < 0x3c + colorMargin
      )
      {
        // 100% point area (0x978f3c)
        //wb_display_set_color(display, 0xffff00);
        //wb_display_draw_pixel(display, x, y);
        displayData[x][y] = 0xffff00;
      }
      else if(
        r >= 0xff - colorMargin && r <= 0xff &&
        g >= 0 && g <= colorMargin * 2 &&
        b >= 0xff - colorMargin && b <= 0xff
      )
      {
        // Color bottles (0xff00ff)
        //wb_display_set_color(display, 0x00ff00);
        //wb_display_draw_pixel(display, x, y);
        displayData[x][y] = 0xff00ff;
      }
      else
      {
        // Void, color in black
        //wb_display_set_color(display, 0x000000);
        //wb_display_draw_pixel(display, x, y);
        displayData[x][y] = 0x000000;
      }
    }
  }
  
  // Draw the processed image
  for(int x = 0; x < 128; x++)
  {
    for(int y = 0; y < 64; y++)
    {
      wb_display_set_color(display, displayData[x][y]);
      wb_display_draw_pixel(display, x, y);
    }
  }
  
  // Find accessible regions from the 4 lower pixel lines
  unsigned char averageVertical[3] = {0, 0, 0};
  // 4 regions
  unsigned char averageHorizontal[4][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
  int pixelLine = cageState == CAGE_RETRACT ? 63 : 31;
  for(int x = 0, xx = 0; x < 128; x++, xx = x >> 5)
  {
    for(int y = pixelLine, i = 0; y < pixelLine + 1; y++, i++)
    {
      // Running average of vertical pixel lines
      /*pixel = image + (((y << 7) + x) << 2);
      
      r = *(pixel + 2);
      g = *(pixel + 1);
      b = *(pixel + 0);*/
      
      r = displayData[x][y] >> 16;
      g = (displayData[x][y] >> 8) & 0xff;
      b = displayData[x][y] & 0xff;
      
      //printf("Average before: %x, %x, %x\n", r, g, b);
      averageVertical[0] = (averageVertical[0] * i + r) / (i + 1);
      averageVertical[1] = (averageVertical[1] * i + g) / (i + 1);
      averageVertical[2] = (averageVertical[2] * i + b) / (i + 1);
    }
    
    //printf("Vertical average: %x\n", (averageVertical[0] << 16) | (averageVertical[1] << 8) | averageVertical[2]);
    
    averageHorizontal[xx][0] = (averageHorizontal[xx][0] * (x % 32) + averageVertical[0]) / ((x % 32) + 1);
    averageHorizontal[xx][1] = (averageHorizontal[xx][1] * (x % 32) + averageVertical[1]) / ((x % 32) + 1);
    averageHorizontal[xx][2] = (averageHorizontal[xx][2] * (x % 32) + averageVertical[2]) / ((x % 32) + 1);
    
    // Reset
    averageVertical[0] = 0;
    averageVertical[1] = 0;
    averageVertical[2] = 0;
  }
    
  // Update the obstacle sensor values
  cameraObstacleSensor[0] = averageHorizontal[0][1];
  cameraObstacleSensor[1] = averageHorizontal[1][1];
  cameraObstacleSensor[2] = averageHorizontal[2][1];
  cameraObstacleSensor[3] = averageHorizontal[3][1];
  
  // Draw accessible zones to display view
  for(int x = 0, i = 0; x < 128; x++, i = x >> 5)
  {
    unsigned int zone = (averageHorizontal[i][0] << 16) | (averageHorizontal[i][1] << 8) | averageHorizontal[i][2];
    //printf("Zone %d: %lx\n", i, zone);
    wb_display_set_color(display, zone);
    for(int y = 0; y < 4; y++)
    {
      wb_display_draw_pixel(display, x, y);
    }
  }
  
  bottle[0] = -1;
  bottle[1] = -1;
  bottle[2] = -1;
  bottle[3] = -1;
  // Detect bottles by finding the purple areas and draw a square around the blobs
  for(int x = 0; x < 128; x++)
  {
    for(int y = 0; y < 64; y++)
    {
      // Find a purle pixel
      if(displayData[x][y] == 0xff00ff)
      {
        // Set the starting x and y positions
        bottle[0] = x;
        bottle[1] = y;
        
        // Find the end in x direction
        for(int xx = x; xx < 128; xx++)
        {
          if(displayData[xx][y] != 0xff00ff)
          {
            bottle[2] = xx;
            break;
          }
        }
        
        // Find the end in y direction
        for(int yy = y; yy < 64; yy++)
        {
          if(displayData[x][yy] != 0xff00ff)
          {
            bottle[3] = yy;
            break;
          }
        }
        //printf("Bottle: %d, %d, %d, %d\n", bottle[0], bottle[1], bottle[2], bottle[3]);
        break;
      }
    }
    
    if(bottle[0] != -1)
    {
      break;
    }
  }
  
  // Draw the outline of the bottle
  if(bottle[0] != -1)
  {
    wb_display_set_color(display, 0xffffff);
    for(int x = bottle[0]; x < bottle[2]; x++)
    {
      wb_display_draw_pixel(display, x, bottle[1]);
      wb_display_draw_pixel(display, x, bottle[3]);
      //displayData[x][bottle[1]] = 0xffffff;
      //displayData[x][bottle[3]] = 0xffffff;
    }
    for(int y = bottle[1]; y < bottle[3]; y++)
    {
      wb_display_draw_pixel(display, bottle[0], y);
      wb_display_draw_pixel(display, bottle[2], y);
      //displayData[bottle[0]][y] = 0xffffff;
      //displayData[bottle[2]][y] = 0xffffff;
    }
    
    // Inform the robot of the horizontal position of the bottle
    if(bottle[2] - bottle[0] > 5)
    {
      bottlePosition = (bottle[0] + bottle[2]) / 2;
    }
  }
  else
  {
    bottlePosition = -1;
  }
}

void getPosition()
{
  const double *acc = wb_accelerometer_get_values(accelerometer);
  const double *gyr = wb_gyro_get_values(gyro);
  
  robotRotation += gyr[1] * TIME_STEP / 1000;
  
  robotSpeed[0] += acc[0] * TIME_STEP / 1000 * cos(robotRotation);
  robotSpeed[1] += acc[1] * TIME_STEP / 1000 * sin(robotRotation);
  
  robotPosition[0] += robotSpeed[0] * TIME_STEP / 1000;
  robotPosition[1] += robotSpeed[1] * TIME_STEP / 1000;
  
  printf("Acceleration: %f, %f\n", acc[0], acc[2]);
  printf("Speed: %f, %f\n", robotSpeed[0], robotSpeed[1]);
  printf("Position: %f, %f\n", robotPosition[0], robotPosition[1]);
  printf("Gyroscope: %f\n", gyr[1]);
  printf("Rotation: %f\n", robotRotation);
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
  /* necessary to initialize webots stuff */
  wb_robot_init();
  camera = wb_robot_get_device("camera");
  wb_camera_enable(camera, TIME_STEP);
  
  display = wb_robot_get_device("display");
  
  compass = wb_robot_get_device("compass");
  wb_compass_enable(compass, TIME_STEP); 
  
  distanceSensors[0] = wb_robot_get_device("distanceSensor1");
  distanceSensors[1] = wb_robot_get_device("distanceSensor2");
  wb_distance_sensor_enable(distanceSensors[0], TIME_STEP);
  wb_distance_sensor_enable(distanceSensors[1], TIME_STEP);
  
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
    processCameraImage(camera);
    
    //getPosition();
    
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
    
    double irLeft = wb_distance_sensor_get_value(distanceSensors[0]);
    double irRight = wb_distance_sensor_get_value(distanceSensors[1]);
    //printf("Left: %f, right: %f\n", irLeft, irRight);
    
    // Read "camera sensor" values from 0 to 3 and try to avoid obstacles
    if(cameraObstacleSensor[1] < 70 || irLeft > 1.25)
    {
      speedLeft = 0;
      speedRight = -speed;
      //printf("Obstacle in front left\n");
    }
    else if(cameraObstacleSensor[2] < 70 || irRight > 1.25)
    {
      speedLeft = -speed;
      speedRight = 0;
      //printf("Obstacle in front right\n");
    }
    else if(cameraObstacleSensor[0] < 70)
    {
      speedLeft = speed;
      speedRight = -speed;
      //printf("Obstacle in left\n");
    }
    else if(cameraObstacleSensor[3] < 70)
    {
      speedLeft = -speed;
      speedRight = speed;
      //printf("Obstacle in right\n");
    }
    else
    {
      if(bottlePosition!= -1 && cageState == CAGE_RETRACT)
      {
        // A bottle has been seen, go towards it
        if(bottlePosition < 64 - 20)
        {
          // Bottle on the left
          speedLeft = 0;
          speedRight = speed * (64 - bottlePosition) / 64;
        }
        else if(bottlePosition > 64 + 20)
        {
          // Bottle on the right
          speedLeft = speed * (bottlePosition - 64) / 64;
          speedRight = 0;
        }
        else if(bottle[2] - bottle[0] > 10)
        {
          cageDeploy(CAGE_DEPLOY);
        }
        else
        {
          // Bottle on the right
          speedLeft = speed;
          speedRight = speed;
        }
      }
      else
      {
        // Simply go forwards
        speedLeft = speed;
        speedRight = speed;
      }
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
