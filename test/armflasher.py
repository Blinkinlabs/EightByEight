import subprocess

class ArmFlasher():
	""" Some convenience functions for flashing the ARM """

	def __init__(self):
		pass

	def eraseFlash(self):
		configfile = "openocd.cfg"

		args = ["openocd","-f",configfile,
			"-c", '"init"',
			"-c", "kinetis mdm mass_erase",
			"-c", "reset halt",
			"-c", "kinetis disable_wdog",
			"-c", "reset halt",
			"-c", "exit"
			]
		

		return subprocess.call(args) == 0

	def writeFirmware(self, imageFile):
		configfile = "openocd.cfg"

		args = ["openocd","-f",configfile,
			"-c", '"init"',
			"-c", "reset halt",
			"-c", "kinetis disable_wdog",
			"-c", "reset halt",
			"-c", "flash write_image %s"%(imageFile),
			"-c", "reset",
			"-c", "exit"
			]
		

		return subprocess.call(args) == 0



if __name__ == '__main__':
	armFlasher = ArmFlasher()
	print(armFlasher.eraseFlash())
	print(armFlasher.writeFirmware("../bootloader/blinky-boot.elf"))
	print(armFlasher.writeFirmware("../firmware/app-image.hex"))
