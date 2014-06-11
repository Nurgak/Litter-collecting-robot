/***************************************************************************************
 *
 * Title:      STI competition Arduino code for the Tokamak robot
 * Date:       2014-06-06
 *
 ***************************************************************************************/
#include <Servo.h>
#include <Wire.h>
#include <prismino.h>
#include "robot.h"
#include "pitch.h"
#include "sound.h"

Tokamak::Tokamak()
{
  // initialise default values
  this->flags.enableFrontLeds = false;
  this->flags.running = false;
  this->flags.cagePosition = CAGE_UP;
}

void Tokamak::setCagePosition(cage_positions position)
{
  this->servoRight.attach(S1);
  this->servoLeft.attach(S2);

  uint8_t r, l;

  if(position == CAGE_UP)
  {
    // both sevomotors have 160 steps between up and down positions, so this is allowed
    for(l = SERVO_LEFT_DOWN, r = SERVO_RIGHT_DOWN; r < SERVO_RIGHT_UP; r++, l--)
    {
      this->servoLeft.write(l);
      this->servoRight.write(r);

      delay(CONST_MAX_SERVO_SPEED);
    }
  }
  else if(position == CAGE_DOWN)
  {
    for(l = SERVO_LEFT_UP, r = SERVO_RIGHT_UP; r > SERVO_RIGHT_DOWN; r--, l++)
    {
      this->servoLeft.write(l);
      this->servoRight.write(r);

      delay(CONST_MAX_SERVO_SPEED);
    }
    
  }

  // always detach the motors so that they are free-running and do not consume power in resting positions
  this->servoLeft.detach();
  this->servoRight.detach();
  
  this->flags.cagePosition = position;
}

void Tokamak::setSpeed(int8_t speedLeft, int8_t speedRight)
{
  // controller has been disabled
  #ifndef ENABLE_CONTROLLER
  return;
  #endif
  
  Wire.beginTransmission(I2C_MOTOR_CONTROLLER_ADDRESS);
  Wire.write("s");

  // make sure not to go over the maximum speed limit (100 or -100)
  if(speedLeft > CONST_SPEED_MAX)
  {
    Wire.write(CONST_SPEED_MAX);
  }
  else if(speedLeft < -CONST_SPEED_MAX)
  {
    Wire.write(-CONST_SPEED_MAX);
  }
  else
  {
    Wire.write(speedLeft);
  }

  if(speedRight > CONST_SPEED_MAX)
  {
    Wire.write(CONST_SPEED_MAX);
  }
  else if(speedRight < -CONST_SPEED_MAX)
  {
    Wire.write(-CONST_SPEED_MAX);
  }
  else
  {
    Wire.write(speedRight);
  }

  Wire.endTransmission();
  
  // a small delay is needed so that the speed could actually be applied to the wheels
  delay(CONST_SPEED_SET_DELAY);
}

void Tokamak::stop()
{
  this->setSpeed(0, 0);
}

void Tokamak::checkBattery()
{
  Wire.beginTransmission(I2C_MOTOR_CONTROLLER_ADDRESS);
  
  // casting needed for some reason
  uint8_t available = Wire.requestFrom((uint8_t)I2C_MOTOR_CONTROLLER_ADDRESS, (uint8_t)6);

  // read 6 bytes, 2 by 2, high byte first: voltage, left motor current, right motor current
  if(available == 6)
  {
    this->batteryVoltage = (Wire.read() << 8) | Wire.read();
    this->currentLeft = (Wire.read() << 8) | Wire.read();
    this->currentRight = (Wire.read() << 8) | Wire.read();
  }
  else
  {
    // inform i2c error
    play(TONE_I2C_ERROR, 500);
  }

  Wire.endTransmission();
  
  if(this->batteryVoltage < CONST_BATTERY_LOW)
  {
    play(TONE_BATTERY, 500);
  }
}

void Tokamak::setLights(boolean state)
{
  digitalWrite(PIN_LIGHTS, state);
}

int16_t Tokamak::getHeading()
{
  #ifndef ENABLE_COMPASS
  return 0;
  #endif
  
  int16_t x = 0, y = 0, z = 0;
  
  Wire.beginTransmission(I2C_COMPASS_ADDRESS);
  // select register 3, X MSB register
  Wire.write(0x03);
  Wire.endTransmission();

  Wire.requestFrom(I2C_COMPASS_ADDRESS, 6);
  if(6 <= Wire.available())
  {
    x = (Wire.read() << 8) | Wire.read();
    z = (Wire.read() << 8) | Wire.read();
    y = (Wire.read() << 8) | Wire.read();
  }
  else
  {
    // inform i2c error
    play(TONE_I2C_ERROR, 500);
    return 0;
  }
  
  int16_t angle = atan2(x, z) * 180 / M_PI;
  return angle;
}

void Tokamak::turn(int16_t deviation, int8_t speed)
{
  int8_t speedLeft = 0;
  int8_t speedRight = 0;
  
  // limit deviation to maximum allowed speed
  if(deviation > speed)
  {
    deviation = speed;
  }
  else if(deviation < -speed)
  {
    deviation = -speed;
  }
  
  if(deviation < -CONST_DEVIATION_OK)
  {
    speedLeft = speed;
    speedRight = -speed;
  }
  else if(deviation > CONST_DEVIATION_OK)
  {
    speedLeft = -speed;
    speedRight = speed;
  }
  else
  {
    speedLeft = 0;
    speedRight = 0;
  }
  
  this->setSpeed(speedLeft, speedRight);
}

void Tokamak::headTo(int16_t deviation, int8_t speed)
{
  navigation_avoidance(deviation);
  return;
  
  /*int8_t speedLeft = 0;
  int8_t speedRight = 0;
  
  uint16_t irLeft, irCenterLeft, irCenterRight, irRight;
  this->readIrSensors(&irLeft, &irCenterLeft, &irCenterRight, &irRight);
  
  // these values are incremented over time and reset after every TIME_ANTI_LOOP seconds
  static uint8_t antiLoopCountLeft = 0, antiLoopCountRight = 0;
  static uint32_t timeAntiLoop = 0;
  
  // every TIME_ANTI_LOOP seconds reset the anti-loop timer and wheel counters
  if(millis() > timeAntiLoop)
  {
    // every TIME_ANTI_LOOP seconds reset the anti-loop timer and wheel counters
    antiLoopCountLeft = 0;
    antiLoopCountRight = 0;
    timeAntiLoop = millis() + TIME_ANTI_LOOP;
  }
  
  // limit deviation to maximum allowed speed
  if(deviation > speed)
  {
    deviation = speed;
  }
  else if(deviation < -speed)
  {
    deviation = -speed;
  }
  
  // check if the robot has entered an infinite loop
  if(antiLoopCountLeft > CONST_LOOP_TURN_TIMES && antiLoopCountRight > CONST_LOOP_TURN_TIMES)
  {
    // if the cage is deployed reverse a bit
    if(this->flags.cagePosition == CAGE_UP)
    {
      this->setSpeed(-speed, -speed);
      delay(500);
    }
    
    // set speed immediately
    if(deviation > 0)
    {
      this->setSpeed(speed, -speed);
    }
    else
    {
      this->setSpeed(-speed, speed);
    }
    
    delay(TIME_ANTI_LOOP_TIMEOUT_TURN);
    
    // reset the counters
    antiLoopCountLeft = 0;
    antiLoopCountRight = 0;
  }
  // avoid obstacles using the IR sensors
  else if(irCenterRight > CONST_IR_OBSTACLE_CENTER_CAGE)
  {
    // turn on itself
    speedLeft = speed;
    speedRight = -speed;
    antiLoopCountLeft++;
  }
  else if(irCenterLeft > CONST_IR_OBSTACLE_CENTER_CAGE)
  {
    speedLeft = -speed;
    speedRight = speed;
    antiLoopCountRight++;
  }
  else if(irRight > CONST_IR_OBSTACLE_SIDE_CAGE)
  {
    // block one wheel, reverse the other
    speedLeft = -speed;
    speedRight = 0;
    antiLoopCountLeft++;
  }
  else if(irLeft > CONST_IR_OBSTACLE_SIDE_CAGE)
  {
    speedLeft = 0;
    speedRight = -speed;
    antiLoopCountRight++;
  }
  else if(deviation < -CONST_DEVIATION_OK)
  {
    speedLeft = speed;
    speedRight = speed + deviation;
  }
  else if(deviation > CONST_DEVIATION_OK)
  {
    speedLeft = speed - deviation;
    speedRight = speed;
  }
  else
  {
    // simply go forwards
    speedLeft = speed;
    speedRight = speed;
  }
  
  this->setSpeed(speedLeft, speedRight);*/
}

void Tokamak::followWall(uint16_t *sensorValue, sides side, int8_t speed)
{ 
  if(side == RIGHT && *sensorValue < CONST_IR_WALL_FOLLOW_MIN)
  {
    this->setSpeed(speed, speed >> 2);
  }
  else if(side == RIGHT && *sensorValue > CONST_IR_WALL_FOLLOW_MAX)
  {
    this->setSpeed(speed >> 2, speed);
  }
  else if(side == LEFT && *sensorValue < CONST_IR_WALL_FOLLOW_MIN)
  {
    this->setSpeed(speed >> 2, speed);
  }
  else if(side == LEFT && *sensorValue > CONST_IR_WALL_FOLLOW_MAX)
  {
    this->setSpeed(speed, speed >> 2);
  }
}

void Tokamak::readIrSensors(uint16_t *irLeft, uint16_t *irCenterLeft, uint16_t *irCenterRight, uint16_t *irRight)
{
  *irLeft = analogRead(SENSOR_IR_LEFT);
  *irCenterLeft = analogRead(SENSOR_IR_CENTER_LEFT);
  *irCenterRight = analogRead(SENSOR_IR_CENTER_RIGHT);
  *irRight = analogRead(SENSOR_IR_RIGHT); 
}

// a sound is an array of notes
note notesCoin[] = {
  {B5, 100},
  {E6, 200}
};

sound soundCoin = {sizeof(notesCoin) / sizeof(note), notesCoin};

note notesPowerUp[] = {
  {G3, 50},
  {B4, 50},
  {D4, 50},
  {G4, 50},
  {B5, 50},
  {A4b, 50},
  {C4, 50},
  {E4b, 50},
  {A5b, 50},
  {C5, 50},
  {B4b, 50},
  {D4, 50},
  {F4, 50},
  {B5b, 50},
  {D5, 50}
};

sound soundPowerUp = {sizeof(notesPowerUp) / sizeof(note), notesPowerUp};

note notesOneUp[] = {
  {E4, 100},
  {G4, 100},
  {E5, 100},
  {C5, 100},
  {D5, 100},
  {G5, 100}
};

sound soundOneUp = {sizeof(notesOneUp) / sizeof(note), notesOneUp};

note notesFlagpoleFanfare[] = {
  {G2, 100},
  {C3, 100},
  {E3, 100},
  {G3, 100},
  {C4, 100},
  {E4, 100},
  {G4, 300},
  {E4, 300},
  {A2b, 100},
  {C3, 100},
  {E3b, 100},
  {A3b, 100},
  {C4, 100},
  {E4b, 100},
  {A4b, 300},
  {E4b, 300},
  {B2b, 100},
  {D3, 100},
  {F3, 100},
  {B3b, 100},
  {D4, 100},
  {F4, 100},
  {B4b, 300},
  {B4b, 100},
  {B4b, 100},
  {B4b, 100},
  {C5, 600},
};

sound soundFlagpoleFanfare = {sizeof(notesFlagpoleFanfare) / sizeof(note), notesFlagpoleFanfare};

// function that actually plays the sounds
void Tokamak::playSound(sounds theSound)
{
  #ifndef ENABLE_SOUND
  return;
  #endif
  
  sound soundPtr;
  
  // pointer to the sound object
  switch(theSound)
  {
    case COIN:
      soundPtr = soundCoin;
      break;
    case POWERUP:
      soundPtr = soundPowerUp;
      break;
    case ONEUP:
      soundPtr = soundOneUp;
      break;
    case FLAGPOLE:
      soundPtr = soundFlagpoleFanfare;
      break;
  }
  
  uint16_t frequency, duration;
  uint8_t length = soundPtr.length;
  for(uint8_t i = 0; i < length; i++)
  {
    // get data from program memory
    frequency = soundPtr.notes[i].pitch;
    duration = soundPtr.notes[i].duration;
    
    // if the next note is the same then make a short pause
    uint8_t pause = 0;
    if(i < length - 1 && frequency == soundPtr.notes[i + 1].pitch)
    {
      // 5 millisecond pause
      pause = 5;
    }
    
    // play the right pitch for the determined duration
    play(frequency, duration - pause);
    
    // play is not a blocking function so one has to manually set a delay
    delay(duration);
  }
}

void Tokamak::resetReturnBase(void)
{
  this->flags.wall = NO_WALL;
}

return_codes Tokamak::goHome(int16_t direction_home)
{
  //tell RASP to stop looking for bottles -> accelerates image processing
  //tell RASP to start looking for beacon

  //getHeading();
  //gotoHeading(direction_home);
  uint16_t irFarLeft = analogRead(SENSOR_IR_LEFT);
  uint16_t irFarRight = analogRead(SENSOR_IR_RIGHT);
  
  if(this->flags.wall == NO_WALL)
  {
    this->flags.wall = detectWall();
    gotoHeading(direction_home);
    if(this->flags.wall != NO_WALL) 
    {
      //Serial.println("wall detected");
      this->setSpeed(0,0);
    }
  }
  else 
  {
    if(this->flags.wall == WALL_LEFT) 
    {
//      Serial.println("WALL LEFT");
      if(irFarRight > CONST_IR_OBSTACLE_SIDE_CAGE)
      {
        this->setSpeed(0,0);
        play(440, 500);
        return OK;
      }
      else navigation_avoidance(-6);
    }
    else if(this->flags.wall == WALL_RIGHT)
    {
//      Serial.println("WALL RIGHT");
      if(irFarLeft > CONST_IR_OBSTACLE_SIDE_CAGE)
      {
        this->setSpeed(0,0);
        play(440, 500);
        return OK;
      }
      else navigation_avoidance(6);
    }
  }
  return REPEAT;
  //if(beacon_detected) //stop, open cage, wallSide = 0;
}


uint8_t Tokamak::detectWall(void)
{
  uint16_t irFarLeft = analogRead(SENSOR_IR_LEFT);
  uint16_t irFarRight = analogRead(SENSOR_IR_RIGHT);
  static uint32_t timeAntiRecheck;

  if((irFarLeft < 100 && irFarRight < CONST_IR_OBSTACLE_SIDE_CAGE) || (irFarLeft < CONST_IR_OBSTACLE_SIDE_CAGE && irFarRight < 100) ) 
  {
    return NO_WALL; // no wall detected
    timeAntiRecheck = millis() + TIME_WALL_RECHECK;
  }
  else
  {
    if(millis()<timeAntiRecheck) return NO_WALL;
    if (irFarLeft > irFarRight) 
    {
      if(irFarLeft < 200)
      {
          this->setSpeed(-100,+100);
          delay(TIME_WALL_CHECK_TURN);
          this->stop();
          irFarLeft = analogRead(SENSOR_IR_LEFT);
          irFarRight = analogRead(SENSOR_IR_RIGHT);
          delay(50); //wait for IR
          if (irFarLeft > 100 && irFarRight > 100) 
          {
            this->setSpeed(+100,-100);
            delay(TIME_WALL_CHECK_TURN+500);
            //Serial.println("WALL LEFT");
            return WALL_LEFT; // wall detected on the left
          }
          else 
          {
            this->setSpeed(+100,-100);
            delay(TIME_WALL_CHECK_TURN);
            timeAntiRecheck = millis() + TIME_WALL_RECHECK;
            return NO_WALL;
          }
        }
        else
        {
          //incase the robot wants to turn on itself, but an obstacle is too near and would destroy the deployed cage
          this->setSpeed(-100,-100);
          delay(1000);
        }
      }
    else
    {
      if(irFarRight < 200)
      {
        this->setSpeed(+100,-100);
        delay(TIME_WALL_CHECK_TURN);
        this->stop();
        irFarLeft = analogRead(SENSOR_IR_LEFT);
        irFarRight = analogRead(SENSOR_IR_RIGHT);
        delay(50); //wait for IR
        
        if (irFarLeft > 100 && irFarRight > 100) 
        {
          this->setSpeed(-100,+100);
          delay(TIME_WALL_CHECK_TURN+500);
          return WALL_RIGHT; // wall detected on the left
        }
        else 
        {
          this->setSpeed(-100,+100);
          delay(TIME_WALL_CHECK_TURN);
          timeAntiRecheck = millis() + TIME_WALL_RECHECK;
          return NO_WALL;
        }
      }
      else
      {
        //incase the robot wants to turn on itself, but an obstacle is too near and would destroy the deployed cage
        this->setSpeed(-100,-100);
        delay(1000);
      }
    }
  }
}

void Tokamak::gotoHeading(int16_t direction_home)
{

  int16_t currentHeading = this->getHeading();
  int16_t error;

  error = currentHeading - direction_home;
  
  // in order to avoid the -180 -> 180 jump
  if(error > 180){
    error = error - 360 ;
  }
  else if(error < -180){
    error = error + 360 ;
  }
  
  //reduce error so that the navigation function can use it
  if(error>100) error = 100;
  else if(error<-100) error = -100;

  this->navigation_avoidance(-error);  
}

void Tokamak::navigation_avoidance(int16_t error)
{
  uint16_t irLeft, irRight, irFarLeft, irFarRight;
  int8_t speedL,speedR;
  static int8_t countL,countR;
  
  irFarLeft = analogRead(SENSOR_IR_LEFT);
  irFarRight = analogRead(SENSOR_IR_RIGHT);
  irLeft = analogRead(SENSOR_IR_CENTER_LEFT);
  irRight = analogRead(SENSOR_IR_CENTER_RIGHT);

  static uint32_t timeAntiLoop = 0;

  if(millis() > timeAntiLoop)
  {
    // every 3 seconds reset the anti-loop timer and wheel counters
    countL = 0;
    countR = 0;
    timeAntiLoop = millis() + TIME_ANTI_LOOP;
  }
  if(countL > 3 && countR > 3) // this condition is to not get stuck
  {
    
    if (this->flags.cagePosition == CAGE_DOWN) //reverse
    {
      speedL = -CONST_SPEED_OBSTACLE;
      speedR = -CONST_SPEED_OBSTACLE;
      this->setSpeed(speedL, speedR);
      delay(500);
    }
    if(error>0)
    {
      speedL = CONST_SPEED_OBSTACLE;
      speedR = -CONST_SPEED_OBSTACLE;
    }
    else
     {
      speedL = -CONST_SPEED_OBSTACLE;
      speedR = CONST_SPEED_OBSTACLE;
    }
    // set speed immediately
    this->setSpeed(speedL, speedR);
    // turn for 1 second
    delay(1000);
    // reset the counters
    countL = 0;
    countR = 0;
  }
  // avoid obstacles using the IR sensors
  else if(irRight > CONST_IR_OBSTACLE_CENTER_CAGE)
  {
    // turn on itself
    speedL = CONST_SPEED_OBSTACLE;
    speedR = -CONST_SPEED_OBSTACLE;
    countL++;
  }
  else if(irLeft > CONST_IR_OBSTACLE_CENTER_CAGE)
  {
    speedL = -CONST_SPEED_OBSTACLE;
    speedR = CONST_SPEED_OBSTACLE;
    countR++;
  }
  else if(irFarRight > CONST_IR_OBSTACLE_SIDE_CAGE)
  {
    // block one wheel, reverse the other
    speedL = -CONST_SPEED_OBSTACLE;
    speedR = 0;
    countL++;
  }
  else if(irFarLeft > CONST_IR_OBSTACLE_SIDE_CAGE)
  {
    speedL = 0;
    speedR = -CONST_SPEED_OBSTACLE;
    countR++;
  }
  else
  {
    if( abs(error)<5 )
    {
      // under an error of 5 units go straight
      speedL = CONST_SPEED_MAX;
      speedR = CONST_SPEED_MAX;
    }
    else
    {
      if(error>0)
      {
        if(error == 100 && irFarRight > 100)
        {
            speedL = CONST_SPEED_OBSTACLE;
            speedR = -CONST_SPEED_OBSTACLE;
        }
        else
        {
          speedL = CONST_SPEED_OBSTACLE;
          speedR =(100 - abs(error));//* CONST_SPEED_OBSTACLE / 100 );
        }
        //Serial.write("turn right\t");
      }
      else
      {
         if(abs(error) == 100 && irFarLeft > 100)
        {
            speedL = -CONST_SPEED_OBSTACLE;
            speedR = CONST_SPEED_OBSTACLE;
        }
        else
        {
          speedL = (100-abs(error)); //* CONST_SPEED_OBSTACLE / 100 );
          speedR = CONST_SPEED_OBSTACLE;
        }
        //Serial.write("turn left\t");
      }
    }
  }

  this->setSpeed(speedL, speedR);
}


