#!/usr/bin/env python

import visa
import time
from pylab import *

class ArduinoDAQ(object):
	def __init__(self, resource):
		
		self.instrument = visa.instrument(resource, term_chars = '\n', timeout=0.1)
		time.sleep(2)

		try:
			idn_response = self.instrument.ask('*IDN?')
		except:
			self.instrument.close()
			raise #Exception("Could not interrogate device.")
		
		if idn_response != 'OIC,Embedded SCPI Example,1,10':
			self.instrument.close()
			raise Exception("Incorrect IDN response.")
	
	def close(self):
		self.instrument.close()
		
	def read(self, channel):
		if channel == 0:
			return float(self.instrument.ask(':MEASURE:VOLTAGE?'))
		elif channel == 1:
			return float(self.instrument.ask(':MEASURE:VOLTAGE1?'))
		else:
			raise Exception("Invalid input channel ID.")
			
	def write(self, channel, value):
		if channel == 0:
			self.instrument.write(':SOURCE:VOLTAGE ' + value)
		elif channel == 1:
			self.instrument.write(':SOURCE:VOLTAGE1 ' + value)
		else:
			raise Exception("Invalid output channel ID.")
			
	def __del__(self):
		self.instrument.close()

daq = None

for resource in visa.get_instruments_list():
	try:
		daq = ArduinoDAQ('COM11')
	except:
		continue
		
	print "Found instrument on " + resource + "."
	break
	
if daq == None:
	print "Could not open find proper instrument."

t0 = []
in0 = []

t1 = []
in1 = []

# Set the initial output values of 0V.
daq.write(0, '0V')
daq.write(1, '0V')
time.sleep(1)

tbase = time.time()

# Read in baseline values.
t0.append(time.time() - tbase)
in0.append(daq.read(0))

t1.append(time.time() - tbase)
in1.append(daq.read(1))

# Attempt to create a unit step.
daq.write(0, '5V')
daq.write(1, '5V')

# Repeatedly measure output voltage.
for i in range(1,20):
	t0.append(time.time() - tbase)
	in0.append(daq.read(0))
	
	t1.append(time.time() - tbase)
	in1.append(daq.read(1))

# Reset our outputs to zero.
daq.write(0, '0V')
daq.write(1, '0V')

# Plot the results.
plot(t0, in0)
plot(t1, in1)
legend(['A0', 'A1'])
xlabel('Time (s)')
ylabel('Voltage (V)')
show()

daq.close()