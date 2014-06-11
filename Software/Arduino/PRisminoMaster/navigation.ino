void navigation_avoidance(int16_t error)
{
  uint16_t irLeft, irRight, irFarLeft, irFarRight;
  int8_t speedL,speedR;
  static int8_t countL,countR;
  uint16_t threshold_center,threshold_side;

  if(cagePos == CAGE_UP)
  {
    threshold_center = CONST_IR_OBSTACLE_CENTER;
    threshold_side = CONST_IR_OBSTACLE_SIDE;
  }
  else
  {
    threshold_center = CONST_IR_OBSTACLE_CENTER_CAGE_DOWN;
    threshold_side = CONST_IR_OBSTACLE_SIDE_CAGE_DOWN;
  }
  
  irFarLeft = getAnalogSensor(IR_FAR_LEFT);
  irFarRight = getAnalogSensor(IR_FAR_RIGHT);
  irLeft = getAnalogSensor(IR_LEFT);
  irRight = getAnalogSensor(IR_RIGHT);
  


  if(millis() > timeAntiLoop)
  {
    // every 3 seconds reset the anti-loop timer and wheel counters
    countL = 0;
    countR = 0;
    timeAntiLoop = millis() + TIME_ANTI_LOOP;
  }
  if(countL > 3 && countR > 3) // this condition is to not get stuck
  {
    
    if (cagePos == CAGE_DOWN) //reverse
    {
      speedL = -CONST_SPEED_OBSTACLE;
      speedR = -CONST_SPEED_OBSTACLE;
      setMotorSpeed(speedL, speedR);
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
    setMotorSpeed(speedL, speedR);
    // turn for 1 second
    delay(1000);
    // reset the counters
    countL = 0;
    countR = 0;
  }
  // avoid obstacles using the IR sensors
  else if(irRight > threshold_center)
  {
    // turn on itself
    speedL = CONST_SPEED_OBSTACLE;
    speedR = -CONST_SPEED_OBSTACLE;
    countL++;
  }
  else if(irLeft > threshold_center)
  {
    speedL = -CONST_SPEED_OBSTACLE;
    speedR = CONST_SPEED_OBSTACLE;
    countR++;
  }
  else if(irFarRight > threshold_side)
  {
    // block one wheel, reverse the other
    speedL = -CONST_SPEED_OBSTACLE;
    speedR = 0;
    countL++;
  }
  else if(irFarLeft > threshold_side)
  {
    speedL = 0;
    speedR = -CONST_SPEED_OBSTACLE;
    countR++;
  }
  else
  {
    navigation_follow_heading(error, speedL, speedR);

  }

  setMotorSpeed(speedL, speedR);
  
  delay(50);
}



void navigation_follow_heading(int16_t error, int8_t & speedL, int8_t & speedR)
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
      speedL = CONST_SPEED_OBSTACLE;
      speedR =(100-abs(error));//* CONST_SPEED_OBSTACLE / 100 );
      //Serial.write("turn right\t");
    }
    else
    {
      speedL = (100-abs(error)); //* CONST_SPEED_OBSTACLE / 100 );
      speedR = CONST_SPEED_OBSTACLE;
      //Serial.write("turn left\t");
    }
  }
// Serial.print(speedL);
// Serial.print("\t");
// Serial.print(speedR);
// Serial.print("\t");
// Serial.println("\t");
}





void setMotorSpeed(int8_t speedLeft, int8_t speedRight)
{
  Wire.beginTransmission(I2C_MOTOR_CONTROLLER_ADDRESS);
  Wire.write("s");

  // make sure not to go over the maximum speed limit
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
}
