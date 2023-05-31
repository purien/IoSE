#!/bin/sh
sudo i2cdetect -y 1
sudo gpio export 16 out
sudo gpio export 20 out
sudo gpio export 21 out
sudo gpio export 19 out
sudo gpio export 26 out
sudo gpio export 13 out
sudo gpio export 6  out
sudo gpio export 5  out
sudo gpio export 12 out
