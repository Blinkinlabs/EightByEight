import sys
sys.path.append('/home/pi/esptool')

import esptool
from timeout import timeout


class EspFlasher():
	""" Some convenience functions for flashing the ESP """

	def __init__(self, port="/dev/ttyACM0", baud=115200):
		self.port = port
		self.baud = baud

	def connect(self):
		device = esptool.ESPROM(self.port, self.baud)
		device.connect()
		return device

	def readChipInfo(self):
		device = self.connect()

		info = {}
		info["mac"] = "%02x:%02x:%02x:%02x:%02x:%02x" % device.read_mac()
		info["chip_id"] = device.chip_id()
		info["flash_id"] = device.flash_id()

		return info

	def writeFirmware(self, address, filename):
		device = self.connect()

		class argsStub():
			def __init__(self, address, filename):
				self.flash_mode = 'qio'
				self.flash_size = '4m'
				self.flash_freq='40m'
				self.baud=115200
				self.no_progress=False
				self.operation='write_flash'
				self.verify=False
				self.addr_filename = []
				argfile = open(filename, 'rb')
				self.addr_filename.append((address,argfile))

		args = argsStub(address, filename)
		return esptool.write_flash(device, args)

if __name__ == '__main__':

	flasher = EspFlasher()

	flasher.connect()
	print(flasher.readChipInfo())
	flasher.writeFirmware(0, "/home/pi/EightByEight/bin/MegaDemo.bin")
