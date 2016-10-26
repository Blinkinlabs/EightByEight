import Adafruit_BBIO.SPI as SPI
import Adafruit_BBIO.GPIO as GPIO
import time

# Device signature bytes
ATTINY25_SIGNATURE= [0x1E, 0x91, 0x08]
ATTINY45_SIGNATURE= [0x1E, 0x92, 0x06]

# Amount of time, in seconds, to wait for a flash erase to complete
ATTINY_FLASH_DELAY = .01
ATTINY_ERASE_DELAY = .01
ATTINY_FUSE_DELAY = .01

# Page sizes, in words
ATTINY25_PAGE_SIZE = 16
ATTINY45_PAGE_SIZE = 32

class AvrISP:
	def __init__(self, deviceType, spiBus, resetPin):
		self.reset = resetPin
		GPIO.setup(self.reset, GPIO.IN)
		
		self.spi = SPI.SPI(spiBus,1)
		self.spi.open(1,1)
		self.spi.mode=0
		self.spi.msh = 100000

		if(deviceType == 'ATTINY25'):
			self.signature = ATTINY25_SIGNATURE
			self.pageSize = ATTINY25_PAGE_SIZE
		elif(deviceType == 'ATTINY45'):
			self.signature = ATTINY45_SIGNATURE
			self.pageSize = ATTINY45_PAGE_SIZE
		else:
			print("Unsupported device type!")


	def open(self):
		GPIO.setup(self.reset, GPIO.OUT)
		GPIO.output(self.reset, GPIO.HIGH)
		time.sleep(.01)
		GPIO.output(self.reset, GPIO.LOW)
		time.sleep(.01)

	def programmingEnable(self):
		response = self.spi.xfer2([0xAC, 0x53, 0x00, 0x00])
		return (response[2] == 0x53)

	def checkSignature(self):
		# For ATTiny25
		SIGNATURE_BYTE_0 = 0x1E
		SIGNATURE_BYTE_1 = 0x92
		SIGNATURE_BYTE_2 = 0x93

		responses = []
		responses += self.spi.xfer([0x30,0x00,0x00,0x00])
		responses += self.spi.xfer([0x30,0x00,0x01,0x00])
		responses += self.spi.xfer([0x30,0x00,0x02,0x00])
		
		return ((responses[3] == self.signature[0]) and (responses[7] == self.signature[1]) and (responses[11] == self.signature[2]))

	def eraseChip(self):
		response = self.spi.xfer2([0xAC, 0x80, 0x00, 0x00])

		if response[2] != 0x80:
			return False

		time.sleep(ATTINY_ERASE_DELAY)
		GPIO.output(self.reset, GPIO.HIGH)
		time.sleep(.01)
		GPIO.output(self.reset, GPIO.LOW)
		time.sleep(.01)

		return True

	def writePage(self, address, data):
		if (address % self.pageSize) != 0:
			print("address does not fall on a page boundary!")
			return False

		if len(data) != self.pageSize*2:
			print("data size not equal to page size!")
			return False

		for i in range(0,self.pageSize):
			self.spi.xfer2([0x40, (((address+i) >> 8) & 0xFF), ((address+i) & 0xFF), data[i*2]])
			self.spi.xfer2([0x48, (((address+i) >> 8) & 0xFF), ((address+i) & 0xFF), data[i*2+1]])
			

		self.spi.xfer2([0x4C, ((address >> 8) & 0xFF), (address & 0xFF), 0x0A0])
		time.sleep(ATTINY_FLASH_DELAY)
		return True

	def readWord(self, address):
		word = []
		word.append(self.spi.xfer2([0x20, ((address >> 8) & 0xFF), (address & 0xFF), 0x00])[3])
		word.append(self.spi.xfer2([0x28, ((address >> 8) & 0xFF), (address & 0xFF), 0x00])[3])
		return word

	def writeVerifyPage(self, address, data):
		if not self.writePage(address, data):
			return False

		readData = []
		for i in range(0,self.pageSize):
			readData += self.readWord(address+i)

		if(data != readData):
			print("data verify error at address 0x%x"%(address))
			print("Expected: ", data)
			print("Got: ", readData)
			return False

		return True

	def readFuse(self, fuse):
		if(fuse == 'lock'):
			return self.spi.xfer2([0x58, 0x00, 0x00, 0x00])[3]

		elif(fuse == 'low'):
			return self.spi.xfer2([0x50, 0x00, 0x00, 0x00])[3]

		elif(fuse == 'high'):
			return self.spi.xfer2([0x58, 0x08, 0x00, 0x00])[3]
		
		elif(fuse == 'efuse'):
			return self.spi.xfer2([0x50, 0x08, 0x00, 0x00])[3]

	def writeFuse(self, fuse, value):
		if(fuse == 'lock'):
			self.spi.xfer2([0xAC, 0xE0, 0x00, value])

		elif(fuse == 'low'):
			self.spi.xfer2([0xAC, 0xA0, 0x00, value])

		elif(fuse == 'high'):
			self.spi.xfer2([0xAC, 0xA8, 0x00, value])
		
		elif(fuse == 'efuse'):
			self.spi.xfer2([0xAC, 0xA4, 0x00, value])

		time.sleep(ATTINY_FUSE_DELAY)

	def writeAndVerifyFuse(self, fuse, value):
		self.writeFuse(fuse, value)

		newValue = self.readFuse(fuse)

		if (value != newValue):
			print("Fuse value mismatch, expected 0x%x, got 0x%x"%(value, newValue))
			return False

		return True

	def close(self):
		GPIO.setup(self.reset, GPIO.IN)
		self.spi.close()

if __name__ == '__main__':
	DUT_0_RADIO_CSN = "P8_14"
	DUT_0_RADIO_CE = "P8_16"
	DUT_0_RESET = "P9_42"

	icp = AvrISP("ATTINY25", 0, DUT_0_RESET)
	icp.open()

	GPIO.setup(DUT_0_RADIO_CE, GPIO.OUT)
	GPIO.output(DUT_0_RADIO_CE, GPIO.HIGH)
	GPIO.setup(DUT_0_RADIO_CSN, GPIO.OUT)
	GPIO.output(DUT_0_RADIO_CSN, GPIO.HIGH)

	icp.programmingEnable()
	if not icp.checkSignature():
		print("Invalid signature")
		exit(1)

	icp.eraseChip()

	icp.programmingEnable()
	if not icp.checkSignature():
		print("Invalid signature")
		exit(1)

	data = []
	for i in range(0,16):
		data.append(i)

	print(icp.writeVerifyPage(0, data))

	icp.close()
