#!/bin/sh
INCLUDE="-I/usr/include/PCSC -I."
# LIB= "-L/usr/lib"
# #ttps://transang.me/library-path-in-gcc/
rm *.o
rm racs2
# gcc $INCLUDE -O2 -Wall  main.c -o main.o mutuex.c -o mutuex.o  pcsc.c -o pcsc.o pcscemulator.c -o pcscemulator.c  grid.c -o grid.o serverk.c -o serverk.o windowglue.c -o windowglue.o
gcc -c -O2 -Wall ./main.c           -o  ./main.o $INCLUDE
gcc -c -O2 -Wall ./reentrant2.c     -o ./reentrant2.o $INCLUDE
gcc -c -O2 -Wall ./common2.c        -o ./common2.o $INCLUDE
gcc -c -O2 -Wall ./pcsc.c           -o ./pcsc.o   $INCLUDE
gcc -c -O2 -Wall ./atr.c            -o ./atr.o    $INCLUDE
gcc -c -O2 -Wall ./pcscemulator.c   -o ./pcscemulator.o $INCLUDE
gcc -c -O2 -Wall ./grid.c           -o ./grid.o       $INCLUDE
gcc -c -O2 -Wall ./serverk.c        -o ./serverk.o    $INCLUDE
gcc -c -O2 -Wall ./server6.c        -o ./server6.o    $INCLUDE
gcc -c -O2 -Wall ./windowglue.c     -o ./windowglue.o $INCLUDE
gcc -c -O2 -Wall ./i2c.c            -o ./i2c.o        $INCLUDE
gcc -c -O2 -Wall ./i2cmod.c         -o ./i2cmod.o     $INCLUDE


# gcc -o racs2 ./main.o  ./pcsc.o ./atr.o ./pcscemulator.o ./grid.o ./serverk.o ./windowglue.o  ./common2.o  ./reentrant2.o ./server6.o ./i2c.o ./i2cmod.o $LIB -lpcsclite -lpthread -lssl -lcrypto -lwiringPi
gcc -o racs2 ./main.o  ./pcsc.o ./atr.o ./pcscemulator.o ./grid.o ./serverk.o ./windowglue.o  ./common2.o  ./reentrant2.o ./server6.o ./i2c.o ./i2cmod.o -lpcsclite -lpthread -lssl -lcrypto -lwiringPi

cp ./racs2 ./../racs2



