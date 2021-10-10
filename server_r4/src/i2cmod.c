/* i2cmod.c */
/* Copyright (C) 2021 Pascal Urien (pascal.urien@gmail.com)
 * All rights reserved.
 *
 * This software is an implementation of the internet draft
 * https://tools.ietf.org/html/draft-urien-core-racs-00
 * "Remote APDU Call Secure (RACS)" by Pascal Urien.
 * https://datatracker.ietf.org/doc/html/draft-urien-coinrg-iose-00
 * "Internet of Secure Elements" by Pascal Urien
 * The implementation was written so as to conform with these drafts.
 * 
 * This software is free for non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution.
 * 
 * Copyright remains Pascal Urien's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Pascal Urien should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes RACS-Server software written by
 *     Pascal Urien (pascal.urien@gmail.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY PASCAL URIEN ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */




#include "i2cmod.h"

int i2c_fdebug=0;

#ifdef NOI2C
#else

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>			//Needed for I2C port
#include <fcntl.h>			//Needed for I2C port
#include <sys/ioctl.h>		//Needed for I2C port
#include <linux/i2c-dev.h>	//Needed for I2C port
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <wiringPi.h>

extern int gPrintf(int id,char *fmt, ...);

#define SCR 0

#include "common2.h"
#include "reentrant2.h"
#include "mutuex.h"

uint32_t udelay(uint32_t tmax, uint32_t tmin)
{
if (tmax >= tmin) 
return tmax-tmin;

tmin =  ~tmin+ 1;
return tmax+tmin;
 
}

uint32_t micros() 
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint32_t)1000000+tv.tv_usec;
}

void adelay(int milliseconds)
{
	uint32_t tr=0, ta=0,td=0;
	td= 1000* milliseconds;
	
	tr = micros();
	ta=  micros();
	while (udelay(ta,tr)< td)
		ta=micros();
}



uint16_t crc16(char *data, int len)
{ int i,j;
  uint16_t current_crc_value = 0xFFFF ;
  for (i = 0; i < len; i++ )
  {
    current_crc_value ^= ((uint16_t)data[i] & 0xFF);
    for (j = 0; j < 8; j++)
    {
      if ((current_crc_value & 1) != 0)
      current_crc_value = (current_crc_value >> 1) ^ (uint16_t)0x8408;
      
      else
      current_crc_value = current_crc_value >> 1;
     }
  }
  current_crc_value = ~current_crc_value;

  return current_crc_value & 0xFFFF;
}

void myPrintf(char *str, uint8_t *vli, int size)
{ int i;
  char buf[128];

  if (size <= 0)    return ;

  sprintf(buf, "%s ", str);
  gPrintf(SCR,"%s\n",buf)   ;
  
  buf[0] = 0;
  for (i = 0; i < size; ++i)
  {
    sprintf(&buf[strlen(buf)], "%02X", (unsigned)vli[i]);
    if (i % 32 == 31)
    { gPrintf(SCR,"%s\n",buf);
      buf[0] = 0;
    }
  }

  i--;
  if ((i % 32) != 31)
    gPrintf(SCR,"%s\n",buf);
}



#define  MAXI2C 128
int i2c_ptrx2=0 ;
char bufi2c[300] ;
int error_i2c=0;
int error_pcb=0;
int error_crc=0;
int error_timeout=0;
int file_i2c=0;

int txi2c(char *buf, int len, uint8_t adr)
{ int err=0;
//file_i2c=0;
 ;
//----- OPEN THE I2C BUS -----

//har *filename = (char*)"/dev/i2c-1";

if (file_i2c <= 0)
	return -1;

MUTEX_LOCK(Pmutex[M_SYSTEM+2]);

/*

//if ((file_i2c = open(filename, O_RDWR)) < 0)
if ((file_i2c = open(filename, O_RDWR)) < 0)
{ MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
  if (i2c_fdebug) printf("Failed to open the i2c bus");
  error_i2c++;
  return -1;
}
*/

// ioctl(file_i2c,I2C_TIMEOUT,10000);
	
	
if (ioctl(file_i2c, I2C_SLAVE,(int)(adr & 0xFF)) < 0)
{
//close (file_i2c);
error_i2c++;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
if (i2c_fdebug) gPrintf(SCR,"Failed to acquire bus access and/or talk to slave.\n");
return -1;
}

err = write(file_i2c, buf, len);
//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
if (err != len)		
{
/* ERROR HANDLING: i2c transaction failed */
error_i2c++;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
if (i2c_fdebug) gPrintf(SCR,"Failed to write to the i2c bus (req %d read %d.\n",len,err);
//close (file_i2c);
return -1 ;
}

// close (file_i2c);
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);

return len;
	
}

int rxi2c(char *buf, int len, uint8_t adr)
{ //int file_i2c 
  int err=0;
//----- OPEN THE I2C BUS -----
//char *filename = (char*)"/dev/i2c-1";

if (file_i2c <= 0)
	return -1 ;

MUTEX_LOCK(Pmutex[M_SYSTEM+2]);

/*

if ((file_i2c = open(filename, O_RDWR)) < 0)
{ MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
  if (i2c_fdebug) printf("Failed to open the i2c bus");
  error_i2c++;
  return -1;
}

//if (len == 1) ioctl(file_i2c,I2C_TIMEOUT,1);
//else          ioctl(file_i2c,I2C_TIMEOUT,10000);
 
  */
	
if (ioctl(file_i2c, I2C_SLAVE,(int)(adr & 0xFF)) < 0)
{
//close (file_i2c);
error_i2c++;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
if (i2c_fdebug) gPrintf(SCR,"Failed to acquire bus access and/or talk to slave.\n");
return -1;
}


//read() returns the number of bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
err = read(file_i2c, buf, len) ;
if (err != len)		
{
//ERROR HANDLING: i2c transaction failed
// printf("Failed to read from the i2c bus.\n");
//close (file_i2c);
error_i2c++;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
if (i2c_fdebug) gPrintf(SCR,".[%d %d])",len,err);
return -1;
}


//close (file_i2c);
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);

return len;
	
}


int i2c_send(uint8_t i2c_adr, char pcb,int len,char *buf, int dt,int f_tx, int f_rx,char * bufi2c)
{ uint16_t mycrc=0;
  uint32_t tr=0,ts=0,trx=0,te=0;
  int err=0,toread;
  int i2c_ptrx2=0 ;
 
  buf[0]= 0x5A;

  if (f_tx)  
  {
  buf[1]= pcb ;
  buf[2]= 0xFF & len;
  mycrc= crc16(buf,len+3);
  buf[3+len]= 0xFF & mycrc ;
  buf[4+len]= 0xFF & (mycrc >> 8);

  if (i2c_fdebug)
  myPrintf("TxI2C",(uint8_t *)buf,5+len);
  
  int nb,pt=0;
  nb= 5+len;

  tr= micros();
  
  while(nb > 0)
  {
  if (nb > MAXI2C)
  {
  err = txi2c(buf+pt,MAXI2C,i2c_adr);
  //WIRE2.beginTransmission(i2c_adr);
  //WIRE2.write(buf+pt,MAXI2C);
  //c=WIRE2.endTransmission() ;
  nb-=MAXI2C;
  pt+=MAXI2C;
  }
  else
  {
  err = txi2c(buf+pt,nb,i2c_adr);
  // WIRE2.beginTransmission(i2c_adr);
  // WIRE2.write(buf+pt,nb);
  // c=WIRE2.endTransmission();
  
  nb=0;
  }
 
  if (err < 0) 
  { if (i2c_fdebug) {gPrintf(SCR,"Tx Error\n");}
    return -1;
  }
  }
  }

  ts= micros();

 if (!f_rx) 
 return 0;
 
int tt=0,dt0=10,ct=0;
int nb=0;
i2c_ptrx2=0;


while(nb == 0)
{
nb=1;
//WIRE2.requestFrom(i2c_adr,nb,true);
err= rxi2c(bufi2c,nb,i2c_adr);

while(err != nb) 
{ 
  //adelay(dt0);
  usleep(dt0*1000);
  tt += dt0  ;
  ct++;
  if (tt > dt) 
  { if (i2c_fdebug) gPrintf(SCR,"%s\n","I2C Timeout !");
    error_timeout++;
    return -1;
  }
  // WIRE2.requestFrom(i2c_adr,nb,true);
  err= rxi2c(bufi2c,nb,i2c_adr);
}

i2c_ptrx2++;
nb--;
if (bufi2c[i2c_ptrx2-1] == (char)0xA5) 
break;

i2c_ptrx2-- ;
// adelay(dt0) ;
usleep(dt0*1000);
tt += dt0   ;
ct++;
}

trx= micros();
nb=2;
//WIRE2.requestFrom(i2c_adr,nb,true);
err= rxi2c(bufi2c+1,nb,i2c_adr);

while(err != nb) 
{   
  //adelay(dt0);
  usleep(1000*dt0);
  tt += dt0;
  ct++;
  
  if (tt > dt) 
  { if (i2c_fdebug) gPrintf(SCR,"%s\n","I2C Timeout !");
    error_timeout++;
    return -1;
  }
  err= rxi2c(bufi2c,nb,i2c_adr);
  //WIRE2.requestFrom(i2c_adr,nb,true);
}

nb=0;
i2c_ptrx2+= 2;
nb= (0xFF & bufi2c[2]) + 2;

while (nb>0)
{

if (nb > MAXI2C)
{ toread = MAXI2C;}

else
{ toread=nb;}

err = rxi2c(bufi2c+i2c_ptrx2,toread,i2c_adr);
if (err < 0) return -1;
i2c_ptrx2 += toread;
nb -= toread;
}


te= micros();

mycrc = crc16(bufi2c,i2c_ptrx2-2);
if (i2c_fdebug) myPrintf("RxI2C",(uint8_t *)bufi2c,i2c_ptrx2);

if ( (bufi2c[i2c_ptrx2-2] != (char)(0xFF & mycrc) ) || (bufi2c[i2c_ptrx2-1] != (char)(0xFF & (mycrc>>8))) )
{ if (i2c_fdebug) gPrintf(SCR,"Error CRC !\n");
  error_crc++;
  return -1;
}

if (bufi2c[0] != (char)0xA5)
{ if (i2c_fdebug) gPrintf(SCR,"Error NAD !\n");
  error_pcb++;
  return -1;
}

// 1 0 0 N(R) 0 0 Error code
if ( ( (bufi2c[1] & (char)0xE0) == (char)0x80 ) && ( (bufi2c[1] & (char)0x3) != (char)0 ))
{ error_pcb++;
 if (i2c_fdebug) gPrintf(SCR,"%s\n","Error PCB");
 return -1;
}


if (i2c_fdebug)
{
gPrintf(SCR,"i2c_time: %d\n",udelay(te,tr));
gPrintf(SCR,"i2c_tx: %d\n",udelay(ts,tr));
gPrintf(SCR,"i2c_wait: %d\n",udelay(trx,ts));
gPrintf(SCR,"i2c_rx: %d\n",udelay(te,trx));
gPrintf(SCR,"loop_rx: %d(in unit of %dms)\n",ct,dt0);
}
return i2c_ptrx2;

}


int echo_i2c(uint8_t i2c_adr, int len)
{ int stat=0,i;
  char buf[256];
  for (i=0;i<len;i++) buf[i+3]= 0xFF & (i+1);
  stat=  i2c_send(i2c_adr,(char)0xC0,len,buf,5000,1,1,buf);
  return stat;
}



int I2C_Reset_Pin(int pin)
{  
  //if (i2c_fdebug)
  gPrintf(SCR,"Reset I2C PIN %d\n",pin);

  //digitalWrite (pin, HIGH) ; 
  //adelay (50) ;
  //usleep(10000);
  digitalWrite (pin,LOW)   ; 
  //adelay (50) ;
  usleep(10000);
  digitalWrite (pin, HIGH) ;
  //adelay(100);
  usleep(200000);
   
return 0;
}


extern int NBI2C;
extern int i2c_reset_pins[64] ;


int i2c_start()
{ int err,i;
  char *filename = (char*)"/dev/i2c-1";
  if ((file_i2c = open(filename, O_RDWR)) < 0)
  gPrintf(SCR,"Error..can't open i2c buf\n");

 if (getuid()) 
 { gPrintf(SCR, "%s", "not root\n");
   err= wiringPiSetupSys() ;
 }

 else 
 { gPrintf(SCR, "%s", "root\n");
   // err= wiringPiSetup () ;
    err= wiringPiSetupGpio();
   for(i=0;i<NBI2C;i++)
   { gPrintf(SCR,"Pin %d => output\n",i2c_reset_pins[i]);
     pinMode(i2c_reset_pins[i], OUTPUT) ;
     digitalWrite (i2c_reset_pins[i], HIGH) ;
   }
 }

  //err= wiringPiSetupSys() ;
   for(i=0;i<NBI2C;i++)
   digitalWrite (i2c_reset_pins[i], HIGH) ;
   usleep(200000);
   
   int k=0;
   
   for (k=0;k<NBI2C;k++)
   { for (i=0;i<5;i++) I2C_Reset_Pin (i2c_reset_pins[k])   ; 
   }

  return err;
}

#endif
