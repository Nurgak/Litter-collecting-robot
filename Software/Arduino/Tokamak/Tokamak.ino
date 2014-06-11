/***************************************************************************************
 *
 * Title:      STI competition Arduino code for the Tokamak robot.
 * Date:       2014-06-06
 *
 ***************************************************************************************/
#include <Servo.h>
#include <prismino.h>
#include <Wire.h>
#include "robot.h"

Tokamak robot;

// transition structure
struct transition
{
    enum state_codes state_source;
    enum return_codes return_code;
    enum state_codes state_destination;
};

// state functions and codes must be in sync
return_codes (*state[])(void) = {
  stateSearching,
  stateFetchingBottle,
  stateLowerCage,
  stateGoHome,
  stateRaiseCage
};

struct transition state_transitions[] = {
  {STATE_SEARCHING,       OK,     STATE_FETCHING_BOTTLE},
  {STATE_SEARCHING,       REPEAT, STATE_SEARCHING},
  {STATE_FETCHING_BOTTLE, OK,     STATE_LOWER_CAGE},
  {STATE_FETCHING_BOTTLE, REPEAT, STATE_FETCHING_BOTTLE},
  {STATE_FETCHING_BOTTLE, FAIL,   STATE_SEARCHING},
  {STATE_LOWER_CAGE,      OK,     STATE_GO_HOME},
  {STATE_GO_HOME,         REPEAT, STATE_GO_HOME},
  {STATE_GO_HOME,         OK,     STATE_RAISE_CAGE},
  {STATE_RAISE_CAGE,      OK,     STATE_SEARCHING}
};

enum state_codes currentState;
enum return_codes returnCode;
comm_methods inputMethod;

// pointer to the current called function in the state machine
return_codes (*stateFunction)(void);

// other global variables
volatile uint8_t bottlePosition;
volatile uint8_t bottleDistance;
sides sideWall;

volatile uint32_t timeBottleLastSeen;
volatile uint32_t timeNextBottleCheck;
uint32_t timeCheckBattery;
volatile boolean booleanBottleUpdate;
boolean booleanBottleSecondTry;
boolean booleanFalseBottle;

// ############################################################ SETUP

void setup()
{
  // set pin output mode (sources current)
  pinMode(LED, OUTPUT);
  pinMode(PIN_LIGHTS, OUTPUT);
  
  // enable button pull-up
  pinMode(BTN, INPUT);
  digitalWrite(BTN, HIGH);

  // play a sound on boot, repeated sounds will indicate very low battery voltage
  robot.playSound(ONEUP);

  // initialise serial bus for communication with the Raspberry Pi
  Serial.begin(9600);

  // initialise the bus for communication with the computer via Bluetooth
  #ifdef ENABLE_BLUETOOTH
  Bluetooth.begin(9600);
  #endif

  // join i2c bus as master
  Wire.begin();
  // disable internal pull-ups to 5V as there are external 2K pull-ups to 3.3V
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  
  #ifdef ENABLE_COMPASS
  // wait for the imu to boot
  delay(500);
  // put the HMC5883 IC into the correct operating mode
  // open communication with HMC5883
  Wire.beginTransmission(I2C_COMPASS_ADDRESS);
  // select mode register
  Wire.write(0x02);
  // continuous measurement mode
  Wire.write(0x00);
  Wire.endTransmission();
  #endif
  
  // initialise global variables
  bottlePosition = 0;
  bottleDistance = 0;
  
  timeCheckBattery = millis() + TIME_CHECK_BATTERY;
  timeBottleLastSeen = 0;
  
  sideWall = RIGHT;
  booleanBottleUpdate = 0;
  
  // reset the robot state
  reset();
  
  currentState = ENTRY_STATE;
}

// ############################################################ MAIN LOOP

void loop()
{
  if(Serial.available())
  {
    processInput(USB);
  }
  
  #ifdef ENABLE_BLUETOOTH
  if(Bluetooth.available())
  { 
    processInput(BLUETOOTH);
  }
  #endif
  
  uint16_t irLeft, irCenterLeft, irCenterRight, irRight;
  robot.readIrSensors(&irLeft, &irCenterLeft, &irCenterRight, &irRight);
  
  Bluetooth.print(irLeft);
  Bluetooth.print("\t");
  Bluetooth.print(irCenterLeft);
  Bluetooth.print("\t");
  Bluetooth.print(irCenterRight);
  Bluetooth.print("\t");
  Bluetooth.println(irRight);
  
  //Bluetooth.println(robot.getHeading());
  
  // toggle robot running state via the button on the shield
  if(!digitalRead(BTN))
  {
    robot.playSound(POWERUP);
    robot.flags.running = !robot.flags.running;
    
    // robot has been stopped
    if(!robot.flags.running)
    {
      robot.stop();
      // reset robot state to ENTRY_STATE
      currentState = ENTRY_STATE;
    }
    
    // wait for button debounce
    delay(1000);
  }

  #ifdef ENABLE_CONTROLLER
  // check battery level and make a beep if it's too low
  if(millis() > timeCheckBattery)
  {
    robot.checkBattery();
  }
  #endif
  
  if(robot.flags.running)
  {
    // current function to call according to the state machine
    stateFunction = state[currentState];
    // actually call the function
    returnCode = stateFunction();
    // fetch next state
    currentState = lookupTransitions(currentState, returnCode);
  }
}

// ############################################################ STATE TRANSITIONS

// returns the new state according to the current state and the return value
state_codes lookupTransitions(state_codes state, return_codes code)
{
  uint8_t i;
  // default return state is the entry state
  state_codes nextState = ENTRY_STATE;
  // see if a state transition matches and switch to the next state
  for(i = 0; i < sizeof(state_transitions) / sizeof(transition); i++)
  {
    if(state_transitions[i].state_source == state && state_transitions[i].return_code == code)
    {
      nextState = state_transitions[i].state_destination;
      if(nextState != state_transitions[i].state_source)
      {
        robot.playSound(COIN);
      }
      break;
    }
  }
  return nextState;
}

// ############################################################ ROBOT STATES

return_codes stateSearching()
{
  // check if a bottle has been seen and change state
  if(bottlePosition && millis() > timeNextBottleCheck)
  {
    robot.stop();
    booleanBottleSecondTry = 0;
    booleanFalseBottle = 1;
    return OK;
  }
  
  // make robot roam the arena set deviation to 0 to go straight when there are no obstacles
  robot.headTo(0);
  
  return REPEAT;
}

return_codes stateFetchingBottle()
{
  // check that the last time a bottle has been seen doesn't exceed a limit
  if(millis() > timeBottleLastSeen)
  {
    timeNextBottleCheck = millis() + TIME_NEXT_BOTTLE_CHECK;
    
    // a bottle hasn't been seed since TIME_BOTTLE_SEEN_TIMEOUT milliseconds, it was probably a false positive
    if(booleanBottleSecondTry)
    {
      bottlePosition = 0;
      return FAIL;
    }
    
    // back up a little bit just to be sure it was a false positive
    booleanBottleSecondTry = 1;
    
    robot.setSpeed(-CONST_SPEED_BOTTLE, -CONST_SPEED_BOTTLE);
    delay(500);
    robot.stop();
    delay(1000);
  }
  
  // make the 0-255 bottle position value signed
  int8_t deviation = 127 - bottlePosition;
  static uint32_t timeBottleApproachingLastCheck = 0;
  
  if(deviation > -CONST_DEVIATION_OK && deviation < CONST_DEVIATION_OK)
  {
    // if the bottle is close enough or it was lost while approaching from it (the bottle is right next to the robot), lower the cage
    if(bottleDistance > CONST_BOTTLE_SIZE_LOWER_CAGE || millis() > timeBottleApproachingLastCheck)
    {
      // if the bottle magically appeared in front of the robot consider it a false positive
      if(booleanFalseBottle)
      {
        bottlePosition = 0;
        return FAIL;
      }
    
      // just to be sure move forwards for a little while
      robot.setSpeed(CONST_SPEED_BOTTLE, CONST_SPEED_BOTTLE);
      delay(500);
      robot.stop();
      
      return OK;
    }
    
    // approach the bottle only if the robot got an update on the bottle position
    if(booleanBottleUpdate)
    {
      robot.setSpeed(CONST_SPEED_BOTTLE, CONST_SPEED_BOTTLE);
      booleanBottleUpdate = 0;
      timeBottleApproachingLastCheck = millis() + TIME_GRAB_LOST_BOTTLE;
    }
  }
  
  // head towards the bottle at a lower speed
  if(booleanBottleUpdate)
  {
    robot.turn(deviation, CONST_SPEED_BOTTLE);
    delay(CONST_SPEED_SET_DELAY);
    robot.stop();
    booleanBottleUpdate = 0;
    timeBottleApproachingLastCheck = millis() + TIME_GRAB_LOST_BOTTLE;
  }
  
  // the bottle was not perfectly detected in front of the robot the first time, so it's probably a true bottle
  booleanFalseBottle = 0;
  
  return REPEAT;
}

return_codes stateLowerCage()
{
  // make sure the wheels are stopped
  robot.stop();
  robot.setCagePosition(CAGE_DOWN);
  robot.playSound(ONEUP);
  return OK;
}

/*return_codes stateFindWall()
{
  uint16_t irLeft, irCenterLeft, irCenterRight, irRight;
  robot.readIrSensors(&irLeft, &irCenterLeft, &irCenterRight, &irRight);
  
  // get the heading angle between -180 and 180 degrees
  int16_t deviation = robot.getHeading() - CONST_HEADING_HOME;
  
  // must check of it will enter an infinite loop
  static uint32_t timeAntiRecheck = 0;
  static uint32_t timeTurnTime;
  
  if(
    millis() > timeAntiRecheck &&
    (irLeft > CONST_IR_OBSTACLE_SIDE_CAGE || irRight > CONST_IR_OBSTACLE_SIDE_CAGE)
  )
  {
    // if the robot is too close to a wall it's impossible for it to be a wall
    if(irLeft > CONST_IR_TOO_CLOSE || irRight > CONST_IR_TOO_CLOSE)
    {
      timeAntiRecheck = millis() + TIME_WALL_RECHECK;
      return REPEAT;
    }
    
    // see if this is an obstacle or a wall
    // the obstacle was on the left, turn left and check if the robot can see it with its right side sensor, if at this point the left is still detecting the obstacle it's a wall
    if(irLeft > irRight)
    {
      // the "obstacle" was on the left, turn towards the left to check for a wall
      sideWall = LEFT;
      timeTurnTime = millis();
      robot.setSpeed(-CONST_SPEED_OBSTACLE, CONST_SPEED_OBSTACLE);
      
      // wait until the right sensor sees the obstacle
      while(analogRead(SENSOR_IR_RIGHT) < CONST_IR_OBSTACLE_SIDE_CAGE);
      
      // stop before checking the other IR sensor
      timeTurnTime = millis() - timeTurnTime;
      robot.setSpeed(0, 0);
      
      // check if the left sensor still sees the obstacle, if yes it's a wall
      if(analogRead(SENSOR_IR_LEFT) > CONST_IR_OBSTACLE_FAR)
      {
        return OK;
      }
    }
    // the obstacle was on the right, turn right and check if the robot can see it with its left side sensor, if at this point the right is still detecting the obstacle it's a wall
    else
    {
      // the "obstacle" was on the right, turn towards the right to check for a wall
      sideWall = RIGHT;
      timeTurnTime = millis();
      robot.setSpeed(CONST_SPEED_OBSTACLE, -CONST_SPEED_OBSTACLE);
      
      // wait until the right sensor sees the obstacle
      while(analogRead(SENSOR_IR_LEFT) < CONST_IR_OBSTACLE_SIDE_CAGE);
      
      // stop before checking the other IR sensor
      timeTurnTime = millis() - timeTurnTime;
      robot.setSpeed(0, 0);
      
      // check if the left sensor still sees the obstacle, if yes it's a wall
      if(analogRead(SENSOR_IR_RIGHT) > CONST_IR_OBSTACLE_FAR)
      {
        return OK;
      }
    }
    
    // false positive, turn back
    if(sideWall == LEFT)
    {
      robot.setSpeed(CONST_SPEED_OBSTACLE, -CONST_SPEED_OBSTACLE);
    }
    else
    {
      robot.setSpeed(-CONST_SPEED_OBSTACLE, CONST_SPEED_OBSTACLE);
    }
    delay(timeTurnTime);
    
    timeAntiRecheck = millis() + TIME_WALL_RECHECK;
  }
  
  // head towards the direction the compass indicates
  robot.headTo(deviation);
  
  return REPEAT;
}*/

return_codes stateGoHome()
{
  return robot.goHome();
  
  /*uint16_t irLeft, irCenterLeft, irCenterRight, irRight;
  robot.readIrSensors(&irLeft, &irCenterLeft, &irCenterRight, &irRight);
  
  int16_t deviation = robot.getHeading() - CONST_HEADING_HOME;
  
  // at this point we know the robot is in front of the wall
  
  // turn towards the home heading within a margin
  Serial.println(deviation);
  
  if(deviation > CONST_DEVIATION_OK)
  {
    robot.setSpeed(-CONST_SPEED_BOTTLE, CONST_SPEED_BOTTLE);
  }
  else if(deviation < -CONST_DEVIATION_OK)
  {
    robot.setSpeed(CONST_SPEED_BOTTLE, -CONST_SPEED_BOTTLE);
  }
  else if(irLeft > irRight)
  {
    // wall is on the left, turn left a little bit for the next state
    robot.setSpeed(CONST_SPEED_MAX, 0);
    delay(TIME_TURN_FOLLOW_WALL);
    robot.setSpeed(0, 0);
    sideWall = LEFT;
    return OK;
  }
  else if(irRight > irLeft)
  {
    // wall is on the right, turn left a little bit for the next state
    robot.setSpeed(0, CONST_SPEED_MAX);
    delay(TIME_TURN_FOLLOW_WALL);
    robot.setSpeed(0, 0);
    sideWall = RIGHT;
    return OK;
  }
  else
  {
    robot.headTo(deviation);
  }
  
  return REPEAT;*/
}

return_codes stateFollowWall()
{
  uint16_t irLeft, irCenterLeft, irCenterRight, irRight;
  robot.readIrSensors(&irLeft, &irCenterLeft, &irCenterRight, &irRight);
  
  // follow the left wall
  if(sideWall == LEFT)
  {
    // a wall was detected on the right, it can only be the goal
    if(irRight > CONST_IR_OBSTACLE_SIDE_CAGE)
    {
      robot.stop();
      return OK;
    }
    
    // go straight with an offset to the left
    //robot.headTo(CONST_SPEED_MAX - 1);
    robot.followWall(&irLeft, LEFT);
  }
  else
  {
    // a wall was detected on the left, it can only be the goal
    if(irLeft > CONST_IR_OBSTACLE_SIDE_CAGE)
    {
      robot.stop();
      return OK;
    }
    
    // go straight with an offset to the right
    //robot.headTo(-(CONST_SPEED_MAX - 1));
    robot.followWall(&irRight, RIGHT);
  }
  
  return REPEAT;
}

return_codes stateRaiseCage()
{
  robot.setCagePosition(CAGE_UP);
  
  // announce the glorious point it just probably got
  //robot.playSound(FLAGPOLE);

  if(robot.flags.wall == WALL_RIGHT)
  {
    robot.setSpeed(-CONST_SPEED_MAX, -(CONST_SPEED_MAX-10));
  }
  else
  {
    robot.setSpeed(-(CONST_SPEED_MAX-10), -CONST_SPEED_MAX);
  }
  delay(1000);
  
  while(robot.getHeading() < 160 && robot.getHeading() > -160)
  {
    if(robot.flags.wall == WALL_LEFT)
    {
      robot.setSpeed(CONST_SPEED_MAX, -CONST_SPEED_MAX);
    }
    else
    {
      robot.setSpeed(-CONST_SPEED_MAX, CONST_SPEED_MAX);
    }
  }
  
  robot.stop();
  
  // reset the robot state before restarting
  reset();
  
  return OK;
}

void reset()
{
  robot.flags.wall = NO_WALL;
  bottlePosition = 0;
  timeNextBottleCheck = millis() + TIME_NEXT_BOTTLE_CHECK;
  booleanBottleSecondTry = 0;
}

// ############################################################ USER INPUT METHODS

void processInput(comm_methods method)
{
  inputMethod = method;
  digitalWrite(LED, HIGH);
  switch(input())
  {
  case '0':
    // force robot state to searching
    output("Toggle robot\n");
    bottlePosition = 0;
    robot.flags.running = !robot.flags.running;
    
    if(!robot.flags.running)
    {
      robot.stop();
    }
    robot.playSound(POWERUP);
    break;
  case '1':
    output("Set state: searching\n");
    currentState = STATE_SEARCHING;
    break;
  case '2':
    output("Set state: fetching bottle\n");
    currentState = STATE_FETCHING_BOTTLE;
    break;
  case '3':
    output("Set state: go home\n");
    currentState = STATE_LOWER_CAGE;
    break;
  case '4':
    output("Set state: follow wall\n");
    currentState = STATE_GO_HOME;
    break;
  case '5':
    output("Set state: raise cage\n");
    currentState = STATE_RAISE_CAGE;
    break;
  case 'B':
    // a bottle was seen
    bottlePosition = input();
    bottleDistance = input();
    timeBottleLastSeen = millis() + TIME_BOTTLE_SEEN_TIMEOUT;
    booleanBottleUpdate = 1;
    break;
  case 'S':
    robot.setSpeed(input(), input());
    break;
  case 't' :
    play((input() << 8) | input(), 500);
    break;
  case '.' :
    output("Lower cage\n");
    robot.setCagePosition(CAGE_DOWN);
    break;
  case ',' :
    output("Raise cage\n");
    robot.setCagePosition(CAGE_UP);
    break;
  case 'm' :
    output("Turn lights on\n");
    robot.setLights(true);
    break;
  case 'n' :
    output("Turn lights off\n");
    robot.setLights(false);
    break;
  case 'w':
    output("Go forwards\n");
    robot.setSpeed(CONST_SPEED_MAX, CONST_SPEED_MAX);
    break;
  case 'a':
    output("Go left\n");
    robot.setSpeed(-CONST_SPEED_MAX, CONST_SPEED_MAX);
    break;
  case 's':
    output("Go backwards\n");
    robot.setSpeed(-CONST_SPEED_MAX, -CONST_SPEED_MAX);
    break;
  case 'd':
    output("Go right\n");
    robot.setSpeed(CONST_SPEED_MAX, -CONST_SPEED_MAX);
    break;
  case 'q':
    output("Stop\n");
    robot.stop();
    break;
  default:
    output("Command not recognised\n");
  }
  digitalWrite(LED, LOW);
}

char input()
{
  if(inputMethod == USB)
  {
    return Serial.read();
  }
  else if(inputMethod == BLUETOOTH)
  {
    return Bluetooth.read();
  }
}

char output(const char* data)
{
  if(inputMethod == USB)
  {
    Serial.print(data);
  }
  else if(inputMethod == BLUETOOTH)
  {
    Bluetooth.print(data);
  }
}

