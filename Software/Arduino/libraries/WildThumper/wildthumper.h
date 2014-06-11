/***************************************************************************************
 *
 * Title:       WildThumper Motor Controller Library v1.0
 * File:        wildthumper.h
 * Date:        2014-05-27
 * Author:      Karl Kangur <karl.kangur@epfl.ch>
 *
 ***************************************************************************************/
#include <Arduino.h>

#ifndef _wildthumper_h
#define _wildthumper_h

// Servo output definitions
#define S0  2
#define S1  4
#define S2  7
#define S3  8
#define S4  9
#define S5 10
#define S6 12

// Other constant definitions
#define PIN_BATTERY   0 // Analog input 00
#define PIN_CURRENTL  6 // Analog input 06
#define PIN_CURRENTR  7 // Analog input 07
#define LED           13

void setSpeed(int8_t, int8_t);

class WildThumper
{
    public:
    WildThumper(void);
    void setSpeed(int8_t, int8_t);
    
    uint16_t battery(void);
    uint8_t currentLeft(void);
    uint8_t currentRight(void);
    uint8_t currentTotal(void);
    uint8_t charging(void);
};

#endif
