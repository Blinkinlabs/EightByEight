# EightByEight Blinky Test Program

The factory test uses a Raspberry Pi 

Setup instructions for 'Raspbian Jessie Lite'

# Get the system up to date

	sudo apt-get update
	sudo apt-get install build-essential git libtool automake libusb-1.0.0-dev python-smbus python-serial i2c-tools python3-gpiozero python-gpiozero vim screen telnet python3 python3-pip python-dev python-pip gcc-arm-none-eabi dfu-uitl


# Enable I2c, disable serial

sudo raspi-config
Select '9 Advanced Options'
Select 'A6 I2C'
Select 'yes'
Select 'A? Serial'
Select 'no'
Select yes to reboot now

# Configure a wifi hotspot

Follow: https://frillip.com/using-your-raspberry-pi-3-as-a-wifi-access-point-with-hostapd/
   

	ap: twosigma-blinky
	pw: blinkblinkblink

# Install dfu (optional)

Install from git, since we need version .9 (why?)

	git clone git://git.code.sf.net/p/dfu-util/dfu-util
	cd dfu-util
	./autogen.sh
	./configure
	make
	sudo make install


# Install openocd

	git clone git://git.code.sf.net/p/openocd/code openocd-code
	cd openocd-code/

make this modification:

diff --git a/src/flash/nor/kinetis.c b/src/flash/nor/kinetis.c
index aed37b9..8ab4c11 100644
--- a/src/flash/nor/kinetis.c
+++ b/src/flash/nor/kinetis.c
@@ -1479,6 +1479,9 @@ static int kinetis_probe(struct flash_bank *bank)
        }
 
        /* when the PFLSH bit is set, there is no FlexNVM/FlexRAM */
+
+       fcfg2_pflsh = true;
+
        if (!fcfg2_pflsh) {
                switch (fcfg1_nvmsize) {
                case 0x03:
                

./bootstrap
./configure --enable-sysfsgpio --enable-bcm2835gpio
make
make install

# Install esptool

git clone https://github.com/themadinventor/esptool.git

# Install python libs

	sudo pip install adafruit-ads1x15
	sudo pip install 

# Install this repository

	git clone https://github.com/Blinkinlabs/EightByEight.git


# Fix monitor resolution

Only needed for 8" hdmi monitor

sudo vi /boot/config.txt

	# force 800x600
	hdmi_group=2
	hdmi_mode=9


# Configure to start at boot

Run in the terminal:

	vi ~/.bashrc


cd /home/pi/EightByEight/test/; sudo ./run-eightbyeight-tests.sh


And set up the rgbw test pattern generator:

	sudo cp spamrgb.service /etc/systemd/system
	sudo systemctl enable spamrgb.service

# Rejoice!

