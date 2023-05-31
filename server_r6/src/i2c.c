/* i2c.c */
/* Copyright (C) 2021-2022 Pascal Urien (pascal.urien@gmail.com)
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

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <malloc.h>

#ifndef WIN32
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <netdb.h>
   #define DWORD long
   #include <stdint.h>
 
#else
  #include <winsock.h>
#endif

#include "i2cmod.h"
#include "i2c.h"


//MUTEX_LOCK(Pmutex[M_SYSTEM+2]);
//MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);

int NBI2C=0 ;
int I2CADR=0;
int i2c_reset_pins[128] ;
int i2c_decoder_pins[16];
int i2c_E_pins[4] ;
int nbdecoder=1   ;

int i2c_index[128];
int fdecoder=0;
int i2c_adr[128] ;

static char ATR[128][32] ;
static int  ATRlen[128]  ;

extern int NBSC ;


int is_i2c(int num)
{ if (NBI2C ==0)
  return 0 ;
  if ( (num>= NBSC) && (num<(NBI2C+NBSC)) )
	  return 1;
  return 0;
}

#ifdef NOI2C
int i2c_init(void)
{
	return NBI2C ;
}



int i2c_on(int nb)
{
	return 0;
}

int i2c_off(int nb)
{
return 0;
}

int i2c_atr(int nb, char *atr)
{  char Atr[] = {(char)0x3B,(char)0x06,(char)'k',(char)'e',(char)'y'};
   memmove(atr,Atr,5);
   sprintf(atr+5,"%03d",nb);

   return 8;
}

int i2c_senda(int nb, char* APDU, DWORD APDUlen, char* Response, DWORD* Rlen)
{
	*Rlen=2;
	Response[0]=(char)0x90;
    Response[1]=(char)0x00;
	return 0 ;
}

#else

extern int i2c_send(uint8_t i2c_adr, char pcb,int len,char *buf, int dt,int f_tx, int f_rx,char * bufi2c);
extern int I2C_Reset_Pin(int pin);
extern int i2c_start();
extern int i2c_adr[]  ;

int i2c_init(void)
{  int err,i;
   
   err= i2c_start();
   printf("WiringPi: %d\n",err);
   for(i=0;i<NBI2C;i++)
   // I2C_Reset_Pin(i2c_reset_pins[i]);
   I2C_Reset_Pin(i2c_reset_pins[i2c_adr[i]]);
   
return NBI2C ;
}

//int i2c_reset(int nb)
//{ return 0;}

int i2c_on(int nb)
{   uint8_t adr;
    int err,pin,i; 
    char buf[256];
    
    // Added for test: reset for every new connection 
    // pin = i2c_reset_pins[nb-I2CADR];
    pin = i2c_reset_pins[nb];
    I2C_Reset_Pin(pin);

    adr =  (uint8_t) (0xFF & nb) ;   
    err=  i2c_send(adr,(char)0xCF,0,buf,5000,1,1,buf);
    if (err > 5)
    { ATRlen[nb] = err - 5 ;
      for (i=0;i< ATRlen[nb]; i++)
      ATR[nb][i]= buf[i+3];
      
    }
	else
	{ ATRlen[nb]=0;
	  ATR[nb][0]=0;
	  return -1   ;
	}

    return 0;
}

int i2c_off(int nb)
{ uint8_t adr;
  int err,pin;
  char buf[256];
 
  //adr =  (uint8_t) (0xFF & nb) ; 
  //err=  i2c_send(adr,(char)0xC3,0,buf,5000,1,1,buf);
  
  //pin = i2c_reset_pins[nb-I2CADR];
  pin = i2c_reset_pins[nb];
  I2C_Reset_Pin(pin);
 
  
  return 0;
}

int i2c_atr(int nb, char *atr)
{  int i;
   atr[0]= (char)0x3B ;
   atr[1]= 0x0F & ATRlen[nb];
   for (i=0;i<ATRlen[nb];i++)
   atr[2+i]= ATR[nb][i];
   

   return 2+ATRlen[nb] ;
}

int i2c_senda(int nb, char* APDU, DWORD APDUlen, char* Response, DWORD* Rlen)
{ uint8_t adr;
  char buf[300];
  int err;

  adr =  (uint8_t) (0xFF & nb) ; 
  memmove(buf+3,APDU,(int)APDUlen);

  *Rlen=0 ;

  err=  i2c_send(adr,(char)0xC1,(int)APDUlen,buf,5000,1,1,buf);

  if (err < 7)
  return -1 ;

  memmove(Response,buf+3,err-5);
  *Rlen= (DWORD)(err-5);

 // *Rlen=2;
 // Response[0]=(char)0x90;
 // Response[1]=(char)0x00;

  return 0 ;

}
#endif