import ina219
import Adafruit_ADS1x15
import RPi.GPIO as GPIO


class TestRig:
	leds = {"pass" : 14, "fail" : 15}
	powerModes = {"full" : 24, "limited" : 27}

	#"name" : gpio
	digitalPins = {
"1" : 5,
"2" : 6,
"3" : 12,
"4" : 13,
"5" : 16,
"6" : 19,
"7" : 20,
"8" : 21,
"9" : 26,
"10" : 4,
"11" : 17,
"12" : 22,
"13" : 23,
#14 conflicts with power_limited
#15 conflicts with power_full
"JTAG_TMS" : 25,
"JTAG_TCK" : 11,
"JTAG_TDI" : 10,
"JTAG_TDO" : 9,
"JTAG_RESET" : 7
}

	#"name" : [adc, channel]
	analogPins = {
"1" : [0, 3],
"2" : [0, 2],
"3" : [0, 1],
"4" : [0, 0],
"5" : [1, 3],
"6" : [1, 2],
"7" : [1, 1],
"8" : [1, 0]
}

	usbPin = 18
	startButtonPin = 8

	def __init__(self):
		GPIO.setmode(GPIO.BCM)
		GPIO.setwarnings(False)

		for pin in self.leds.itervalues():
			GPIO.setup(pin, GPIO.OUT)
			GPIO.output(pin, GPIO.LOW)

		for pin in self.powerModes.itervalues():
			GPIO.setup(pin, GPIO.OUT)
			GPIO.output(pin, GPIO.LOW)

		for pin in self.digitalPins.itervalues():
			GPIO.setup(pin, GPIO.IN)

		self.dutCurrent = ina219.INA219()

		self.adc0 = Adafruit_ADS1x15.ADS1015(address=0x48)
		self.adc1 = Adafruit_ADS1x15.ADS1015(address=0x49)

		GPIO.setup(self.usbPin, GPIO.OUT)
		GPIO.output(self.usbPin, GPIO.LOW)

		GPIO.setup(self.startButtonPin, GPIO.IN)

	def setLED(self, led, state):
		if led in self.leds:
			GPIO.output(self.leds[led], state)
		else:
			raise("Invalid led")

	def readStartButton(self):
		return not GPIO.input(self.startButtonPin)
		

	def setPowerMode(self, powerMode):
		for pin in self.powerModes.itervalues():
			GPIO.output(pin, GPIO.LOW)

		if powerMode in self.powerModes:
			GPIO.output(self.powerModes[powerMode], GPIO.HIGH)
		elif powerMode == "off":
			pass
		else:
			raise(NameError("Invalid power state"))

	def enableUSB(self):
		GPIO.output(self.usbPin, GPIO.HIGH)
		
	def disableUSB(self):
		GPIO.output(self.usbPin, GPIO.LOW)

	def readDutPower(self):
		values = {}

		self.dutCurrent.measure()
		values["Iin"] = self.dutCurrent.getCurrent_mA()
		values["Vin"] = self.dutCurrent.getBusVoltage_V()
		return values
	
	def readVoltage(self, pin):
		GAIN=2/3

		if (pin in self.analogPins):
			adcinfo = self.analogPins[pin]
			if (adcinfo[0] == 0):
				return self.adc0.read_adc(adcinfo[1], gain=GAIN)
			elif (adcinfo[0] == 1):
				return self.adc1.read_adc(adcinfo[1], gain=GAIN)
			else:
				raise(NameError("Invalid adc"))
		else:
			raise(NameError("Invalid pin"))

	def readDigitalPin(self, pin):
		if pin in self.digitalPins:
			return GPIO.input(self.digitalPins[pin])
		else:
			raise(NameError("Invalid pin"))

	def digitalPinMode(self, pin, mode):
		if pin in self.digitalPins:
			GPIO.setup(self.digitalPins[pin], mode)
		else:
			raise(NameError("Invalid pin"))

	def digitalPinWrite(self, pin, state):
		if pin in self.digitalPins:
			GPIO.output(self.digitalPins[pin], state)
		else:
			raise(NameError("Invalid pin"))
	
if __name__ == '__main__':
	import time

	rig = TestRig()
	
#while(True):
#	rig.setPowerMode("limited")
#	time.sleep(.1)
#	rig.dutCurrent.measure()
#	print(rig.dutCurrent.getCurrent_mA()),
#	print(rig.dutCurrent.getBusVoltage_V())
#
#	rig.setPowerMode("full")
#	time.sleep(.1)
#	rig.dutCurrent.measure()
#	print(rig.dutCurrent.getCurrent_mA()),
#	print(rig.dutCurrent.getBusVoltage_V())
#
#	rig.setPowerMode("off")
#	time.sleep(.1)
#	rig.dutCurrent.measure()
#	print(rig.dutCurrent.getCurrent_mA()),
#	print(rig.dutCurrent.getBusVoltage_V())
#	print("")

#rig.enableUSB()
#time.sleep(.1)
#rig.disableUSB()

#rig.setPowerMode("full")
#rig.enableUSB()
#rig.readVoltages()
#print(rig.readDigitalPins())
#print(rig.readDigitalPin("2"))

#rig.setPowerMode("off")
#rig.disableUSB()
#rig.readVoltages()
