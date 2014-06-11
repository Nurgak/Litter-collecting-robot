/*
 * File:          supervisor.c
 * Date:          
 * Description:   
 * Author:        
 * Modifications: 
 */

/*
 * You may need to add include files like <webots/distance_sensor.h> or
 * <webots/differential_wheels.h>, etc.
 */
#include <webots/robot.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <webots/supervisor.h>

#define ROBOTS 1
#define BOTTLES 4

WbNodeRef robotRefArray[ROBOTS];
WbNodeRef bottlesRefArray[BOTTLES];
unsigned int points = 0;

// Minimum distance between robot and bottle for it to count as a point
double pointDistance = 1.5; // in meterss

/*
 * You may want to add macros here.
 */
#define TIME_STEP 64

static void setScore()
{
  char score[16];

  sprintf(score, "Points: %d", points);
  wb_supervisor_set_label(0, score, 0.01, 0.01, 0.1, 0x000000, 0);
}

/*
 * Get the bottle points according to difficulty zone
 */
unsigned int getBottlePoints(double x, double z)
{
  if(x < -1 && z < -1)
  {
    return 40;
  }
  else if(x > 1 && z > 1)
  {
    return 20;
  }
  else if(x > 1 && z < -1)
  {
    return 40;
  }
  else
  {
    return 10;
  }
}

void moveBottle(WbNodeRef bottleRef)
{
  double pos[3] = {0, 1, 0};
  unsigned int bottlePoints;
  char pts[5];
  
  pos[0] = (double) (rand() % 64 - 32) / 8;
  pos[2] = (double) (rand() % 64 - 32) / 8;
    
  WbFieldRef translation = wb_supervisor_node_get_field(bottleRef, "translation");
  wb_supervisor_field_set_sf_vec3f(translation, pos);
    
  bottlePoints = getBottlePoints(pos[0], pos[2]);
  sprintf(pts, "%d", bottlePoints);
  WbFieldRef description = wb_supervisor_node_get_field(bottleRef, "description");
  wb_supervisor_field_set_sf_string(description, pts);
}

void initBottles()
{
  char bottleDefines[BOTTLES][8] = {"BOTTLE1", "BOTTLE2", "BOTTLE3", "BOTTLE4"};
  
  for(int i = 0; i < BOTTLES; i++)
  {
    bottlesRefArray[i] = wb_supervisor_node_get_from_def(bottleDefines[i]);
    moveBottle(bottlesRefArray[i]);
  }
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
  
  srand(time(NULL));
  
  /*
   * You should declare here WbDeviceTag variables for storing
   * robot devices like this:
   *  WbDeviceTag my_sensor = wb_robot_get_device("my_sensor");
   *  WbDeviceTag my_actuator = wb_robot_get_device("my_actuator");
   */
  
  setScore();
  initBottles();
  
  char robotDefines[ROBOTS][8] = {"ROBOT1"};
  for(int i = 0; i < ROBOTS; i++)
  {
    robotRefArray[i] = wb_supervisor_node_get_from_def(robotDefines[i]);
  }
  
  /* main loop
   * Perform simulation steps of TIME_STEP milliseconds
   * and leave the loop when the simulation is over
   */
  while (wb_robot_step(TIME_STEP) != -1) {
    
    /* 
     * Read the sensors :
     * Enter here functions to read sensor data, like:
     *  double val = wb_distance_sensor_get_value(my_sensor);
     */
    
    /* Process sensor data here */
    
    // Check if there's a bottle inside the recycling area
    for(int i = 0; i < BOTTLES; i++)
    {
      const double *position = wb_supervisor_node_get_position(bottlesRefArray[i]);
      
      //printf("Bottle %d: x = %f, y = %f\n", i, position[0], position[2]);
      
      // Count point only when robot is far away from the bottle in question
      if(position[0] <= (double) -2 && position[2] >= (double) 2)
      {
        // Point is counted only when the robot is at certain distance away of the bottle
        unsigned char potentialPoint = 1;
        for(int j = 0; j < ROBOTS; j++)
        {
          const double *robot = wb_supervisor_node_get_position(robotRefArray[j]);
          
          double distVect[2] = {robot[0] - position[0], robot[2] - position[2]};
          if(distVect[0] * distVect[0] + distVect[1] * distVect[1] < pointDistance * pointDistance)
          {
            // Too close, dealbreaker
            potentialPoint = 0;
            break;
          }
        }
        
        if(!potentialPoint)
        {
          continue;
        }
        
        WbFieldRef description = wb_supervisor_node_get_field(bottlesRefArray[i], "description");
        int bottlePoints = atoi(wb_supervisor_field_get_sf_string(description));
        
        //printf("Position: %f, %f\n", position[0], position[2]);
        
        if(position[0] < (double) -3 && position[2] > (double) 3)
        {
          printf("Bottle collected: +%d points\n", bottlePoints);
        }
        else
        {
          bottlePoints = bottlePoints / 2;
          printf("Bottle collected in 50%% zone: +%d points\n", bottlePoints);
        }
        
        points = points + bottlePoints;
        setScore();
        
        // Reset bottle position to a random one
        moveBottle(bottlesRefArray[i]);
      }
    }
    
    /*
     * Enter here functions to send actuator commands, like:
     * wb_differential_wheels_set_speed(100.0,100.0);
     */
  };
  
  /* Enter your cleanup code here */
  
  /* This is necessary to cleanup webots resources */
  wb_robot_cleanup();
  
  return 0;
}
