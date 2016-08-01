#!/usr/bin/python

import unittest
#import colour_runner
#import redgreenunittest as unittest
import redgreenunittest

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
		IIN_MIN = 30
		IIN_MAX = 60
		VIN_MIN = 2
		VIN_MAX = 3.5

		time.sleep(.5)
		power = self.dut.testrig.readDutPower()
		self.results["shortTest_power"] = power

		self.assertGreaterEqual(power["Iin"],IIN_MIN)
		self.assertLessEqual(power["Iin"],IIN_MAX)
		self.assertGreaterEqual(power["Vin"],VIN_MIN)
		self.assertLessEqual(power["Vin"],VIN_MAX)

		self.dut.dutPinMode("ARM_RESET", GPIO.IN)
		self.dut.dutPinMode("ESP_RESET", GPIO.IN)

	def test_020_poweronTest(self):
		self.dut.dutPinMode("ARM_RESET", GPIO.OUT)
		self.dut.dutPinWrite("ARM_RESET", GPIO.LOW)
		self.dut.dutPinMode("ESP_RESET", GPIO.OUT)
		self.dut.dutPinWrite("ESP_RESET", GPIO.LOW)

		self.dut.testrig.setPowerMode("full")
		IIN_MIN = 30
		IIN_MAX = 60
		VIN_MIN = 5
		VIN_MAX = 5.3

		time.sleep(.5)
		power = self.dut.testrig.readDutPower()
		self.results["poweronTest_power"] = power

		self.assertGreaterEqual(power["Iin"],IIN_MIN)
		self.assertLessEqual(power["Iin"],IIN_MAX)
		self.assertGreaterEqual(power["Vin"],VIN_MIN)
		self.assertLessEqual(power["Vin"],VIN_MAX)

		self.dut.dutPinMode("ARM_RESET", GPIO.IN)
		self.dut.dutPinMode("ESP_RESET", GPIO.IN)

#	def test_030_dut5V_rail(self):
#		V_MIN = 4.6
#		V_MAX = 4.8
#
#		voltage = self.dut.readDutVoltage("+5V")
#		self.results["dut5V_rail_voltage"] = voltage



# battery charger tests


# Kinetis programming tests


	def test_400_programFirmware(self):
		self.dut.testrig.setPowerMode("full")
		self.dut.testrig.enableUSB()

		self.assertTrue(self.dut.armFlasher.writeFirmware("../bootloader/blinky-boot.elf"))


	def test_460_usbBootloaderMode(self):
		self.dut.testrig.setPowerMode("full")
		self.dut.testrig.enableUSB()

		self.dut.dutPinMode("ARM_BOOTLOADER", GPIO.OUT)
		self.dut.dutPinMode("ARM_BOOTLOADER", GPIO.LOW)

		self.dut.dutPinMode("ARM_RESET", GPIO.OUT)

		self.dut.dutPinWrite("ARM_RESET", GPIO.LOW)
		self.dut.dutPinWrite("ARM_RESET", GPIO.HIGH)

		self.assertTrue(self.dut.checkForUsbBootloaderDevice())

		self.dut.dutPinMode("ARM_BOOTLOADER", GPIO.IN)
		self.dut.dutPinMode("ARM_RESET", GPIO.IN)

	def test_470_usbApplicationMode(self):
		self.dut.dutPinMode("ARM_RESET", GPIO.OUT)

		self.dut.dutPinWrite("ARM_RESET", GPIO.LOW)
		self.dut.dutPinWrite("ARM_RESET", GPIO.HIGH)

		self.assertTrue(self.dut.checkForUsbDevice())



# ESP based tests
	def test_600_readChipInfo(self):
		self.results["readChipInfo"] = self.dut.espFlasher.readChipInfo()
		self.assertTrue(True)

	def test_610_flashTestFirmware(self):
		address = 0x0000
		filename = "/home/pi/EightByEight/bin/espTestFirmware.bin"

		self.dut.espFlasher.writeFirmware(address, filename)
		self.assertTrue(True)

	def test_620_wifiConnection(self):
		self.assertTrue(self.dut.checkForWifiConnection(self.results["readChipInfo"]["mac"]))

# LED current tests


# Accelerometer test



if __name__ == '__main__':
	rig = eightbyeight.EightByEightTestRig()

	while True:

		print("-------------------------------------------------------")
		print("""
  _____  ______          _______     __
 |  __ \|  ____|   /\   |  __ \ \   / /
 | |__) | |__     /  \  | |  | \ \_/ / 
 |  _  /|  __|   / /\ \ | |  | |\   /  
 | | \ \| |____ / ____ \| |__| | | |   
 |_|  \_\______/_/    \_\_____/  |_|   
""")

		print("-------------------------------------------------------")

		while (not rig.testrig.readStartButton()):
			pass
	
		rig.testrig.setLED("pass", True)
		rig.testrig.setLED("fail", True)

		#runner = unittest.TextTestRunner(failfast = True)
		runner = redgreenunittest.TextTestRunner(failfast = True)
		#runner = colour_runner.ColourTextTestRunner(failfast = True)
		result = runner.run(unittest.TestLoader().loadTestsFromTestCase(EightByEightTests))

		if len(result.failures) > 0 or len(result.errors) > 0:
			rig.testrig.setLED("pass", False)
			rig.testrig.setLED("fail", True)
			print("""
  ______      _____ _      
 |  ____/\   |_   _| |     
 | |__ /  \    | | | |     
 |  __/ /\ \   | | | |     
 | | / ____ \ _| |_| |____ 
 |_|/_/    \_\_____|______|
""")
		else:
			rig.testrig.setLED("pass", True)
			rig.testrig.setLED("fail", False)

			print("""
  _____         _____ _____ 
 |  __ \ /\    / ____/ ____|
 | |__) /  \  | (___| (___  
 |  ___/ /\ \  \___ \\___ \ 
 | |  / ____ \ ____) |___) |
 |_| /_/    \_\_____/_____/ 
""")

	
