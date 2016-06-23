import serial
import time

dut = serial.Serial('/dev/serial/by-id/usb-Blinkinlab_EightByEight_NNYUKETXDPPYQWRN-if00', baudrate=115200)

while True:
	time.sleep(.02)
	dut.setDTR(True)
	time.sleep(.01)
	dut.setDTR(False)
	dut.setRTS(True)
	dut.setRTS(False)
