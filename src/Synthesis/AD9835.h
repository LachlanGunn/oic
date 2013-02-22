#ifndef __AD9835_H
#define __AD9835_H

#include <Arduino.h>
#include <WProgram.h>

/**
 * Interface class for the %AD9835.
 */
class AD9835
{
public:
    AD9835(const int pinFSYNC, const int pinSCLK, const int pinSDATA,
           const int pinFSEL, const int pinPSEL1, const int pinPSEL0,
           const unsigned long hzMasterClockFrequency);
           
    void begin();
    void end();

    void enable();
    void disable();

    void setFrequencyCode(byte frequencyRegister,
                          unsigned long fcodeFrequency);
    void setFrequencyHz(byte frequencyRegister, unsigned long hzFrequency);
    void selectFrequencyRegister(byte frequencyRegister);
    unsigned long calculateFrequencyCodeHz(unsigned long hzFrequency);

    void setPhaseCode(byte phaseRegister, unsigned long pcodePhase);    
    void setPhaseDeg(byte phaseRegister, int degPhase);
    void selectPhaseRegister(byte phaseRegister);
    unsigned long calculatePhaseCodeDeg(unsigned long degPhase);
    
private:
    void writeSPI(byte msb, byte lsb);
    
private:
    int pinSCLK;
    int pinSDATA;
    int pinFSYNC;
    int pinFSEL;
    int pinPSEL1;
    int pinPSEL0;
    
    unsigned long hzMasterClockFrequency;
};

#endif
