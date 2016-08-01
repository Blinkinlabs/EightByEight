import testrig
import espflasher
import armflasher
from timeout import timeout

import RPi.GPIO as GPIO

import subprocess

class EightByEightTestRig():

	dutPins = {
"ARM_BOOTLOADER" : "1", #ok
"ESP_GPIO0" : "2",	#ok
"ARM_RX1" : "3",	#ok
"ESP_RESET" : "4",	#ok
"ESP_GPIO16" : "5",	#ok
"I2C_SDA" : "6",	#ok
"LED_DATA" : "7",	#not connected on test rig
"I2C_SCL" : "8",	#ok
"LED_CLK" : "9",	#ok
"BUTTON" : "10",	#ok
"LED_STB" : "11",	#ok
"ESP_GPIO5" : "12",	#ok
"LED_OE" : "13",	#ok
"ARM_TMS" : "JTAG_TMS",
"ARM_TCK" : "JTAG_TCK",
"ARM_TX" : "JTAG_TDO",  # TODO FIX ME IN HARDWARE
"ARM_RX" : "JTAG_TDI",  # TODO FIX ME IN HARDWARE
"ARM_RESET" : "JTAG_RESET"
}

	dutVoltages = {
"BOOST_5V" : "1",
"VBAT" : "2",
"+5V" : "3",
"+3V3" : "4",
#"ARM_3V3" : "5"
}

	def __init__(self):
		self.testrig = testrig.TestRig()
		self.espFlasher = espflasher.EspFlasher()
		self.armFlasher = armflasher.ArmFlasher()

	def reset(self):
		self.testrig.setPowerMode("off")
		self.testrig.disableUSB()

		for pin, number in self.dutPins.items():
			self.testrig.digitalPinMode(number, GPIO.IN)

	def readDutPin(self, pin):
		if (pin in self.dutPins):
			return self.testrig.readDigitalPin(self.dutPins[pin])
		else:
			raise(NameError("Invalid pin"))

	def readDutPins(self):
		values = {}
		for pin in self.dutPins:
			values[pin] = self.readDutPin(pin)
		return values

	def dutPinMode(self, pin, mode):
		if pin in self.dutPins:
			self.testrig.digitalPinMode(self.dutPins[pin], mode)
		else:
			raise(NameError("Invalid pin"))

	def dutPinWrite(self, pin, state):
		if pin in self.dutPins:
			self.testrig.digitalPinWrite(self.dutPins[pin], state)
		else:
			raise(NameError("Invalid pin"))

	def readDutVoltage(self, name):
		if (name in self.dutVoltages):
			return self.testrig.readVoltage(self.dutVoltages[name])
		else:
			raise(NameError("Invalid pin"))
		
	def readDutVoltages(self):
		values = {}
		for pin in self.dutVoltages:
			values[pin] = self.readDutVoltage(pin)
		return values

	@timeout(5)
	def checkForUsbBootloaderDevice(self):
		# Test if the device is present on the USB bus, in bootloader mode

		result = 1
		while (result != 0):
			result = subprocess.call(["lsusb", "-d", "1d50:60f5"])
			#result = subprocess.call(["lsusb", "-d", "1d50:60a9"]) # TODO

		return True

	@timeout(5)
	def checkForUsbDevice(self):
		# Test if the device is present on the USB bus, in device mode
		result = 1
		while (result != 0):

			result = subprocess.call(["lsusb", "-d", "1d50:60f6"])
			#result = subprocess.call(["lsusb", "-d", "1d50:8888"]) # TODO

		return True

	@timeout(30)
	def checkForWifiConnection(self, macAddress):
		# Test if the device has connected to the wifi hotspot
		while True:
			clients = [line.rstrip('\n') for line in open('/var/lib/misc/dnsmasq.leases')]

			for client in clients:
				if macAddress in client:
					return True
		
		return False



def testIcspGpio():
	# ICSP GPIO test: use logic analyzer to verify each output is exercised in sequence
	dut.dutPinMode("ARM_RESET", GPIO.OUT)
	dut.dutPinMode("ARM_TMS", GPIO.OUT)
	dut.dutPinMode("ARM_TCK", GPIO.OUT)
	dut.dutPinMode("ARM_TX", GPIO.OUT)
	dut.dutPinMode("ARM_RX", GPIO.OUT)
	dut.dutPinWrite("ARM_RESET", GPIO.HIGH)
	dut.dutPinWrite("ARM_RESET", GPIO.LOW)
	dut.dutPinWrite("ARM_TX", GPIO.HIGH)
	dut.dutPinWrite("ARM_TX", GPIO.LOW)
	dut.dutPinWrite("ARM_RX", GPIO.HIGH)
	dut.dutPinWrite("ARM_RX", GPIO.LOW)
	dut.dutPinWrite("ARM_TMS", GPIO.HIGH)
	dut.dutPinWrite("ARM_TMS", GPIO.LOW)
	dut.dutPinWrite("ARM_TCK", GPIO.HIGH)
	dut.dutPinWrite("ARM_TCK", GPIO.LOW)

def testDigitalGpio():
	dut.dutPinMode("ARM_BOOTLOADER", GPIO.OUT)
	dut.dutPinMode("ESP_GPIO0", GPIO.OUT)
	dut.dutPinMode("ARM_RX1", GPIO.OUT)
	dut.dutPinMode("ESP_RESET", GPIO.OUT)
	dut.dutPinMode("ESP_GPIO16", GPIO.OUT)
	dut.dutPinMode("I2C_SDA", GPIO.OUT)
	dut.dutPinMode("LED_DATA", GPIO.OUT)
	dut.dutPinMode("I2C_SCL", GPIO.OUT)
	dut.dutPinMode("LED_CLK", GPIO.OUT)
	dut.dutPinMode("BUTTON", GPIO.OUT)
	dut.dutPinMode("LED_STB", GPIO.OUT)
	dut.dutPinMode("ESP_GPIO5", GPIO.OUT)
	dut.dutPinMode("LED_OE", GPIO.OUT)
	dut.dutPinWrite("ARM_BOOTLOADER", GPIO.HIGH)
	dut.dutPinWrite("ARM_BOOTLOADER", GPIO.LOW)
	dut.dutPinWrite("ESP_GPIO0", GPIO.HIGH)
	dut.dutPinWrite("ESP_GPIO0", GPIO.LOW)
	dut.dutPinWrite("ARM_RX1", GPIO.HIGH)
	dut.dutPinWrite("ARM_RX1", GPIO.LOW)
	dut.dutPinWrite("ESP_RESET", GPIO.HIGH)
	dut.dutPinWrite("ESP_RESET", GPIO.LOW)
	dut.dutPinWrite("ESP_GPIO16", GPIO.HIGH)
	dut.dutPinWrite("ESP_GPIO16", GPIO.LOW)
	dut.dutPinWrite("I2C_SDA", GPIO.HIGH)
	dut.dutPinWrite("I2C_SDA", GPIO.LOW)
	dut.dutPinWrite("LED_DATA", GPIO.HIGH)
	dut.dutPinWrite("LED_DATA", GPIO.LOW)
	dut.dutPinWrite("I2C_SCL", GPIO.HIGH)
	dut.dutPinWrite("I2C_SCL", GPIO.LOW)
	dut.dutPinWrite("LED_CLK", GPIO.HIGH)
	dut.dutPinWrite("LED_CLK", GPIO.LOW)
	dut.dutPinWrite("BUTTON", GPIO.HIGH)
	dut.dutPinWrite("BUTTON", GPIO.LOW)
	dut.dutPinWrite("LED_STB", GPIO.HIGH)
	dut.dutPinWrite("LED_STB", GPIO.LOW)
	dut.dutPinWrite("ESP_GPIO5", GPIO.HIGH)
	dut.dutPinWrite("ESP_GPIO5", GPIO.LOW)
	dut.dutPinWrite("LED_OE", GPIO.HIGH)
	dut.dutPinWrite("LED_OE", GPIO.LOW)

if __name__ == '__main__':
	import time

	dut = EightByEightTestRig()

	dut.reset()

	#testIcspGpio()
	#testDigitalGpio()



	dut.testrig.setPowerMode("full")
	dut.testrig.enableUSB()

	# Reset into bootloader mode

	dut.dutPinMode("ARM_BOOTLOADER", GPIO.OUT)
	dut.dutPinMode("ARM_BOOTLOADER", GPIO.LOW)

	dut.dutPinMode("ARM_RESET", GPIO.OUT)

	dut.dutPinWrite("ARM_RESET", GPIO.LOW)
	dut.dutPinWrite("ARM_RESET", GPIO.HIGH)

	print(dut.checkForUsbBootloaderDevice())


	# Reset into application mode

	dut.dutPinMode("ARM_BOOTLOADER", GPIO.OUT)
	dut.dutPinMode("ARM_BOOTLOADER", GPIO.HIGH)

	dut.dutPinMode("ARM_RESET", GPIO.OUT)

	dut.dutPinWrite("ARM_RESET", GPIO.LOW)
	dut.dutPinWrite("ARM_RESET", GPIO.HIGH)


	print(dut.checkForUsbDevice())


	print(dut.testrig.readDutPower())
	print(dut.readDutPins())
	print(dut.readDutVoltages())

