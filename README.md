Open Instrument Control
=======================

The goal of this project is to develop an embedded-side software stack for
instrument control.  The initial target will be the Arduino platform, however
the goal is to eventually provide support for at least 8/16/32-bit PIC, AVR,
and ARM.

Initial development is for PC, and not much effort has been spent on
efficiency.  Data structures are all dynamically-allocated linked lists,
unsuitable for platforms with constrained resources.

The system will be based upon SCPI.  Adherence to the requirements specified
in IEEE 488.2 is not initially planned, as this standard is not available
to the author.

## Getting Started ##

We have developed a simple DAQ interface using an Arduino board, with the
ability to both read (using the in-built ADC) and write (using PWM) voltages.

1. Copy the src/ArduinoSCPIParser directory to your Arduino library directory.
	In my case, this was C:\Users\Lachlan\Arduino\libraries\ .
2. Open the sketch src/Examples/Meter/Meter.ino
3. Upload it to your Arduino board.
4. Use the serial monitor to talk to the board.  Some example commands
	* *IDN? (print some version information)
	* :SOURCE:VOLTAGE 1V (set the PWM on pin three to output 1V)
	* :MEASURE:VOLTAGE? (read the voltage on analogue input zero)
	
## Version 1 (In development) ##

The first version of OIC will provide a basic platform upon which further
development may begin.  This release shall provide a virtual instrument
platform for the Arduino Uno.

Communication shall be via the virtual serial port provided by the device,
which shall provide a bare-bones implementation of SCPI.

In addition to the basic platform, this release will include two virtual
instruments:

* An implementation of the Digital Meter instrument class using the
	on-board ADC.
* An implementation of the RF and Microwave Source class using the Sparkfun
	AD9835 breakout board.

### 0.1 (Completed) ###

Release 0.1 is a prototype of the SCPI implementation, and will
an implementation of SCPI-99 on the PC.

### 0.2 (Completed) ###

Release 0.2 contains an embedded version of the SCPI implementation.  This
is to demonstrated with a simple implementation of the digital meter
instrument and an accompanying Python script to perform the read operation.

### 0.3 (In Development) ###

The 0.3 release will provide complete implementations of the Digital Meter
and RF Source classes as described by the SCPI implementation, along with
the host-side software necessary to operate them from either a GUI or a
Python shell.

### 1.0 (Planned) ###

The 1.0 release will provide multi-instrument support, allowing both the
voltage measurement and signal generator instruments to be accessed with
a single communications channel.

## Version 2 (Planned) ##

The second version will provide support for PIC32 using the Microchip
USB to Serial driver and USBTMC.