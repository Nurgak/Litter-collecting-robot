/***************************************************************************************
 *
 * Title:       Servo Motor
 * Description: Use the servo motors to turn to a specific angle from 0-180 degrees,
 *              uses the timer2.
 *
 ***************************************************************************************/
#include <prismino.h>
#include <Servo.h>

// define the servo motor instances here
Servo servo1;
Servo servo2;

void setup()
{
  // attach the servo motor pins to the instances
  servo1.attach(S1); // pin 6
  servo2.attach(S2); // pin 5
}

void loop()
{
  // sweep from 0 to 180 degrees
  for(uint8_t pos = 0; pos < 180; pos++)
  {
    servo1.write(pos);
    servo2.write(pos);
    // add a small delay so the servo motors have time to move
    delay(50);
  }
  
  for(uint8_t pos = 180; pos > 0; pos--)
  {
    servo1.write(pos);
    servo2.write(pos);
    delay(50);
  }
}
