import serial
import time

#dut = serial.Serial('/dev/serial/by-id/usb-Blinkinlab_EightByEight_NNYUKETXDPPYQWRN-if00', baudrate=115200)
dut = serial.Serial('/dev/tty.usbmodem1A1231', baudrate=15200)

while True:
	dut.setDTR(True)
	dut.setRTS(True)
	time.sleep(.02)
	dut.setRTS(False)
	time.sleep(.1)
	dut.setDTR(False)
	time.sleep(1)
