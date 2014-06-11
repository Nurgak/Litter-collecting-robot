/***************************************************************************************
 *
 * Title:       WildThumper Motor Controller Library v1.0
 * File:        wildthumper.cpp
 * Date:        2014-05-27
 * Author:      Karl Kangur <karl.kangur@epfl.ch>
 *
 ***************************************************************************************/
#include <wildthumper.h>

static volatile int8_t speedLeft, speedRight;

// initialisation routine setting up the timer, enabling pins and interrupt vectors
WildThumper::WildThumper(void)
{
  // h-bridge uses timer/counter 2 (8-bit), channels A and B
  // stop timer, set port operations to normal and waveform generation mode to Fast PWM (mode 3)
  // counter top value is 0xff (255) which gives 16MHz/256 = 62.5kHz, a prescaler is required
  TCCR2A = (1 << WGM21) | (1 << WGM20);
  // stop the timer set by Arduino (stops the default PWM)
  TCCR2B = 0;
  
  // set h-bridge control ports to output mode
  DDRB |= (1 << 3);
  DDRD |= (1 << 6) | (1 << 5) | (1 << 3);
  
  // enable interrupt vectors
  TIMSK2 = (1 << OCIE2A) | (1 << OCIE2B) | (1 << TOIE2);
  // enable overflow and compare interrupts for channels A and B
  TIFR2 = (1 << OCF2A) | (1 << OCF2B) | (1 << TOV2);
  
  // just in case enable interrupts if not already done by Arduino
  asm("sei");
}

// sets pwm for h-bridge
// pin mapping reference for the Atmega168: http://arduino.cc/en/Hacking/PinMapping168
void WildThumper::setSpeed(int8_t _speedLeft, int8_t _speedRight)
{
  // stop the PWM by clearing the prescaler
  TCCR2B = 0;
  
  // reset the h-bridge by clearing all port values
  PORTB &= ~(1 << 3);
  PORTD &= ~((1 << 6) | (1 << 5) | (1 << 3));
  
  // do not allow higher or lower values than 100 or -100
  if(_speedLeft < 0)
  {
    speedLeft = _speedLeft < -100 ? -100 : _speedLeft;
  }
  else
  {
    speedLeft = _speedLeft > 100 ? 100 : _speedLeft;
  }
  
  if(_speedRight < 0)
  {
    speedRight = _speedRight < -100 ? -100 : _speedRight;
  }
  else
  {
    speedRight = _speedRight > 100 ? 100 : _speedRight;
  }
  
  // set compare interrupt 
  uint16_t temp;
  
  // PWM compare value between 0 and 255
  temp = (long) 255 * (speedLeft > 0 ? speedLeft : -speedLeft) / 100;
  
  OCR2A = temp & 0xff;
  
  temp = (long) 255 * (speedRight > 0 ? speedRight : -speedRight) / 100;
  
  OCR2B = temp & 0xff;
  
  // reset timer
  TCNT2 = 0;
  
  // set prescaler to 64 (enable timer), do not set a lower prescaler
  // it won't work because of hardware restrictions (power transistors do not commute fast enough)
  // this gives 16MHz/256/64 = 976.5625Hz, documentation says maximum frequency is 24kHz
  TCCR2B |= (1 << CS22);
}

// interrupt vectors for pin toggling
ISR(TIMER2_COMPA_vect)
{
  if(speedLeft > 0)
  {
    PORTD &= ~(1 << 3);
  }
  else if(speedLeft < 0)
  {
    PORTB &= ~(1 << 3);
  }
}

ISR(TIMER2_COMPB_vect)
{
  if(speedRight > 0)
  {
    PORTD &= ~(1 << 5);
  }
  else if(speedRight < 0)
  {
    PORTD &= ~(1 << 6);
  }
}

ISR(TIMER2_OVF_vect)
{
  if(speedLeft > 0)
  {
    PORTD |= (1 << 3);
  }
  else if(speedLeft < 0)
  {
    PORTB |= (1 << 3);
  }
  
  if(speedRight > 0)
  {
    PORTD |= (1 << 5);
  }
  else if(speedRight < 0)
  {
    PORTD |= (1 << 6);
  }
}

uint16_t WildThumper::battery(void)
{
    return analogRead(PIN_BATTERY);
}

uint8_t WildThumper::currentLeft(void)
{
    return analogRead(PIN_CURRENTL);
}

uint8_t WildThumper::currentRight(void)
{
    return analogRead(PIN_CURRENTR);
}

uint8_t WildThumper::currentTotal(void)
{
    return this->currentLeft() + this->currentRight();
}
