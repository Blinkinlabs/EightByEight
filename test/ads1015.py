#!/usr/bin/python

import time

# ===========================================================================
# ADS1015 Class
# ===========================================================================

class ADS1015:

# ===========================================================================
#   I2C ADDRESS/BITS
# ==========================================================================
	__ADS1015_ADDRESS                         = 0x48
	__ADS1015_READ                            = 0x01
# ===========================================================================

# ===========================================================================

# ===========================================================================
#   CONVERSION REGISTER (R)
# ===========================================================================
	__ADS1015_REG_CONVERSION                = 0x00
# ===========================================================================


# ===========================================================================
#    CONFIG REGISTER (R/W)
# ===========================================================================
	__ADS1015_REG_CONFIG                      = 0x01
# ===========================================================================
	__ADS1015_CONFIG_OS                   = 0x8000  # Operational Status bit

	__ADS1015_CONFIG_MUX_MASK             = 0x7000  # Multiplexer mask
	__ADS1015_CONFIG_MUX_DIFF_01          = 0x0000  # AINp = AIN0, AINn = AIN1
	__ADS1015_CONFIG_MUX_DIFF_03          = 0x1000  # AINp = AIN0, AINn = AIN3
	__ADS1015_CONFIG_MUX_DIFF_13          = 0x2000  # AINp = AIN1, AINn = AIN3
	__ADS1015_CONFIG_MUX_DIFF_23          = 0x3000  # AINp = AIN2, AINn = AIN3
	__ADS1015_CONFIG_MUX_SINGLE_0         = 0x4000  # AINp = AIN0, AINn = GND
	__ADS1015_CONFIG_MUX_SINGLE_1         = 0x5000  # AINp = AIN1, AINn = GND
	__ADS1015_CONFIG_MUX_SINGLE_2         = 0x6000  # AINp = AIN2, AINn = GND
	__ADS1015_CONFIG_MUX_SINGLE_3         = 0x7000  # AINp = AIN3, AINn = GND

	__ADS1015_CONFIG_PGA_MASK             = 0x0E00  # Programmable gain amplifier mask
	__ADS1015_CONFIG_PGA_6_144V           = 0x0000  # Full scale: +/-6.144V
	__ADS1015_CONFIG_PGA_4_096V           = 0x0200  # Full scale: +/-4.096V
	__ADS1015_CONFIG_PGA_2_048V           = 0x0400  # Full scale: +/-2.048V
	__ADS1015_CONFIG_PGA_1_024V           = 0x0600  # Full scale: +/-1.024V
	__ADS1015_CONFIG_PGA_0_512V           = 0x0800  # Full scale: +/-.512V
	__ADS1015_CONFIG_PGA_0_256V           = 0x0A00  # Full scale: +/-.256V

	__ADS1015_CONFIG_MODE_CONTINUOUS      = 0x0100  # Continuous operation mode
	__ADS1015_CONFIG_MODE_SINGLE_SHOT     = 0x0000  # Single shot mode

	__ADS1015_CONFIG_DR_MASK              = 0x00E0  # Data rate mask
	__ADS1015_CONFIG_DR_128S              = 0x0000  # Data rate: 128 samples/s
	__ADS1015_CONFIG_DR_250S              = 0x0020  # Data rate: 250 samples/s
	__ADS1015_CONFIG_DR_490S              = 0x0040  # Data rate: 490 samples/s
	__ADS1015_CONFIG_DR_920S              = 0x0060  # Data rate: 920 samples/s
	__ADS1015_CONFIG_DR_1600S             = 0x0080  # Data rate: 1600 samples/s
	__ADS1015_CONFIG_DR_2400S             = 0x00A0  # Data rate: 2400 samples/s
	__ADS1015_CONFIG_DR_3300S             = 0x00C0  # Data rate: 3300 samples/s


	__ADS1015_CONFIG_COMP_MODE_TRADITIONAL = 0x0010  # Traditional comparitor with hysteresis
	__ADS1015_CONFIG_COMP_MODE_WINDOW      = 0x0000  # Window comparitor mode

	__ADS1015_CONFIG_COMP_POL_ACTIVE_LOW   = 0x0000  # Active low comparitor
	__ADS1015_CONFIG_COMP_POL_ACTIVE_HIGH  = 0x0004  # Active high comparitor

	__ADS1015_CONFIG_COMP_NONLATCHING      = 0x0000  # Nonlatching comparitor
	__ADS1015_CONFIG_COMP_LATCHING         = 0x0002  # Latching comparitor

	__ADS1015_CONFIG_COMP_QUE_MASK         = 0x0003  # Comparator queue and disable mask
	__ADS1015_CONFIG_COMP_QUE_1            = 0x0000  # Assert after one conversion
	__ADS1015_CONFIG_COMP_QUE_2            = 0x0001  # Assert after two conversions
	__ADS1015_CONFIG_COMP_QUE_4            = 0x0002  # Assert after four conversions
	__ADS1015_CONFIG_COMP_QUE_DISABLE      = 0x0003  # Comparitor disabled 


# ===========================================================================
#   LOW THRESHHOLD REGISTER (R/W)
# ===========================================================================
	__ADS1015_REG_LO_THRESH                  = 0x02
# ===========================================================================

# ===========================================================================
#   HIGH THRESHHOLD REGISTER (R/W)
# ===========================================================================
	__ADS1015_REG_HI_THRESH                       = 0x03
# ===========================================================================

	# Constructor
	def __init__(self, address=0x48, i2c=None, debug=False, **kwargs):
		if i2c is None:
			import Adafruit_GPIO.I2C as I2C
			i2c = I2C
		self._device = i2c.get_i2c_device(address, **kwargs)
		self.address = address
		self.debug = debug

		self.setCalibration()

	def twosToInt(self, val, len):
		# Convert twos compliment to integer

		if(val & (1 << len - 1)):
			val = val - (1<<len)

		return val

	def dumpRegisters(self):
                for reg in range(0,4):
                        result = self._device.readU16(reg, little_endian=False)
                        print(reg),
                        print(format(result,'04x'))
		


	def setCalibration(self):
		config = 0;

		# Widest input range setting
		config |= self.__ADS1015_CONFIG_PGA_6_144V
		
		# Slowest sample rate
		config |= self.__ADS1015_CONFIG_DR_128S
		
		# Disable comparitor
		config |= self.__ADS1015_CONFIG_COMP_QUE_DISABLE

		self.config = config
		
		bytes = [(config >> 8) & 0xFF, config & 0xFF]
		self._device.writeList(self.__ADS1015_REG_CONFIG, bytes)

		result = self._device.readList(self.__ADS1015_REG_CONFIG, 2)
		for index in range(0,len(bytes)):
			if bytes[index] != result[index]:
				print("Error setting config register")

	def measure(self, channel):
		# Trigger a measurement by writing out the config register
		config = self.config

		if(channel == 0):
			config |= self.__ADS1015_CONFIG_MUX_SINGLE_0
		elif(channel == 1):
			config |= self.__ADS1015_CONFIG_MUX_SINGLE_1
		elif(channel == 2):
			config |= self.__ADS1015_CONFIG_MUX_SINGLE_2
		elif(channel == 3):
			config |= self.__ADS1015_CONFIG_MUX_SINGLE_3

		bytes = [(config >> 8) & 0xFF, config & 0xFF]
		self._device.writeList(self.__ADS1015_REG_CONFIG, bytes)

		config |= self.__ADS1015_CONFIG_OS

		bytes = [(config >> 8) & 0xFF, config & 0xFF]
		self._device.writeList(self.__ADS1015_REG_CONFIG, bytes)
		bytes = [(config >> 8) & 0xFF, config & 0xFF]
		self._device.writeList(self.__ADS1015_REG_CONFIG, bytes)
		bytes = [(config >> 8) & 0xFF, config & 0xFF]
		self._device.writeList(self.__ADS1015_REG_CONFIG, bytes)
		bytes = [(config >> 8) & 0xFF, config & 0xFF]
		self._device.writeList(self.__ADS1015_REG_CONFIG, bytes)

		# then wait for it to complete
		while(True):
			result = self._device.readU16(self.__ADS1015_REG_CONFIG, little_endian=False)

			if not (result & self.__ADS1015_CONFIG_OS):
				break

		# Finally, read out the result
		result = self._device.readU16(self.__ADS1015_REG_CONVERSION, little_endian=False)
		#return (result >> 4)/float(1<<12-1)*6.144
		return (result >> 4)


if __name__ == '__main__':
	ads = ADS1015(address=0x49)
	ads.dumpRegisters()

	print(ads.measure(0))
	print(ads.measure(1))
	print(ads.measure(2))
	print(ads.measure(3))
