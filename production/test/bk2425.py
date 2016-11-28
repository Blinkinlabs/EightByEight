import Adafruit_BBIO.SPI as SPI
import Adafruit_BBIO.GPIO as GPIO
import time

DUT_0_RADIO_CSN = "P8_14"
DUT_0_RADIO_CE = "P8_16"
DUT_0_LED_ENABLE = "P8_18"
DUT_0_RESET = "P9_42"

V_LED_0_SENSE = "AIN2"
V_BATT_0_SENSE = "AIN0"


class bk2425:
	def __init__(self, input = 0):
		if (input == 0):
			self.spi = SPI.SPI(1,1)
			self.v_led_sense =  V_LED_0_SENSE
			self.v_batt_sense = V_BATT_0_SENSE
			self.led_enable =   DUT_0_LED_ENABLE
			self.reset =        DUT_0_RESET
			self.radio_ce =     DUT_0_RADIO_CE
			self.radio_csn =     DUT_0_RADIO_CSN	# TODO: Handle me externally

		GPIO.setup(self.reset, GPIO.IN)

		self.spi.mode=0
		self.spi.msh = 10000

	def open(self):

		GPIO.setup(self.reset, GPIO.OUT)
		GPIO.output(self.reset, GPIO.HIGH)
		time.sleep(.01)
		GPIO.output(self.reset, GPIO.LOW)
		
		time.sleep(.01)
		GPIO.setup(self.radio_csn, GPIO.OUT)
		GPIO.output(self.radio_csn, GPIO.HIGH)
		GPIO.setup(self.radio_ce, GPIO.OUT)
		GPIO.output(self.radio_ce, GPIO.HIGH)

		time.sleep(.03)


		# pull reset low and then high again?

	def programmingEnable(self):
		self.spi.open(1,1)
		print(self.spi.xfer2([0xAC, 0x53, 0x00, 0x00]))
		self.spi.close()

		return False

	def getSignature(self):
		# For ATTiny25
		SIGNATURE_BYTE_0 = 0x1E
		SIGNATURE_BYTE_1 = 0x92
		SIGNATURE_BYTE_2 = 0x93

		self.spi.open(1,1)
		print(self.spi.xfer2([0x30,0x00,0x00,0x00]))
		self.spi.close()

		return False

	def close(self):
		GPIO.setup(self.radio_csn, GPIO.IN)
		GPIO.setup(self.reset, GPIO.IN)

		GPIO.cleanup()

if __name__ == '__main__':
	icp = AvrISP(0)
	icp.open()
	print(icp.programmingEnable())
	print(icp.getSignature())
	icp.close()
