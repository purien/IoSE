LINUX install notes

//install PC/SC
=======================================================
sudo apt-get install libpcsclite1 pcscd
sudo apt-get install libpcsclite-dev libusb-dev
sudo apt-get install pcsc-tools
// PC/SC TEST
usr/bin/pcsc_scan  pcsc-spy
=======================================================


// Compile OPENSSL l-1.0.2u
=======================================================
mkdir openssl
cd openssl
wget https://www.openssl.org/source/openssl-1.0.2u.tar.gz
tar xzvf openssl-1.0.2u.tar.gz
cd openssl-1.0.2u
./config
make
make test
sudo make install
// /include and /lib are in usr/local/ssl/
========================================================


// Enable I2C for Raspberry pi
========================================================
sudo apt-get install i2c-tools
sudo nano /boot/config.txt
// in /boot/config.txt find the line containing “dtparam=i2c_arm=on”.
// Add “,i2c_arm_baudrate=200000” where 200000 is the new speed (200 Kbit/s).
// This should give you a line looking like:
dtparam=i2c_arm=on,i2c_arm_baudrate=200000
// Raspberry has 4 GPUs fix the frequency in order to avoid I2C clock dynamic modification
core_freq=250
core_freq_min=250
// I2C test
sudo i2cdetect -y 1
=========================================================


// WiringPi for Pasperry pi
========================================================
// http://wiringpi.com/news/
cd /tmp
wget https://unicorn.drogon.net/wiringpi-2.46-1.deb
sudo dpkg -i wiringpi-2.46-1.deb
// TEST
gpio -v
gpio readall
========================================================


// Raspberry PU firmware update
========================================================
sudo rpi-update
========================================================

// gcc install
============================================================================
// The command installs a bunch of new packages including gcc, g++ and make.
sudo apt install build-essential
// only dor doc  
sudo apt-get install manpages-dev  
//Test 
gcc --version
============================================================================


// Update Ubuntu System
================================================
sudo apt-get update && sudo apt-get upgrade
// Check UBUNTU OPENSSL version
openssl version -a
================================================