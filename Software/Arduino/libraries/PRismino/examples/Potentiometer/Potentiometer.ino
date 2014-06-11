/***************************************************************************************
 *
 * Title:       Potentiometer
 * Description: Read the analog value of the 10Kohm potentiometer on pin A0.
 *
 ***************************************************************************************/
#include <prismino.h>

// stores values from 0 to 1023
uint16_t pot_value;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  pot_value = analogRead(POT);
  Serial.println(pot_value);
  // wait 500ms between measurements
  delay(500);
}

