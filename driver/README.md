# Build a DFU driver for the BlinkyTile bootloader

Start with instructions here:
https://github.com/libusbx/libusbx/wiki/Windows-Backend#Driver_Installation

use this set for reference:
http://libusb-winusb-wip.googlecode.com/files/winusb%20driver.zip

Modify the 'libusb_device_multiple_interfaces_0' example to include the EightByEight runtime interface.
Modify the 'libusb_device' example to include the EightByEight interface

 

# Updating firmware on Windows

Get dfu-util 0.8 from the web site:
http://dfu-util.sourceforge.net/releases/dfu-util-0.8-binaries/win32-mingw32/


# Signing

Instructions based on: [http://www.davidegrayson.com/signing/](http://www.davidegrayson.com/signing/)

First, get a GlobalSign code signing signature, and install the .cer file on the machine that will be used for development.


Next, install the GlobalSign R1-R3 cross certificate on the machine. It can be obtained from here:

	https://jp.globalsign.com/support/docs/r1cross.cer

Third, download the Microsoft Globalsign Cross certificate, and place it in this driver/ directory. It can be obtained from here:

	http://go.microsoft.com/fwlink/?LinkId=321777

Finally, rrom a Windows command prompt, run the included script:

	driver_signer.bat


