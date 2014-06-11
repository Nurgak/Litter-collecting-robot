/***************************************************************************************
 *
 * Title:       DIP Switch
 * Description: This sketch shows how to use the DIP switch with interrupts.
 *
 ***************************************************************************************/
#include <prismino.h>

// callback functions for DIP switch state changes
void test1()
{
  Serial.println("Switch 1");
}

void test2()
{
  Serial.println("Switch 2");
}

void test3()
{
  Serial.println("Switch 3");
}

void test4()
{
  Serial.println("Switch 4");
}

void setup()
{
  // set LED pin (13) as output
  pinMode(LED, OUTPUT);
  
  // set switch callback, switch number (1 to 4), function to be called when the switch changes states and the callback mode
  // FALLING triggers on a falling edge (1->0 transisiton)
  // RISING triggers on a rising edge (0->1 transisiton)
  // CHANGE triggers on either a falling or a rising edge (default)
  // LOW triggers on a low level (0), will continue to trigger if the level is low
  dipSwitch(DIP1, test1, CHANGE);
  dipSwitch(DIP2, test2, FALLING);
  dipSwitch(DIP3, test3, RISING);
  dipSwitch(DIP4, test4, CHANGE);
  
  Serial.begin(9600);
}

void loop()
{
  // toggle LED every second
  delay(1000);
  digitalWrite(LED, !digitalRead(LED));
}
