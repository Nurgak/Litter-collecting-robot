void serialCommunication()
{
  if(Serial.available())
  {
    digitalWrite(LED, HIGH);
    switch(Serial.read())
    {
    case 'i':
      Serial.println(getAnalogSensor(Serial.read()));
      break;
    case 's':
      setMotorSpeed(Serial.read(), Serial.read());
      break;
    case 'm':
      Serial.println("Go home");
      if(goHomeValue)
      {
        goHomeValue = 0;
        setMotorSpeed(0, 0);
      }
      else
      {
        goHomeValue = 1;
      }
      break;
    case 'b' :
      getPower();
      Serial.println(batteryVoltage);
      break;
    case 't' :
      play((Serial.read() << 8) | Serial.read(), 500);
      break;
    case 'd' :
      setCagePosition(CAGE_DOWN);
      break;
    case 'r':
      Serial.println("Toggle IR dumping");
      enableSerialDumpIrValues = enableSerialDumpIrValues ? 0 : 1;
    case 'l' :
      enableFrontLeds = Serial.read();
      if(enableFrontLeds == 1 || enableFrontLeds == '1')
      {
        digitalWrite(PIN_LIGHTS, HIGH);
      }
      else
      {
        digitalWrite(PIN_LIGHTS, LOW);
      }
      break;
    case 'u' :
      setCagePosition(CAGE_UP);
      break;
    case 'p':
      // read in ip from Raspberry Pi
      ip[0] = Serial.read();
      ip[1] = Serial.read();
      ip[2] = Serial.read();
      ip[3] = Serial.read();
      break;
    case 'a':
      if(enableAutoObstacleAvoidance)
      {
        enableAutoObstacleAvoidance = 0;
        setMotorSpeed(0, 0);
      }
      else
      {
        enableAutoObstacleAvoidance = 1;
      }
      break;
    case 'w':
      setMotorSpeed(CONST_SPEED_MAX, CONST_SPEED_MAX);
      break;
    case 'q':
      setMotorSpeed(0,0);
      break;
    }
    digitalWrite(LED, LOW);
  }
}


void bluetoothCommunication()
{
  if(Bluetooth.available())
  { 
    //Bluetooth.readBytesUntil('\n', serialData, 31);
    //switch(serialData[0])
    switch(Bluetooth.read())
    {
    case 'w':
      setMotorSpeed(CONST_SPEED_MAX, CONST_SPEED_MAX);
      break;
    case 'a':
      setMotorSpeed(-CONST_SPEED_MAX, CONST_SPEED_MAX);
      break;
    case 's':
      setMotorSpeed(-CONST_SPEED_MAX, -CONST_SPEED_MAX);
      break;
    case 'd':
      setMotorSpeed(CONST_SPEED_MAX, -CONST_SPEED_MAX);
      break;
    case 'q':
      setMotorSpeed(0, 0);
      break;
    case '1':
      setCagePosition(CAGE_UP);
      break;
    case '2':
      setCagePosition(CAGE_DOWN);
      break;
    case 'p':
      if(enableAutoObstacleAvoidance)
      {
        enableAutoObstacleAvoidance = 0;
        setMotorSpeed(0, 0);
      }
      else
      {
        enableAutoObstacleAvoidance = 1;
      }
      break;
    case 'm':
      Bluetooth.println("Go Home");
      if(goHomeValue)
      {
        goHomeValue = 0;
        setMotorSpeed(0, 0);
      }
      else
      {
        goHomeValue = 1;
      }
      break;
    case 'l' :
      enableFrontLeds = enableFrontLeds ? 0 : 1;
      digitalWrite(PIN_LIGHTS, enableFrontLeds);
      break;
   
    }
  }
}

