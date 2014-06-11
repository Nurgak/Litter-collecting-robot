/***************************************************************************************
 *
 * Title:       WildThumper Motor Controller sketch
 * Date:        2014-05-27
 *
 ***************************************************************************************/
#include <wildthumper.h>
#include <Wire.h>

#define I2C_ADDRESS 4

#define TIME_CHECK_BATTERY 500
#define CONST_BATTERY_LEVEL_GOOD 470

#define PIN_BACK_LIGHT 12
#define TIME_BACK_LIGHT_BLINK 50
#define TIME_BACK_LIGHT_PASUE 500

// must be an even number
#define CONST_BACK_LIGHT_BLINK_TIMES 6

uint32_t backLightNextBlink = 0;
uint32_t backLightBlinkTimes = CONST_BACK_LIGHT_BLINK_TIMES;

WildThumper controller;

uint8_t batteryGood = 1;
uint8_t buffer[6];
uint16_t input;
uint32_t timeCheckBattery = 0;

void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  pinMode(PIN_BACK_LIGHT, OUTPUT);
  
  Wire.begin(I2C_ADDRESS);
  // disable internal pull-ups
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop()
{
  // check the battery every now and then
  if(millis() > timeCheckBattery + TIME_CHECK_BATTERY)
  {
    // show the battery state on the on-board LED
    if(controller.battery() < CONST_BATTERY_LEVEL_GOOD)
    {
      batteryGood = 0;
      digitalWrite(LED, LOW);
    }
    timeCheckBattery = millis() + TIME_CHECK_BATTERY;
  }
  
  if(millis() > backLightNextBlink && batteryGood)
  {
    if(backLightBlinkTimes)
    {
      backLightBlinkTimes--;
      digitalWrite(PIN_BACK_LIGHT, !digitalRead(PIN_BACK_LIGHT));
      backLightNextBlink = millis() + TIME_BACK_LIGHT_BLINK;
    }
    else
    {
      backLightBlinkTimes = CONST_BACK_LIGHT_BLINK_TIMES;
      backLightNextBlink = millis() + TIME_BACK_LIGHT_PASUE;
    }
  }
  else if(millis() > backLightNextBlink && !batteryGood)
  {
    // if the battery has gone under the threshold level blink continuously
    digitalWrite(PIN_BACK_LIGHT, !digitalRead(PIN_BACK_LIGHT));
    backLightNextBlink = millis() + TIME_BACK_LIGHT_BLINK;
  }
}

void requestEvent()
{
  input = controller.battery();
  buffer[0] = input >> 8;
  buffer[1] = input & 0xff;

  input = controller.currentLeft();
  buffer[2] = input >> 8;
  buffer[3] = input & 0xff;
  
  input = controller.currentRight();
  buffer[4] = input >> 8;
  buffer[5] = input & 0xff;
  
  Wire.write(buffer, 6);
}

void receiveEvent(int howMany)
{
  static int8_t speedLeft = 0;
  static int8_t speedRight = 0;
  
  while(Wire.available())
  {
    char c = Wire.read();
    if(c == 's')
    {
      speedLeft = Wire.read();
      speedRight = Wire.read();
      controller.setSpeed(speedLeft, speedRight);
    }
  }
}

