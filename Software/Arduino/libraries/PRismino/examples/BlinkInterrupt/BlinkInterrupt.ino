/***************************************************************************************
 *
 * Title:       Blink Interrupt
 * Description: Toggle the on-board LED using an interrupt.
 *
 ***************************************************************************************/
#include <prismino.h>

void blink()
{
  // reverse the LED value
  digitalWrite(LED, !digitalRead(LED));
}

void setup()
{
  // set LED pin (13) as output
  pinMode(LED, OUTPUT);
  // call the blink function every 5*100ms, 10 times and then stop
  // set the number of times (3rd argument) to 0 to call the function indefinitely
  setTimer(blink, 5, 10);
}

void loop()
{
  
}
