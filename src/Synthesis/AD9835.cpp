/*!
 * \mainpage Frequency Synthesis Library for Arduino
 * \author   Lachlan Gunn
 *
 * \section Introduction
 * The Synthesis library is intended initially to provide an
 * interface to the %AD9835 Direct Digital Synthesis (DDS) IC,
 * with an eye towards eventually incorporating other methods
 * of synthesis.  This could mean interfaces to the rest of the
 * Analog Devices DDS family, or perhaps even to SCPI-enabled
 * signal generators.
 *
 * \section Hardware
 *
 * One will first require an %AD9835 IC.  This is available from
 * Digikey and Farnell slightly below the US/AU$15 mark, however
 * this is a needlessly difficult route if one is merely experimenting.
 * An alternative (taken by the author) is the breakout board sold by
 * Sparkfun:
 *
 *   - Sparkfun BOB-09169, US$32.57
 *                         AU$34.95 (from Little Bird Electronics)
 *
 * This is the easiest option for experimentation, as one need
 * not provide their own oscillator or power supplies, nor
 * solder the TSSOP package of the AD9835.  Note that this board requires
 * a 6--9V supply.
 *
 * Having acquired this device, one must connect it to their Arduino
 * board.  In order to use the samples provided, one must connect:
 *
 *    - FSYNC -> Digital 8
 *    - FSEL  -> Digital 7
 *    - PSEL1 -> Digital 6
 *    - PSEL0 -> Digital 5
 *    - SCLK  -> Digital 4
 *    - SDATA -> Digital 3
 *
 * This particular arrangement allows the board to be connected directly with
 * the aid of 0.1" header.
 *
 * Having connected the hardware, one would do well to examine the examples
 * provided.
 */

#include <Arduino.h>
#include <WProgram.h>
#include "AD9835.h"
#include <SPI.h>

#define HARDWARE_SPI 0

/**
 * Constructor for the AD9835 class.
 *
 * This constructor records the pin assignments used in preparation for their
 * setup in begin().
 *
 * \param pinFSYNC The IO pin connected to the FSYNC pin of the AD9835.
 * \param pinSCLK  The IO pin connected to the SPI clock of the AD9835.
 * \param pinSDATA The IO pin connected to the SPI data pin of the AD9835.
 *
 * \param pinFSEL  The IO pin connected to the frequency select pin
 *                 of the AD9835.
 *
 * \param pinPSEL1 The IO pin connected to phase select pin one of the AD9835.
 * \param pinPSEL0 The IO pin connected to phase select pin zero of the AD9835.
 *
 * \param hzMasterClockFrequency The frequency of the
 *                               AD9835 master clock in Hz.
 *
 * \see AD9835::begin
 */
AD9835::AD9835(const int pinFSYNC, const int pinSCLK, const int pinSDATA,
    const int pinFSEL, const int pinPSEL1, const int pinPSEL0,
    const unsigned long hzMasterClockFrequency)
{
    // First we initialise our class.
    this->pinSCLK  = pinSCLK;
    this->pinSDATA = pinSDATA;
    this->pinFSYNC = pinFSYNC;
    this->pinFSEL  = pinFSEL;
    this->pinPSEL1 = pinPSEL1;
    this->pinPSEL0 = pinPSEL0;

    this->hzMasterClockFrequency = hzMasterClockFrequency;
}

/**
 * Initialise the AD9835.
 *
 * The AD9835::begin method sets pin modes and clears the state of the AD9835.
 */
void AD9835::begin()
{
  
  // We then set up our outputs.  In this case, the device is of type (0,1) with FSYNC idle-high,
    // so FSYNC must start high and SCLK low.
    digitalWrite(pinFSYNC, HIGH);
    digitalWrite(pinFSEL,  LOW);
    digitalWrite(pinPSEL0, LOW);
    digitalWrite(pinPSEL1, LOW);

    // We also set everything as an output pin.
    if (!HARDWARE_SPI)
    {
      pinMode(pinSCLK,  OUTPUT);
      pinMode(pinSDATA, OUTPUT);
    }
    pinMode(pinFSYNC, OUTPUT);
    pinMode(pinFSEL,  OUTPUT);
    pinMode(pinPSEL0, OUTPUT);
    pinMode(pinPSEL1, OUTPUT);

    // Sleep, reset, clear.
    //  Sleep - device powers down.
    //  Reset - phase accumulator is set to 0.
    //  Clear - SYNC and SELSRC registers are set to zero.
    writeSPI(0xF8, 0x00);
    delay(1);

    // Device configuration.
    //   SYNC   - FSEL, PSELx are sampled asynchronously.
    //   SELSRC - FSEL, PSELx are read from the FSEL, PSELx pins.
    writeSPI(0x80, 0x00);
}

/**
 * Disables the AD9835.
 */
void AD9835::end()
{
  disable();
}

/**
 * Enables the output of the AD9835.
 */
void AD9835::enable()
{
  writeSPI(0xC0, 0x00);
}

/**
 * Disables the output of the AD9835.
 */
void AD9835::disable()
{
    // Set the device to sleep.
    writeSPI(0xE0, 0x00);
}

/**
 * Sets a frequency register of the AD9835 to some frequency code.
 *
 * \param frequencyRegister The register to be set (can be zero or one).
 * \param fcodeFrequency    The frequency code to be placed in the register.
 */
void AD9835::setFrequencyCode(byte      frequencyRegister,
			      unsigned long fcodeFrequency)
{
    word overlayRegister = (frequencyRegister & 0x01) << 2;

    writeSPI(0x33 | overlayRegister, (fcodeFrequency & 0xFF000000) >> 24);
    writeSPI(0x22 | overlayRegister, (fcodeFrequency & 0x00FF0000) >> 16);
    writeSPI(0x31 | overlayRegister, (fcodeFrequency & 0x0000FF00) >>  8);
    writeSPI(0x20 | overlayRegister, (fcodeFrequency & 0x000000FF)      );
}

/**
 * Wrapper for setFrequencyCode.
 *
 * The setFrequencyHz calculates the frequency code corresponding to the
 * frequency given (in Hertz) before calling setFrequencyCode.
 *
 * Calculating the code is relatively slow, and so it is better to call
 * calculateFrequencyCodeHz directly and cache the result where possible.
 *
 * \param frequencyRegister The register to be set (can be zero or one).
 *
 * \param HzFrequency The frequency (in Hertz) to be placed in the register.
 *
 * \see setFrequencyCode
 * \see calculateFrequencyCodeHz
 */
void AD9835::setFrequencyHz(byte frequencyRegister, unsigned long hzFrequency)
{
    setFrequencyCode(frequencyRegister, calculateFrequencyCodeHz(hzFrequency));
}

/**
 * Select the frequency register to be used.
 *
 * The selectFrequencyRegister sets the FSEL pin to the desired value,
 * thereby setting the output frequency to that set via setFrequencyCode.
 *
 * \see setFrequencyCode
 */
void AD9835::selectFrequencyRegister(byte frequencyRegister)
{
    digitalWrite(pinFSEL, frequencyRegister & 0x01);
}

/**
 * Sets a phase register of the AD9835 to some phase code.
 *
 * \param phaseRegister  The register to be set (can be 0,1,2,3).
 * \param pcodePhase     The phase code to be placed in the register.
 */
void AD9835::setPhaseCode(byte phaseRegister, unsigned long pcodePhase)
{
    word overlayRegister = (phaseRegister & 0x03) << 1;

    writeSPI(0x19 | overlayRegister, (pcodePhase & 0x0F00) >> 8);
    writeSPI(0x08 | overlayRegister, (pcodePhase & 0x00FF)     );
}

/**
 * Wrapper for setPhaseCode.
 *
 * setPhaseDeg sets a phase register of the AD9835 to an angle given in degrees.
 * This is slower than setPhaseCode, so using calculatePhaseCodeDeg directly and
 * caching the result is desirable.
 *
 * \param phaseRegister  The register to be set (can be 0,1,2,3).
 * \param pcodePhase     The phase (in degrees) to be placed in the register.
 *
 * \see setPhaseCode
 * \see setPhaseDeg
 */
void AD9835::setPhaseDeg(byte phaseRegister, int degPhase)
{
    setPhaseCode(phaseRegister, calculatePhaseCodeDeg(degPhase));
}

/**
 * Select the phase register to be activated.
 *
 * The selectPhaseRegister drives the PSEL0 and PSEL1 pins of the AD9835,
 * thereby setting the output phase to that set via setPhaseCode.
 *
 * \see setPhaseCode
 */
void AD9835::selectPhaseRegister(byte phaseRegister)
{
    digitalWrite(pinPSEL0, phaseRegister & 0x01);
    digitalWrite(pinPSEL1, phaseRegister & 0x02);
}

/**
 * Converts a frequency in Hertz to a frequency code.
 *
 * \param hzFrequency  The frequency to be converted.
 *
 * \return The frequency code of the desired frequency.
 *
 * \todo There is an off-by-one error here as we have rounded
 *       the frequency code down, rather than selecting the nearest.
*/
unsigned long AD9835::calculateFrequencyCodeHz(unsigned long hzFrequency)
{
    unsigned long fcodeFrequency = 0;
    const int bitsChunk = 4;

    for (int i = 0; i <= 32/bitsChunk; i++) {
      fcodeFrequency <<= bitsChunk;
      fcodeFrequency  |= hzFrequency / hzMasterClockFrequency;
      hzFrequency      = hzFrequency % hzMasterClockFrequency;
      hzFrequency    <<= bitsChunk;
    }

    return fcodeFrequency;
}

/**
 * Converts a phase in degrees to a phase code.
 *
 * \param degPhase  The phase to be converted.
 *
 * \return The phase code of the desired phase.
 *
 * \todo There is an off-by-one error here as we have rounded
 *       the frequency code down, rather than selecting the nearest.
 */
unsigned long AD9835::calculatePhaseCodeDeg(unsigned long degPhase)
{
    unsigned long pcodePhase = 0;
    const int bitsChunk = 4;

    for (int i = 0; i <= 12/bitsChunk; i++) {
        unsigned long divisor     = degPhase / 360;
        unsigned long hzRemainder = degPhase % 360;

        pcodePhase <<= bitsChunk;
        pcodePhase  |= divisor;
        degPhase     = hzRemainder;
        degPhase   <<= bitsChunk;
    }

    return pcodePhase;
}

void AD9835::writeSPI(byte msb, byte lsb)
{
    digitalWrite(pinSCLK, LOW);
    // Deassert FSYNC to select the chip.
    digitalWrite(pinFSYNC, LOW);

	shiftOut(pinSDATA, pinSCLK, MSBFIRST, msb);
    shiftOut(pinSDATA, pinSCLK, MSBFIRST, lsb);

    // Reassert FSYNC now that the transfer is complete.
    digitalWrite(pinFSYNC, HIGH);
	digitalWrite(pinSCLK, HIGH);
}
