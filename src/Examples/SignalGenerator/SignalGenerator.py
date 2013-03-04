#!/usr/bin/env python

import sys
import time

import visa

from PySide.QtCore import *
from PySide.QtGui  import *

class Form(QDialog):

	def __init__(self, parent = None):
		super(Form, self).__init__(parent)
		self.setWindowTitle("Signal Generator Control")
		
		self.resourceEdit   = QLineEdit()
		self.resourceText   = QLabel("Resource")
		
		self.resourceBtn    = QPushButton("Connect")
		self.resourceBtn.setDefault(True)
		
		self.instrumentId   = QLabel("Not connected.");
		self.controls 		= QWidget(self)
		self.controls.hide()
		
		self.frequencyLabel = QLabel("<span style='font-size: 18pt; color: #008;'>Frequency</span>",
										self.controls)
										
		self.frequency      = QLabel(self.controls)
		self.frequencyEdit  = QLineEdit(self.controls)
		self.frequencyEdit.setPlaceholderText("New Frequency")
		self.frequencyEdit.returnPressed.connect(self.updateFrequency)
		self.frequencyEdit.setValidator(QDoubleValidator())
		
		controls_layout = QGridLayout()
		controls_layout.setHorizontalSpacing(10);
		
		controls_layout.addWidget(self.frequencyLabel, 0, 0);
		controls_layout.addWidget(self.frequency, 0, 1);
		controls_layout.addWidget(self.frequencyEdit, 1, 1);
		self.controls.setLayout(controls_layout)
		
		layout = QGridLayout()
		layout.addWidget(self.resourceText, 0, 0)
		layout.addWidget(self.resourceEdit, 0, 1)
		layout.addWidget(self.resourceBtn, 0, 2)
		layout.addWidget(self.instrumentId, 1, 0, 1, 3)
		layout.addWidget(self.controls, 2, 0, 1, 3)
		self.setLayout(layout)
		
		self.instrument = None
		
		self.resourceBtn.clicked.connect(self.connectInstrument)
		
	def connectInstrument(self):
		if self.instrument == None:
			try:
				self.instrument = visa.instrument(self.resourceEdit.text(), term_chars = '\n', delay=0.1)
				time.sleep(2)
				id = self.instrument.ask('*IDN?')
				
				self.instrumentId.setText(id)
				self.resourceEdit.setEnabled(False)
				self.resourceBtn.setText("Disconnect")
				self.resourceBtn.setDefault(False)
				
				self.readFrequency()
				
				self.controls.show()
			except:
				if self.instrument != None:
					self.instrument.close()
				
				self.controls.hide()
				raise
		else:
			self.instrument.close()
			self.instrument = None
			self.resourceEdit.setEnabled(True)
			self.resourceBtn.setText("Connect")
			self.resourceBtn.setDefault(True)
			self.instrumentId.setText("Not connected.")
			self.controls.hide()
		
	def updateFrequency(self):
		newFrequency = float(self.frequencyEdit.text())
		
		self.instrument.write((':SOURCE:FREQUENCY %f' % newFrequency))
		self.readFrequency()
		self.frequencyEdit.setText('')
		
	def readFrequency(self):
		frequency = self.instrument.ask(':SOURCE:FREQUENCY?')
		self.frequency.setText("<span style='font-size: 18pt; color: #000;'>%f</span>" % float(frequency))

if __name__ == '__main__':
	app = QApplication(sys.argv)
	form = Form()
	form.show()
	
	sys.exit(app.exec_())