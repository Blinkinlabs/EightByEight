#!/usr/bin/python

import unittest

import flybynight

import avrisp
import hexreader
import ina219

import time

import Adafruit_BBIO.ADC as ADC
import Adafruit_BBIO.GPIO as GPIO



def readVoltageAveraged(channel, multiplier, count = 10):
	time.sleep(.5)

	# bug workaround, the samples are off by one?
	ADC.read(channel)
	ADC.read(channel)

	sum = 0.0;
	for i in range(0,count):
		sum += ADC.read(channel)
	return (sum/count)*multiplier*1.8


class FlyByNightTests(unittest.TestCase):
    	@classmethod
	def setUpClass(self):
		self.dut = flybynight.FlyByNightDut(0)
		ADC.setup();

		self.isp = avrisp.AvrISP("ATTINY45", 0, self.dut.reset)
		self.ina = ina219.INA219()

		self.results = {}

    	@classmethod
	def tearDownClass(self):
		self.dut.init()

		print(self.results)
		print("\n")
		with open("data.log", "a") as logfile:
			logfile.write(str(self.results))
			logfile.write("\n")

	def readCurrentAveraged(self, count = 10):
		sum = 0.0;
		for i in range(0,count):
			sum += self.ina.getCurrent_mA()
		return (sum/count)


	def test_000_initDut(self):
		self.dut.init()
		self.dut.enterReset()
		# Make sure the radio is disabled?
		self.assertTrue(True)

### Power tests

	def test_010_shortTest(self):
		self.dut.setPowerMode(1)	# current limited mode
		MA_MIN = -2
		MA_MAX = 10
		current = self.readCurrentAveraged()
		self.results["shortTest_current"] = current
		self.assertGreaterEqual(current,MA_MIN)
		self.assertLessEqual(current,MA_MAX)

	def test_020_poweronTest(self):
		self.dut.setPowerMode(2)	# current limited mode
		MA_MIN = -2
		MA_MAX = 10
		current = self.readCurrentAveraged()
		self.results["poweronTest_current"] = current
		self.assertGreaterEqual(current,MA_MIN)
		self.assertLessEqual(current,MA_MAX)

	def test_030_dutVoltage(self):
		READ_DELAY = .1
		V_MIN = 1.9
		V_MAX = 2.1

		# Sleep for a short time (?)
		time.sleep(READ_DELAY)
			
		voltage = readVoltageAveraged(self.dut.v_batt_sense,2)
		self.results["dutTest_voltage"] = voltage

		# Disable the LED output
		self.dut.disableLed()

		self.assertGreaterEqual(voltage, V_MIN)
		self.assertLessEqual(voltage, V_MAX)

	def test_040_ledCurrent(self):
		READ_DELAY = .1
		MA_MIN = 7
		MA_MAX = 14

		self.dut.enableLed();

		# Sleep for a short time (?)
		time.sleep(READ_DELAY)

		current = self.readCurrentAveraged()	
		self.results["ledTest_current"] = current
		self.assertGreaterEqual(current,MA_MIN)
		self.assertLessEqual(current,MA_MAX)

	def test_050_ledVoltage(self):
		V_MIN = 2.9
		V_MAX = 3.2

			
		voltage = readVoltageAveraged(self.dut.v_led_sense,2)
		self.results["ledTest_voltage"] = voltage

		self.assertGreaterEqual(voltage, V_MIN)
		self.assertLessEqual(voltage, V_MAX)

		# Disable the LED output
		self.dut.disableLed()

# attiny programming tests

	def test_200_enableProgramming(self):
		self.isp.open()
	
		self.assertTrue(self.isp.programmingEnable())
		self.assertTrue(self.isp.checkSignature())

	def test_210_eraseChip(self):
		self.assertTrue(self.isp.eraseChip())

		self.assertTrue(self.isp.programmingEnable())
		self.assertTrue(self.isp.checkSignature())

	def test_220_programFlash(self):
		firmware = hexreader.loadData('flybynight.hex')
		while len(firmware) % (self.isp.pageSize*2):
			firmware += [0xFF]

		for page in range(0, len(firmware)/(self.isp.pageSize*2)):
			startIndex = page*self.isp.pageSize*2
			self.assertTrue(self.isp.writeVerifyPage(page*self.isp.pageSize, firmware[startIndex:startIndex+self.isp.pageSize*2]))

	def test_230_programFuses(self):
		self.assertTrue(self.isp.writeAndVerifyFuse('low',0x42))
		self.assertTrue(self.isp.writeAndVerifyFuse('high',0xde))
		self.assertTrue(self.isp.writeAndVerifyFuse('efuse',0xff))

	def test_290_releaseProgrammer(self):
		self.isp.close()

	def test_900_releaseDut(self):
		self.dut.exitReset()
		self.dut.init()

if __name__ == '__main__':
	import Adafruit_BBIO.GPIO as GPIO

	GPIO.setup(flybynight.STATUS_LED_RED, GPIO.OUT)
	GPIO.setup(flybynight.STATUS_LED_GREEN, GPIO.OUT)
	GPIO.setup(flybynight.START_BUTTON, GPIO.IN)

	GPIO.output(flybynight.STATUS_LED_GREEN, GPIO.HIGH)
	GPIO.output(flybynight.STATUS_LED_RED, GPIO.LOW)
	while True:

		if GPIO.input(flybynight.START_BUTTON):
			GPIO.output(flybynight.STATUS_LED_GREEN, GPIO.LOW)
			GPIO.output(flybynight.STATUS_LED_RED, GPIO.LOW)

			#result = unittest.TextTestRunner().run(unittest.TestLoader().loadTestsFromTestCase(FlyByNightTests))
			runner = unittest.TextTestRunner(failfast = True)
			result = runner.run(unittest.TestLoader().loadTestsFromTestCase(FlyByNightTests))

			if len(result.failures) > 0 or len(result.errors) > 0:
				GPIO.output(flybynight.STATUS_LED_GREEN, GPIO.LOW)
				GPIO.output(flybynight.STATUS_LED_RED, GPIO.HIGH)
			else:
				GPIO.output(flybynight.STATUS_LED_GREEN, GPIO.HIGH)
				GPIO.output(flybynight.STATUS_LED_RED, GPIO.LOW)
