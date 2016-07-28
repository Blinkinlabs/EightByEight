import Adafruit_BBIO.ADC as ADC
import Adafruit_BBIO.GPIO as GPIO


DUT_0_RADIO_CSN = "P8_33"
DUT_0_RADIO_CE = "P8_35"
DUT_0_LED_ENABLE = "P8_37"
DUT_0_RESET = "P9_42"

DUT_0_LOW_CURRENT = "P8_13"
DUT_0_NORMAL_CURRENT = "P8_14"

V_LED_0_SENSE = "AIN3"
V_BATT_0_SENSE = "AIN5"

STATUS_LED_RED = "P8_10"
STATUS_LED_GREEN = "P8_9"
START_BUTTON = "P8_11"

class FlyByNightDut:
	def __init__(self, input = 0):
		if (input == 0):
			self.v_led_sense =  V_LED_0_SENSE
			self.v_batt_sense = V_BATT_0_SENSE
			self.led_enable =   DUT_0_LED_ENABLE
			self.reset =        DUT_0_RESET
			self.radio_ce =     DUT_0_RADIO_CE
			self.low_current =  DUT_0_LOW_CURRENT
			self.normal_current =  DUT_0_NORMAL_CURRENT


	def init(self):
		GPIO.setup(self.led_enable, GPIO.IN)
		GPIO.setup(self.reset, GPIO.IN)
		GPIO.setup(self.radio_ce, GPIO.IN)
		GPIO.setup(self.low_current, GPIO.OUT)
		GPIO.setup(self.normal_current, GPIO.OUT)
		self.setPowerMode(0)

	def setPowerMode(self, mode):
		if mode == 0:
			GPIO.output(self.low_current, GPIO.HIGH)
			GPIO.output(self.normal_current, GPIO.HIGH)
		if mode == 1:
			GPIO.output(self.low_current, GPIO.LOW)
			GPIO.output(self.normal_current, GPIO.HIGH)
		if mode == 2:
			GPIO.output(self.low_current, GPIO.HIGH)
			GPIO.output(self.normal_current, GPIO.LOW)

	def enterReset(self):
		# Put the microcontroller in reset mode
		GPIO.setup(self.reset, GPIO.OUT)
		GPIO.output(self.reset, GPIO.LOW)

	def exitReset(self):
		# Exit reset mode (starts the microcontroller)
		GPIO.setup(self.reset, GPIO.IN)

	def enableLed(self):
		# Enable the LED output
		GPIO.setup(self.led_enable, GPIO.OUT)
		GPIO.output(self.led_enable, GPIO.HIGH)

	def disableLed(self):
		GPIO.setup(self.led_enable, GPIO.IN)
