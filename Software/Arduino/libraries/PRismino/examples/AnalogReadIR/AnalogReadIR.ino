/***************************************************************************************
 *
 * Title:       Analog read IR
 * Description: Read the infra-red sensor from an analog input port.
 *
 ***************************************************************************************/
#include <prismino.h>

// the analog value is between 0 and 1023 so it needs a 16-bit variable
// can use "unsigned int" or "uint16_t" for short
unsigned int ir_value;

void setup()
{
  // set pin output mode (sources current)
  pinMode(LED, OUTPUT);
}

void loop()
{
  // connect sensor to pin A1, A0 is used by the potentiometer on the shield
  ir_value = analogRead(A1);
  
  if(ir_value < 512)
  {
    // a low value means something (an obstacle) reflects emitted IR to the receiever
    digitalWrite(LED, HIGH);
  }
  else
  {
    // no obstable is detected, turn LED off
    digitalWrite(LED, LOW);
  }
  delay(100);
}

