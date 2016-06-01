# LightBuddy LED controller

## Firmware development

The ARM toolchain 'arm-none-eabi-gcc' is used to compile this project. Specifically, the 4.8-2014-q2-update version. Get it here:

https://launchpad.net/gcc-arm-embedded/4.8/4.8-2014-q2-update

You'll also need dfu-util. Homebrew is possibly the easiest way to get this.

You'll also need GNU Make, which probably requires Xcode on OS/X, or similar developer tools on Linux. The version is probably not as important.

Note: For windows, try following steps 1 and 2 from this site to get up and running:
http://thehackerworkshop.com/?p=391

Once you have the toolchain installed, change to the firmware-gcc directory:

    cd firmware-gcc

and run make to compile:

    make

Use dfu-util (TODO: instructions for installing dfu-util!) to install the new firmware:

    make install

