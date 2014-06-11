/***************************************************************************************
 *
 * Title:       Button callback
 * Description: This sketch shows how to use the Robopoly shield's button with a callback.
 *
 ***************************************************************************************/
#include <prismino.h>

// called when the button is clicked
void buttonCallback()
{
  // play a 440Hz sound during 500ms on the shield buzzer
  play(440, 500);
}

void setup()
{
  // set up the button callback
  buttonCallback(buttonCallback);
}

void loop()
{

}
