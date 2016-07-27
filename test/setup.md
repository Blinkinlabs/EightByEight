# EightByEight Blinky Test Program

The factory test uses a Raspberry Pi 

Setup instructions for 2016 raspian minimal:

# Get the system up to date

sudo apt-get update
sudo apt-get install build-essential git libtool automake

# Install openocd

git clone git://git.code.sf.net/p/openocd/code openocd-code
cd openocd-code/
./bootstrap
./configure

# Install this repository

git clone ...

# Configure to start at boot

sudo ln -s EightByEight/test/tester.service /etc/systemd/system
sudo systemctl enable tester.service






(old)

sudo apt-get update
sudo apt-get install nap python vim git build-essential python-dev python-setuptools python-pip python-smbus -y

(disable HDMI in /boot/uEnv.txt and reboot)

sudo easy_install -U distribute
sudo pip install Adafruit_BBIO

sudo apt-get install python-unittest

git clone ...
cd flybynight
sudo cp flybynight.service /etc/systemd/system/
sudo systemctl enable flybynight.service
