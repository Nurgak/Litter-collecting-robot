/***************************************************************************************
 *
 * Title:       Blink
 * Description: Turn the on-board LED on and off every 500 milliseconds.
 *
 ***************************************************************************************/
#include <prismino.h>

void setup()
{
  // set pin output mode (sources current)
  pinMode(LED, OUTPUT);
}

void loop()
{
  // turn LED on
  digitalWrite(LED, HIGH);
  // wait 500 milliseconds
  delay(500);
  // turn LED off
  digitalWrite(LED, LOW);
  delay(500);
}

