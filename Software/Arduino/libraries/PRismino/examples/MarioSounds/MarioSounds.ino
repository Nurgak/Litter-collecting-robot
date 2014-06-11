/***************************************************************************************
 *
 * Title:       Sounds from Mario
 * Description: In-game sounds from any old handheld (Super)Mario game.
 *
 ***************************************************************************************/
#include <prismino.h>
#include "pitch.h"
#include "sound.h"

// a sound is an array of notes
note notesCoin[] = {
  {B5, 100},
  {E6, 200}
};

// actually make the sound, evaluate the number of notes and give the start of the note list
// save to program memory to avoid using ram
sound soundCoin = {sizeof(notesCoin) / sizeof(note), notesCoin};
// this is equivalent to: sound soundCoin = {2, notesCoin};

note notesPowerUp[] = {
  {G3, 50},
  {B4, 50},
  {D4, 50},
  {G4, 50},
  {B5, 50},
  {A4b, 50},
  {C4, 50},
  {E4b, 50},
  {A5b, 50},
  {C5, 50},
  {B4b, 50},
  {D4, 50},
  {F4, 50},
  {B5b, 50},
  {D5, 50}
};

sound soundPowerUp = {sizeof(notesPowerUp) / sizeof(note), notesPowerUp};

note notesOneUp[] = {
  {E4, 100},
  {G4, 100},
  {E5, 100},
  {C5, 100},
  {D5, 100},
  {G5, 100}
};

sound soundOneUp = {sizeof(notesOneUp) / sizeof(note), notesOneUp};

note notesRescueFanfare[] = {
  {C4, 100},
  {G3, 100},
  {E3, 100},
  {C4, 100},
  {G3, 100},
  {E3, 100},
  {C4, 600},
  {D4b, 100},
  {A3b, 100},
  {F3, 100},
  {D4b, 100},
  {A3b, 100},
  {F3, 100},
  {D4b, 600},
  {E4b, 100},
  {B3b, 100},
  {G3, 100},
  {E4b, 100},
  {B3b, 100},
  {G3, 100},
  {E4b, 300},
  {F4, 100},
  {F4, 100},
  {F4, 100},
  {G4, 600}
};

sound soundRescueFanfare = {sizeof(notesRescueFanfare) / sizeof(note), notesRescueFanfare};

note notesFlagpoleFanfare[] = {
  {G2, 100},
  {C3, 100},
  {E3, 100},
  {G3, 100},
  {C4, 100},
  {E4, 100},
  {G4, 300},
  {E4, 300},
  {A2b, 100},
  {C3, 100},
  {E3b, 100},
  {A3b, 100},
  {C4, 100},
  {E4b, 100},
  {A4b, 300},
  {E4b, 300},
  {B2b, 100},
  {D3, 100},
  {F3, 100},
  {B3b, 100},
  {D4, 100},
  {F4, 100},
  {B4b, 300},
  {B4b, 100},
  {B4b, 100},
  {B4b, 100},
  {C5, 600},
};

sound soundFlagpoleFanfare = {sizeof(notesFlagpoleFanfare) / sizeof(note), notesFlagpoleFanfare};

// list of all available sounds
sound* soundList[] = {&soundCoin, &soundPowerUp, &soundOneUp, &soundRescueFanfare, &soundFlagpoleFanfare};

// sound index
uint8_t currentSound = 0;

void setup()
{
  // enable button pull-up resistor
  pinMode(7, INPUT);
  digitalWrite(7, HIGH);
  Serial.begin(9600);
}

// function that actually plays the sounds
void playSound(sound *s)
{
  // pointer to the sound object
  sound soundPtr = *s;
  
  uint16_t frequency, duration;
  uint8_t length = soundPtr.length;
  for(uint8_t i = 0; i < length; i++)
  {
    // get data from program memory
    frequency = soundPtr.notes[i].pitch;
    duration = soundPtr.notes[i].duration;
    
    // if the next note is the same then make a short pause
    uint8_t pause = 0;
    if(i < length - 1 && frequency == soundPtr.notes[i + 1].pitch)
    {
      // 5 millisecond pause
      pause = 5;
    }
    
    // play the right pitch for the determined duration
    play(frequency, duration - pause);
    
    // play is not a blocking function so one has to manually set a delay
    delay(duration);
  }
}

void loop()
{
  // if the button is clicked play all the sounds in the list
  if(!digitalRead(7))
  {
    // play the current and increment index
    playSound(soundList[currentSound++]);
    
    // wrap around when all sounds have been played
    if(currentSound > sizeof(soundList) / sizeof(sound*) - 1)
    {
      currentSound = 0; 
    }
  }
}

