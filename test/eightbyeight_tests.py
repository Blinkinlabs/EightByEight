#!/usr/bin/python

import unittest

import eightbyeight 
import RPi.GPIO as GPIO
import time



class EightByEightTests(unittest.TestCase):
    	@classmethod
	def setUpClass(self):
		self.dut = eightbyeight.EightByEightTestRig()

		self.results = {}

    	@classmethod
	def tearDownClass(self):
		self.dut.reset()

		print(self.results)
		print("\n")
		with open("data.log", "a") as logfile:
			logfile.write(str(self.results))
			logfile.write("\n")


	def test_000_initDut(self):
		self.dut.reset()

		# Make sure the radio is disabled?
		self.assertTrue(True)

### Power on tests

	def test_010_shortTest(self):
		self.dut.dutPinMode("ARM_RESET", GPIO.OUT)
		self.dut.dutPinWrite("ARM_RESET", GPIO.LOW)
		self.dut.dutPinMode("ESP_RESET", GPIO.OUT)
		self.dut.dutPinWrite("ESP_RESET", GPIO.LOW)

		self.dut.testrig.setPowerMode("limited")
		IIN_MIN = 60
		IIN_MAX = 80
		VIN_MIN = 2
		VIN_MAX = 5.3

		power = self.dut.testrig.readDutPower()
		self.results["shortTest_power"] = power

		self.assertGreaterEqual(power["Iin"],IIN_MIN)
		self.assertLessEqual(power["Iin"],IIN_MAX)
		self.assertGreaterEqual(power["Vin"],VIN_MIN)
		self.assertLessEqual(power["Vin"],VIN_MAX)

	def test_020_poweronTest(self):
		self.dut.testrig.setPowerMode("full")
		IIN_MIN = 100
		IIN_MAX = 300
		VIN_MIN = 5
		VIN_MAX = 5.3

		power = self.dut.testrig.readDutPower()
		self.results["poweronTest_power"] = power

		self.assertGreaterEqual(power["Iin"],IIN_MIN)
		self.assertLessEqual(power["Iin"],IIN_MAX)
		self.assertGreaterEqual(power["Vin"],VIN_MIN)
		self.assertLessEqual(power["Vin"],VIN_MAX)

	def test_030_dut5V_rail(self):
		V_MIN = 4.6
		V_MAX = 4.8

		voltage = self.dut.readDutVoltage("+5V")
		self.results["dut5V_rail_voltage"] = voltage



# battery charger tests


# Kinetis programming tests


# LED current tests


# ESP programming test

# Accelerometer test



if __name__ == '__main__':
	rig = eightbyeight.EightByEightTestRig()

	while True:

		if (rig.testrig.readStartButton()):
			rig.testrig.setLED("pass", True)
			rig.testrig.setLED("fail", True)

			runner = unittest.TextTestRunner(failfast = True)
			result = runner.run(unittest.TestLoader().loadTestsFromTestCase(EightByEightTests))

			if len(result.failures) > 0 or len(result.errors) > 0:
				rig.testrig.setLED("pass", False)
				rig.testrig.setLED("fail", True)
			else:
				rig.testrig.setLED("pass", True)
				rig.testrig.setLED("fail", False)
