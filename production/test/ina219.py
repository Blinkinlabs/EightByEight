#!/usr/bin/python

import time

# ===========================================================================
# INA219 Class
# ===========================================================================

class INA219:

# ===========================================================================
#   I2C ADDRESS/BITS
# ==========================================================================
	__INA219_ADDRESS                         = 0x40    # 1000000 (A0+A1=GND)
	__INA219_READ                            = 0x01
# ===========================================================================

# ===========================================================================
#    CONFIG REGISTER (R/W)
# ===========================================================================
	__INA219_REG_CONFIG                      = 0x00
# ===========================================================================
	__INA219_CONFIG_RESET                    = 0x8000  # Reset Bit
	__INA219_CONFIG_BVOLTAGERANGE_MASK       = 0x2000  # Bus Voltage Range Mask
	__INA219_CONFIG_BVOLTAGERANGE_16V        = 0x0000  # 0-16V Range
	__INA219_CONFIG_BVOLTAGERANGE_32V        = 0x2000  # 0-32V Range

	__INA219_CONFIG_GAIN_MASK                = 0x1800  # Gain Mask
	__INA219_CONFIG_GAIN_1_40MV              = 0x0000  # Gain 1, 40mV Range
	__INA219_CONFIG_GAIN_2_80MV              = 0x0800  # Gain 2, 80mV Range
	__INA219_CONFIG_GAIN_4_160MV             = 0x1000  # Gain 4, 160mV Range
	__INA219_CONFIG_GAIN_8_320MV             = 0x1800  # Gain 8, 320mV Range

	__INA219_CONFIG_BADCRES_MASK             = 0x0780  # Bus ADC Resolution Mask
	__INA219_CONFIG_BADCRES_9BIT_1S_84US     = 0x0080  # 1x 9-bit bus res = 0..511
	__INA219_CONFIG_BADCRES_10BIT_1S_148US   = 0x0100  # 1x 10-bit bus res = 0..1023
	__INA219_CONFIG_BADCRES_11BIT_1S_276US   = 0x0200  # 1x 11-bit bus res = 0..2047
	__INA219_CONFIG_BADCRES_12BIT_1S_532US   = 0x0400  # 1x 12-bit bus res = 0..4097
	__INA219_CONFIG_BADCRES_12BIT_2S_1060US  = 0x0480  # 2 x 12-bit samples averaged together
	__INA219_CONFIG_BADCRES_12BIT_4S_2130US  = 0x0500  # 4 x 12-bit samples averaged together
	__INA219_CONFIG_BADCRES_12BIT_8S_4260US  = 0x0580  # 8 x 12-bit samples averaged together
	__INA219_CONFIG_BADCRES_12BIT_16S_8510US = 0x0600  # 16 x 12-bit samples averaged together
	__INA219_CONFIG_BADCRES_12BIT_32S_17MS   = 0x0680  # 32 x 12-bit samples averaged together
	__INA219_CONFIG_BADCRES_12BIT_64S_34MS   = 0x0700  # 64 x 12-bit samples averaged together
	__INA219_CONFIG_BADCRES_12BIT_128S_69MS  = 0x0780  # 128 x 12-bit samples averaged together

	__INA219_CONFIG_SADCRES_MASK             = 0x0078  # Shunt ADC Resolution and Averaging Mask
	__INA219_CONFIG_SADCRES_9BIT_1S_84US     = 0x0000  # 1 x 9-bit shunt sample
	__INA219_CONFIG_SADCRES_10BIT_1S_148US   = 0x0008  # 1 x 10-bit shunt sample
	__INA219_CONFIG_SADCRES_11BIT_1S_276US   = 0x0010  # 1 x 11-bit shunt sample
	__INA219_CONFIG_SADCRES_12BIT_1S_532US   = 0x0018  # 1 x 12-bit shunt sample
	__INA219_CONFIG_SADCRES_12BIT_2S_1060US  = 0x0048  # 2 x 12-bit shunt samples averaged together
	__INA219_CONFIG_SADCRES_12BIT_4S_2130US  = 0x0050  # 4 x 12-bit shunt samples averaged together
	__INA219_CONFIG_SADCRES_12BIT_8S_4260US  = 0x0058  # 8 x 12-bit shunt samples averaged together
	__INA219_CONFIG_SADCRES_12BIT_16S_8510US = 0x0060  # 16 x 12-bit shunt samples averaged together
	__INA219_CONFIG_SADCRES_12BIT_32S_17MS   = 0x0068  # 32 x 12-bit shunt samples averaged together
	__INA219_CONFIG_SADCRES_12BIT_64S_34MS   = 0x0070  # 64 x 12-bit shunt samples averaged together
	__INA219_CONFIG_SADCRES_12BIT_128S_69MS  = 0x0078  # 128 x 12-bit shunt samples averaged together

	__INA219_CONFIG_MODE_MASK                = 0x0007  # Operating Mode Mask
	__INA219_CONFIG_MODE_POWERDOWN           = 0x0000
	__INA219_CONFIG_MODE_SVOLT_TRIGGERED     = 0x0001
	__INA219_CONFIG_MODE_BVOLT_TRIGGERED     = 0x0002
	__INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED = 0x0003
	__INA219_CONFIG_MODE_ADCOFF              = 0x0004
	__INA219_CONFIG_MODE_SVOLT_CONTINUOUS    = 0x0005
	__INA219_CONFIG_MODE_BVOLT_CONTINUOUS    = 0x0006
	__INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS = 0x0007


# ===========================================================================

# ===========================================================================
#   SHUNT VOLTAGE REGISTER (R)
# ===========================================================================
	__INA219_REG_SHUNTVOLTAGE                = 0x01
# ===========================================================================

# ===========================================================================
#   BUS VOLTAGE REGISTER (R)
# ===========================================================================
	__INA219_REG_BUSVOLTAGE                  = 0x02
# ===========================================================================
	__INA219_BUSVOLTAGE_CNVR		= 0x0002 # Conversion ready
	__INA219_BUSVOLTAGE_OVERFLOW		= 0x0001 # Math overflow

# ===========================================================================
#   POWER REGISTER (R)
# ===========================================================================
	__INA219_REG_POWER                       = 0x03
# ===========================================================================

# ==========================================================================
#    CURRENT REGISTER (R)
# ===========================================================================
	__INA219_REG_CURRENT                     = 0x04
# ===========================================================================

# ===========================================================================
#    CALIBRATION REGISTER (R/W)
# ===========================================================================
	__INA219_REG_CALIBRATION                 = 0x05
# ===========================================================================

	# Constructor
	def __init__(self, address=0x40, i2c=None, resistance=.1, debug=False, **kwargs):
		if i2c is None:
			import Adafruit_GPIO.I2C as I2C
			i2c = I2C
		self._device = i2c.get_i2c_device(address, **kwargs)
		self.address = address
		self.debug = debug

		self.reset()
		self.ina219SetCalibration(resistance, gain=4)

	def twosToInt(self, val, len):
		# Convert twos compliment to integer

		if(val & (1 << len - 1)):
			val = val - (1<<len)

		return val

	def dumpRegisters(self):
                for reg in range(0,6):
                        result = self._device.readU16(reg, little_endian=False)
                        print(reg),
                        print(format(result,'04x'))
		

	def reset(self):
		config  = self.__INA219_CONFIG_RESET

		bytes = [(config >> 8) & 0xFF,  config & 0xFF]
		self._device.writeList(self.__INA219_REG_CONFIG, bytes)

	def ina219SetCalibration(self, resistance, gain=8, voltageRange=32, maxAmps=4):
		self.currentLSB = float(maxAmps)/pow(2,15)
		self.gain = gain
		
		# Set Calibration register to 'Cal' calculated above	
		calibration = int(0.04096/(self.currentLSB*resistance))

		bytes = [(calibration >> 8) & 0xFF,  calibration & 0xFF]
		self._device.writeList(self.__INA219_REG_CALIBRATION, bytes)

		config = 0;

		if (voltageRange == 16):
			config |= self.__INA219_CONFIG_BVOLTAGERANGE_16V
		if (voltageRange == 32):
			config |= self.__INA219_CONFIG_BVOLTAGERANGE_32V

		if (gain == 1):
			config |= self.__INA219_CONFIG_GAIN_1_40MV
		elif (gain == 2):
			config |= self.__INA219_CONFIG_GAIN_2_80MV
		elif (gain == 4):
			config |= self.__INA219_CONFIG_GAIN_4_160MV
		elif (gain == 8):
			config |= self.__INA219_CONFIG_GAIN_8_320MV

		# Slowest, most accurate bus ADC sampling
		config |= self.__INA219_CONFIG_BADCRES_12BIT_128S_69MS

		# Slowest, most accurate current sampling
		config |= self.__INA219_CONFIG_SADCRES_12BIT_128S_69MS

		# and run in continuous mode
		#config |= self.__INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS
		#config |= self.__INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED

		self.config = config

		
		bytes = [(config >> 8) & 0xFF, config & 0xFF]
		self._device.writeList(self.__INA219_REG_CONFIG, bytes)


	def measure(self):
		# Trigger a measurement by writing out the config register
		config = self.config | self.__INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED

		bytes = [(config >> 8) & 0xFF, config & 0xFF]
		self._device.writeList(self.__INA219_REG_CONFIG, bytes)

		# then wait for it to complete
		while(True):
			result = self._device.readU16(self.__INA219_REG_BUSVOLTAGE, little_endian=False)

			if (result & self.__INA219_BUSVOLTAGE_CNVR):
				break
			

	def getBusVoltage_raw(self):
		result = self._device.readU16(self.__INA219_REG_BUSVOLTAGE, little_endian=False)

		if (result & self.__INA219_BUSVOLTAGE_OVERFLOW):
			print("Error- conversion overflow"),
		
		# Shift to the right 3 to drop CNVR
		return (result >> 3) 
		
	def getShuntVoltage_raw(self):
		result = self._device.readList(self.__INA219_REG_SHUNTVOLTAGE,2)
		if (result[0] >> 7 == 1):
			testint = (result[0]*256 + result[1])
			othernew = self.twosToInt(testint, 16)
			return othernew
		else:
			return (result[0] << 8) | (result[1])

	def getCurrent_raw(self):
		result = self._device.readList(self.__INA219_REG_CURRENT,2)
		if (result[0] >> 7 == 1):
			testint = (result[0]*256 + result[1])
			othernew = self.twosToInt(testint, 16)
			return othernew
		else:
			return (result[0] << 8) | (result[1])

	def getPower_raw(self):
		result = self._device.readList(self.__INA219_REG_POWER,2)
		if (result[0] >> 7 == 1):
			testint = (result[0]*256 + result[1])
			othernew = self.twosToInt(testint, 16)
			return othernew
		else:
			return (result[0] << 8) | (result[1])

	def getShuntVoltage_mV(self):
		value = self.getShuntVoltage_raw()
		return value * 0.01
		
	def getBusVoltage_V(self):
		value = self.getBusVoltage_raw()
		return value * 0.004
		
	def getCurrent_mA(self):
		valueDec = self.getCurrent_raw()
		valueDec *= self.currentLSB*1000
		return valueDec
		
	def getPower_mW(self):
		valueDec = self.getPower_raw()
		valueDec *= self.currentLSB*20
		return valueDec



if __name__ == '__main__':
	ina = INA219()
	print(ina.getCurrent_mA())
