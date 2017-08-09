# EightByEight Blinky Test Program

The factory test uses a Raspberry Pi 

Setup instructions for 'Raspbian Jessie Lite'

# Set a unique hostname

	echo 'testrig-003' | sudo tee /etc/hostname

# Get the system up to date

	sudo apt-get update
	sudo apt-get install build-essential git libtool automake libusb-1.0.0-dev python-smbus python-serial i2c-tools python3-gpiozero python-gpiozero vim screen telnet python3 python3-pip python-dev python-pip gcc-arm-none-eabi dfu-util


# Enable I2c, disable serial

	sudo raspi-config

then:
	
	Select '9 Advanced Options'
	Select 'A6 I2C'
	Select 'yes'
	Select '9 Advanced Options'
	Select 'A? Serial'
	Select 'no'
	Select yes to reboot now

# Configure the RTC (not working?)

	sudo su
	modprobe rtc-ds1307
	echo ds1307 0x68 > /sys/class/i2c-adapter/i2c-1/new_device

	sudo hwclock -w

	sudo nano /etc/modules
	
add:

	rtc-ds1307
	i2c_bcm2708

vi  /lib/systemd/system/rtc-i2c.service

	[init]
	Description=Initialize i2c hardware RTC device driver
	DefaultDependencies=no
	Requires=systemd-modules-load.service
	After=systemd-modules-load.service
	Before=sysvinit.target
	ConditionPathExists=/sys/class/i2c-adapter
	Conflicts=shutdown.target
	
	[Service]
	Type=oneshot
	RemainAfterExit=yes
	EnvironmentFile=/etc/conf.d/rtc-i2c
	ExecStart=/bin/sh -c "echo ${CHIP} ${ADDRESS} > /sys/class/i2c-adapter/i2c-${BUS}/new_device"
	
	[Install]
	WantedBy=sysinit.target



mkdir /etc/conf.d
vi /etc/conf.d/rtc-i2c

	# /etc/conf.d/rtc-i2c
	CHIP="ds1307"
	ADDRESS="0x68"
	BUS="1"

then:

	sudo systemctl enable rtc-i2c.service 
	sudo apt-get remove fake-hwclock 

	sudo update-rc.d -f fake-hwclock remove 

# Configure a wifi hotspot

Follow: https://frillip.com/using-your-raspberry-pi-3-as-a-wifi-access-point-with-hostapd/
   

	ap: twosigma-blinky
	pw: blinkblinkblink

# Install dfu (optional)

TODO: We actually need .7 :-(

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
	git checkout 406f4d1c68330e3bf8d9db4e402fd8802a5c79e2	
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
                
then:

	./bootstrap
	./configure --enable-sysfsgpio --enable-bcm2835gpio --enable-ftdi
	make
	sudo make install

# Install esptool

git clone https://github.com/themadinventor/esptool.git

# Install python libs

	sudo pip install Adafruit-GPIO adafruit-ads1x15


# Install this repository

	git clone https://github.com/Blinkinlabs/EightByEight.git


# Fix monitor resolution

Only needed for 8" hdmi monitor

sudo vi /boot/config.txt

	# force 800x600
	hdmi_group=2
	hdmi_mode=9
	display_rotate=3


# Configure to start at boot

Run in the terminal:

	vi ~/.bashrc

Add add to the end:

	if [ -z "$SSH_CLIENT" ]; then
		setterm -blank 0
		clear
		cd /home/pi/EightByEight/production/test/; sudo ./run-eightbyeight-tests.sh
	fi

And set up the rgbw test pattern generator:

	sudo cp spamrgb.service /etc/systemd/system
	sudo systemctl enable spamrgb.service

# Rejoice!

