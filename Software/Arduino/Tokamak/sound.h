#ifndef _sound_h
#define _sound_h

// define a note with a pitch and a duration
struct note
{
  uint16_t pitch;
  uint16_t duration;
};

// define a sound that has a length (number of notes) and a list of notes
struct sound
{
  uint8_t length;
  note *notes;
};

#endif

