#!/bin/sh
INCLUDE="-I/usr/include/PCSC -I./include -I."

rm *.o
rm racs2

gcc -c -O2 -Wall $INCLUDE ./main.c         -o  ./main.o 
gcc -c -O2 -Wall $INCLUDE ./reentrant2.c   -o ./reentrant2.o 
gcc -c -O2 -Wall $INCLUDE ./common2.c      -o ./common2.o
gcc -c -O2 -Wall $INCLUDE ./pcsc.c         -o ./pcsc.o   
gcc -c -O2 -Wall $INCLUDE ./atr.c          -o ./atr.o    
gcc -c -O2 -Wall $INCLUDE ./pcscemulator.c -o ./pcscemulator.o 
gcc -c -O2 -Wall $INCLUDE ./grid.c         -o ./grid.o      
gcc -c -O2 -Wall $INCLUDE ./serverk.c      -o ./serverk.o   
gcc -c -O2 -Wall $INCLUDE ./server6.c      -o ./server6.o   
gcc -c -O2 -Wall $INCLUDE ./windowglue.c   -o ./windowglue.o 
gcc -c -O2 -Wall $INCLUDE ./i2c.c          -o ./i2c.o       
gcc -c -O2 -Wall $INCLUDE ./i2cmod.c       -o ./i2cmod.o    


gcc -o racs2 ./main.o  ./pcsc.o ./atr.o ./pcscemulator.o ./grid.o ./serverk.o ./windowglue.o  ./common2.o  ./reentrant2.o ./server6.o ./i2c.o ./i2cmod.o -lpcsclite -lpthread -L. -lmssl -lmcrypto -ldl 

cp ./racs2 ./../racs2
