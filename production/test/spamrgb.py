#!/usr/bin/python

# Spam RGB test pattern over UDP
import socket
import fcntl
import struct
import time

UDP_PORT = 5453

colors = (
(255,0,0),
(0,255,0),
(0,0,255),
(255,255,255)
)

# from:
# https://www.quora.com/Which-Python-library-allows-us-to-specify-network-interface-to-connect-to-the-Internet
def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,
        struct.pack('256s', ifname[:15])
    )[20:24])

def sendColor(color):
    MESSAGE = ""
    MESSAGE += chr(0x50) #Magic
    MESSAGE += chr(0x4c)
    MESSAGE += chr(0x53)
    MESSAGE += chr(0x54)
    MESSAGE += chr(0x00) # blend time
    MESSAGE += chr(0x00)
    MESSAGE += chr(0x00)
    MESSAGE += chr(0x00)

    for pixel in range(0,8*8):
        MESSAGE += chr(color[0])
        MESSAGE += chr(color[1])
        MESSAGE += chr(color[2])

    s.sendto(MESSAGE, ('<broadcast>', UDP_PORT))

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((get_ip_address("wlan0"), 0))
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)



while True:
    for color in colors:
        sendColor(color)
        time.sleep(1)
