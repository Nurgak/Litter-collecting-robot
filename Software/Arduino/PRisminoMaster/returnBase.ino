// FUNCTIONS TO RETURN TO RECYCLING AREA
#define NO_WALL 0
#define WALL_LEFT 1
#define WALL_RIGHT 2
static uint8_t wall_detected = NO_WALL;

void resetReturnBase(void)
{
  wall_detected = 0;
}

void goHome(int16_t direction_home)
{
  //tell RASP to stop looking for bottles -> accelerates image processing
  //tell RASP to start looking for beacon

  //getHeading();
  //gotoHeading(direction_home);
  uint16_t irFarLeft = getAnalogSensor(IR_FAR_LEFT);
  uint16_t irFarRight = getAnalogSensor(IR_FAR_RIGHT);
  
  if(wall_detected == NO_WALL)
  {
    wall_detected = detectWall();
    gotoHeading(direction_home);
    if(wall_detected != NO_WALL) 
    {
      Serial.println("wall detected");
      setMotorSpeed(0,0);
    }
  }
  else 
  {
    if(wall_detected == WALL_LEFT) 
    {
//      Serial.println("WALL LEFT");
      if(irFarRight > CONST_IR_OBSTACLE_SIDE_CAGE_DOWN)
      {
        setMotorSpeed(0,0);
        play(440, 500);
      }
      else navigation_avoidance(-6);
    }
    else if(wall_detected == WALL_RIGHT)
    {
//      Serial.println("WALL RIGHT");
      if(irFarLeft > CONST_IR_OBSTACLE_SIDE_CAGE_DOWN)
      {
        setMotorSpeed(0,0);
        play(440, 500);
      }
      else navigation_avoidance(6);
    }
  }
  //if(beacon_detected) //stop, open cage, wallSide = 0;
}


uint8_t detectWall(void)
{
  uint16_t irFarLeft = getAnalogSensor(IR_FAR_LEFT);
  uint16_t irFarRight = getAnalogSensor(IR_FAR_RIGHT);
  static uint32_t timeAntiRecheck;

  if((irFarLeft < 100 && irFarRight < CONST_IR_OBSTACLE_SIDE_CAGE_DOWN) || (irFarLeft < CONST_IR_OBSTACLE_SIDE_CAGE_DOWN && irFarRight < 100) ) 
  {
    return NO_WALL; // no wall detected
    timeAntiRecheck = millis() + TIME_WALL_RECHECK;
  }
  else
  {
    if(millis()<timeAntiRecheck) return NO_WALL;
    if (irFarLeft > irFarRight) 
    {
      setMotorSpeed(-100,+100);
      delay(TIME_WALL_CHECK_TURN);
      irFarLeft = getAnalogSensor(IR_FAR_LEFT);
      irFarRight = getAnalogSensor(IR_FAR_RIGHT);
      Serial.print(irFarLeft);
      Serial.print("\t");
      Serial.println(irFarRight);
      delay(50); //wait for IR
      if (irFarLeft > 100 && irFarRight > 100) 
      {
        setMotorSpeed(+100,-100);
        delay(TIME_WALL_CHECK_TURN+500);
        Serial.println("WALL LEFT");
        return WALL_LEFT; // wall detected on the left
      }
      else 
      {
        setMotorSpeed(+100,-100);
        delay(TIME_WALL_CHECK_TURN);
        timeAntiRecheck = millis() + TIME_WALL_RECHECK;
        return NO_WALL;
      }
    }
    else
    {
      setMotorSpeed(+100,-100);
      delay(TIME_WALL_CHECK_TURN);
      irFarLeft = getAnalogSensor(IR_FAR_LEFT);
      irFarRight = getAnalogSensor(IR_FAR_RIGHT);
      Serial.print(irFarLeft);
      Serial.print("\t");
      Serial.println(irFarRight);
      delay(50); //wait for IR
      
      if (irFarLeft > 100 && irFarRight > 100) 
      {
        setMotorSpeed(-100,+100);
        delay(TIME_WALL_CHECK_TURN+500);
        return WALL_RIGHT; // wall detected on the left
      }
      else 
      {
        setMotorSpeed(-100,+100);
        delay(TIME_WALL_CHECK_TURN);
        timeAntiRecheck = millis() + TIME_WALL_RECHECK;
        return NO_WALL;
      }
    }
  }
}

void gotoHeading(int16_t direction_home)
{

  int16_t currentHeading=getHeading();
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

  navigation_avoidance(-error);  
}


int16_t getHeading()
{
  int x=0,y=0,z=0; //triple axis data
  Wire.beginTransmission(I2C_COMPASS_ADDRESS);
  //select register 3, X MSB register
  Wire.write(0x03);
  Wire.endTransmission();

  Wire.requestFrom(I2C_COMPASS_ADDRESS, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8; //X msb
    x |= Wire.read(); //X lsb
    z = Wire.read()<<8; //Z msb
    z |= Wire.read(); //Z lsb
    y = Wire.read()<<8; //Y msb
    y |= Wire.read(); //Y lsb
  }
  
  int16_t angle =atan2(x,z)*180/M_PI;
  return angle;
}

