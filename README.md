videotest.cpp

This is an example program that automatically takes the first frame and streams data from the endpoint then calculates the FPS. Only bulk transfer is supported. This application do not contain a decoder so RAW format is also supported. The Cypress UVC device should have VID:0x04b4&PID:0x00C3

Tested Platform: RaspberryPi 400

Author:   Eddie Zhao

Version : 0.1

Date:     7/12/2021

Build:

sudo apt-get install libusb-1.0-0-dev

git clone http://github.com/zhao1mh/videotest.git

./configure

make
Cancel changes

