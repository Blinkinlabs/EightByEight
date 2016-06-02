import serial

blinky = serial.Serial("/dev/tty.usbmodem1411", 115200)

rows = 8
cols = 8

ledCommand = ""
ledCommand += chr(0xde)
ledCommand += chr(0xad)
ledCommand += chr(0)    # Length 0
ledCommand += chr(1+rows*cols*3) # Length 1
ledCommand += chr(0) # Command: 0 = show color
for row in range(0, rows):
    for col in range(0, cols):
        ledCommand += chr(100)
        ledCommand += chr(10)
        ledCommand += chr(50)
ledCommand += chr(0)    # CRC

print(len(ledCommand))
print(blinky.write(ledCommand))
