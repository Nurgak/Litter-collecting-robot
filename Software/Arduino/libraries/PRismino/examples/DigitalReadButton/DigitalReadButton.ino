/***************************************************************************************
 *
 * Title:       Digital Read Button
 * Description: Read the lever button value digitally.
 *
 ***************************************************************************************/
#include <prismino.h>

// the button value can be 0 or 1
unsigned char button_value;

void setup()
{
  // set pin output mode (sources current)
  pinMode(LED, OUTPUT);
}

void loop()
{
  // connect lever button to pin A4
  button_value = digitalRead(A4);
  
  if(button_value)
  {
    digitalWrite(LED, HIGH);
  }
  else
  {
    digitalWrite(LED, LOW);
  }
  delay(100);
}

