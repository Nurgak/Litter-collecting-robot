/***************************************************************************************
 *
 * Title:      PRismino I2C master sketch communicating with the Raspberry Pi via
 *             serial and the motor controller via I2C.
 * Date:       2014-06-04
 *
 ***************************************************************************************/
#include <Wire.h>
#include <prismino.h>
#include <Servo.h>
//#include <Bluetooth.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "PRisminoMaster.h"

#define Bluetooth Serial1

// comment this line when testing without the motor controller
#define ENABLE_CONTROLLER
// comment this line to permanently disable all Bluetooth functionalities
#define ENABLE_BLUETOOTH

// IMU object instance
MPU6050 imu;

enum cage_positions cagePos = CAGE_UP;

Servo servoLeft;
Servo servoRight;

uint16_t batteryVoltage, currentLeft, currentRight;
uint16_t Z_MAX = 500, Z_MIN=100, X_MAX=500, X_MIN=100;
uint8_t ip[4] = {0, 0, 0, 0};
char buffer[16];
uint8_t enableAutoObstacleAvoidance = 0;
uint8_t enableFrontLeds = 0;
uint8_t enableSerialDumpIrValues = 0;
uint8_t wallSide=0;
uint8_t goHomeValue=0;



#ifdef ENABLE_CONTROLLER
uint32_t timeCheckBattery = 0;
uint32_t timeAntiLoop = 0;
#endif

#ifdef ENABLE_BLUETOOTH
char serialData[32];
#endif

void setup()
{
  // set pin output mode (sources current)
  pinMode(LED, OUTPUT);
  pinMode(PIN_LIGHTS, OUTPUT);
  
  // enable button pull-up
  pinMode(BTN, INPUT);
  digitalWrite(BTN, HIGH);

  // play a sound on boot, repeated sounds will indicate very low battery voltage
  play(TONE_BEEP, 500);

  // initialise serial bus for communication with the Raspberry Pi
  Serial.begin(9600);

  // initialise the bus for communication with the coimuter via Bluetooth
  #ifdef ENABLE_BLUETOOTH
  Bluetooth.begin(9600);
  #endif

  // join i2c bus (as master)
  Wire.begin();
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(I2C_COMPASS_ADDRESS); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();

  // request data from the motor controller as first handshake, this will inform of any i2c bus errors via a beep on the buzzer
  #ifdef ENABLE_CONTROLLER
  getPower();
  // initialise the time to check the battery
  timeCheckBattery = millis() + TIME_CHECK_BATTERY;
  timeAntiLoop = millis() + TIME_ANTI_LOOP;
  #endif

  // initialise servo positions
  //setCagePosition(CAGE_DOWN);
  setCagePosition(CAGE_DOWN);

  
}

void loop()
{
  static int16_t home_heading;
  
  serialCommunication();
  
  #ifdef ENABLE_BLUETOOTH
    bluetoothCommunication();
  #endif
  #ifdef ENABLE_CONTROLLER
    batteryCheck();
  #endif
  static uint8_t test=0;
  
  if(enableAutoObstacleAvoidance)
  {
    // no desired direction, give 0 as argument
    
    //goHome(0);
    
  }
  if(goHomeValue)
  {
    //headHome(home_heading);
    goHome(50);
    //navigation_avoidance(6);
//    getHeading();
  //setMotorSpeed(100,100);
  delay(100);
  }
  
  if(enableSerialDumpIrValues)
  {
    Serial.print(getAnalogSensor(0));
    Serial.print("\t");
    Serial.print(getAnalogSensor(1));
    Serial.print("\t");
    Serial.print(getAnalogSensor(2));
    Serial.print("\t");
    Serial.println(getAnalogSensor(3));
    //Serial.println(determineWallSide());
    delay(100);
  }

  // start auto obstacle avoidance
  if(!digitalRead(BTN))
  {
      if (goHomeValue == 0)
      {
        goHomeValue = 1;
        play(440, 500);
        delay(1000);
        //home_heading = getDirection();
        //resetReturnBase();
        play(440, 500);
      }
      else
      {
        goHomeValue = 0;
        play(440, 500);
        delay(1000);
        //home_heading = getDirection();
        //resetReturnBase();
        resetReturnBase();
        setMotorSpeed(0,0);
        play(440, 500);
      }
  }


}

void batteryCheck()
{
  // check battery level and make a beep if it's too low
  if(millis() > timeCheckBattery)
  {
    getPower();
    if(batteryVoltage < CONST_BATTERY_LOW)
    {
      play(TONE_BATTERY, 500);
    }
    timeCheckBattery = millis() + TIME_CHECK_BATTERY;
  }
}
void setCagePosition(cage_positions position)
{
  servoRight.attach(S1);
  servoLeft.attach(S2);

  uint8_t r, l;

  if(position == CAGE_UP)
  {
    // both sevomotors have 160 steps between up and down positions, so this is allowed
    /*for(l = SERVO_LEFT_DOWN, r = SERVO_RIGHT_DOWN; r < SERVO_RIGHT_UP; r++, l--)
    {
      servoLeft.write(l);
      servoRight.write(r);
      delay(CONST_MAX_SERVO_SPEED);
    }*/
    cagePos = CAGE_UP;
  }
  else if(position == CAGE_DOWN)
  {
    /*for(l = SERVO_LEFT_UP, r = SERVO_RIGHT_UP; r > SERVO_RIGHT_DOWN; r--, l++)
    {
      servoLeft.write(l);
      servoRight.write(r);
      delay(CONST_MAX_SERVO_SPEED);
    }*/
    cagePos = CAGE_DOWN;
  }
  // always detach the motors so that they are free-running and do not consume power for nothing in resting positions
  servoLeft.detach();
  servoRight.detach();
}

uint16_t getAnalogSensor(uint8_t id)
{
  uint16_t value;
  switch(id)
  {
  case 0:
    value = analogRead(A0);
    break;
  case 1:
    value = analogRead(A1);
    break;
  case 2:
    value = analogRead(A2);
    break;
  case 3:
    value = analogRead(A3);
    break;
  case 4:
    value = analogRead(A4);
    break;
  case 5:
    value = analogRead(A5);
    break;
  default:
    value = 0;
  }
  return value;
}


void getPower()
{
  Wire.beginTransmission(I2C_MOTOR_CONTROLLER_ADDRESS);
  // casting needed for some reason
  uint8_t available = Wire.requestFrom((uint8_t)I2C_MOTOR_CONTROLLER_ADDRESS, (uint8_t)6);

  // read 6 bytes, 2 by 2, high byte first: voltage, left motor current, right motor current
  if(available == 6)
  {
    batteryVoltage = (Wire.read() << 8) | Wire.read();
    currentLeft = (Wire.read() << 8) | Wire.read();
    currentRight = (Wire.read() << 8) | Wire.read();
  }
  else
  {
    // inform i2c error
    play(TONE_I2C_ERROR, 500);
  }

  Wire.endTransmission();
}


