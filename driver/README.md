= Build a DFU driver for the BlinkyTile bootloader

Start with instructions here:
https://github.com/libusbx/libusbx/wiki/Windows-Backend#Driver_Installation

use this set for reference:
http://libusb-winusb-wip.googlecode.com/files/winusb%20driver.zip

Modify the 'libusb_device_multiple_interfaces_0' example to include the EightByEight runtime interface.
Modify the 'libusb_device' example to include the EightByEight interface

 

= Updating firmware on Windows

Get dfu-util 0.8 from the web site:
http://dfu-util.sourceforge.net/releases/dfu-util-0.8-binaries/win32-mingw32/
