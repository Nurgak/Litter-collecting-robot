/***************************************************************************************
 *
 * Title:       Sound
 * Description: Use the buzzer to make sounds. It's extremely annoying, don't abuse it.
 *
 ***************************************************************************************/
#include <prismino.h>

// play a decening tone from 880Hz to 440Hz with steps of 2Hz
uint16_t freq_start = 880;
uint16_t freq_end = 440;
uint16_t freq_step = 2;

uint16_t frequency;

void setup()
{

}

void loop()
{
  for(frequency = freq_start; frequency > freq_end; frequency -= freq_step)
  {
    // define playing frequency and time
    play(frequency, 6);
    // define pause between frequencies
    // keep playing time higher than the delay time for a nice square wave
    delay(5);
  }
  
  for(frequency = freq_end; frequency < freq_start; frequency += freq_step)
  {
    // notice how the sound changes when the delay is longer than the playing time
    play(frequency, 4);
    delay(5);
  }
}

