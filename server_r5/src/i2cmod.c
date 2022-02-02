#include "i2cmod.h"

int i2c_fdebug=0 ;
extern int MAXI2C;

/////////////////
//#define P1
/////////////////
/////////////////
#define I2C_MODE1
/////////////////

int i2c_ptrx2=0  ;
char bufi2c[300] ;
int error_i2c=0;
int error_pcb=0;
int error_crc=0;
int error_timeout=0;
int error_b1=0;
int error_b2=0;
int error_b3=0;

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
#define TGUARD_I2C 50      // 10 us minimum time between two I2C accesses
#define RESET_TIME 100     // us RST=LOW
#define START_TIME 2500    // us TIME for I2C Ready
#define GUARD_TIME 250000  // us after reset minimum 200ms



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



void uswait(int micro)
{
	uint32_t tr=0, ta=0,td=0;
	td= micro;
	
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


#ifdef I2C_MODE1
static int file_i2c=-1;
int txi2c(char *buf, int len, uint8_t adr)
{ int err=0;
//file_i2c=0;
 
//----- OPEN THE I2C BUS -----

//char *filename = (char*)"/dev/i2c-1";

if (file_i2c < 0)
	return -1;

if (len <=0) return -1;

MUTEX_LOCK(Pmutex[M_SYSTEM+2]);

//////////////////
usleep(TGUARD_I2C);
//////////////////


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
if (i2c_fdebug) gPrintf(SCR,"fail to talk to slave.\n");
return -2;
}

err = write(file_i2c, buf, len);
//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
if (err != len)		
{
/* ERROR HANDLING: i2c transaction failed */
error_i2c++;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
if (i2c_fdebug) gPrintf(SCR,"Failed to write to the i2c bus (req %d read %d)\n",len,err);
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

if (file_i2c < 0)
	return -1 ;

if (len <= 0)
return -1;

MUTEX_LOCK(Pmutex[M_SYSTEM+2]);

//////////////////
usleep(TGUARD_I2C); // add 125 us
//////////////////

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
if (i2c_fdebug) gPrintf(SCR,"Failed to talk to slave.\n");
return -2;
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

#else
int txi2c(char *buf, int len, uint8_t adr)
{ int err=0,file_i2c=-1;
  char *filename = (char*)"/dev/i2c-1";

MUTEX_LOCK(Pmutex[M_SYSTEM+2]);

///////////////////
usleep(TGUARD_I2C);  // add 125 us
// usdelay(TGUARD_I2C);
///////////////////

//-----OPEN THE I2C BUS ---------
file_i2c = open(filename, O_RDWR);
if (file_i2c < 0)
{ error_i2c++;file_i2c=-1;
  MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
  if (i2c_fdebug) gPrintf(SCR,"Failed to open the i2c bus");
  return -1  ;
}


// ioctl(file_i2c,I2C_TIMEOUT,10000);
	
	
if (ioctl(file_i2c, I2C_SLAVE,(int)(adr & 0xFF)) < 0)
{
close (file_i2c);file_i2c=-1;
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
close (file_i2c);file_i2c=-1;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
if (i2c_fdebug) gPrintf(SCR,"Failed to write to the i2c bus (req %d read %d)\n",len,err);

return -1 ;
}

close (file_i2c);file_i2c=-1;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);

return len;
	
}

int rxi2c(char *buf, int len, uint8_t adr)
{ int err=0,file_i2c=-1;
  char *filename = (char*)"/dev/i2c-1";

MUTEX_LOCK(Pmutex[M_SYSTEM+2]);

///////////////////
usleep(TGUARD_I2C);  // add 125 us
// usdelay(TGUARD_I2C);

///////////////////

file_i2c = open(filename, O_RDWR);
if (file_i2c < 0)
{ close(file_i2c);file_i2c= -1;
  error_i2c++;
  MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
  if (i2c_fdebug) gPrintf(SCR,"Failed to open the i2c bus");
  return -1;
}

//if (len == 1) ioctl(file_i2c,I2C_TIMEOUT,1);
//else          ioctl(file_i2c,I2C_TIMEOUT,10000);
 
if (ioctl(file_i2c, I2C_SLAVE,(int)(adr & 0xFF)) < 0)
{
close(file_i2c);file_i2c= -1;
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
close(file_i2c);file_i2c= -1;
error_i2c++;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);

if (i2c_fdebug) gPrintf(SCR,".[%d %d])",len,err);
return -1;
}


close(file_i2c);file_i2c= -1;
MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);

return len;
	
}



#endif


int wait_i2c(uint8_t i2c_adr, int dt0, int *tt, int dt, int *ct, char *bufi2c, int nb)
{ int err=0;
  //int lct=0;

if (nb <= 0) return -1;

if (*tt >= dt)
  { if (i2c_fdebug) gPrintf(SCR,"%s\n","I2C Rx Timeout !");
    error_timeout++;
    return -1;
  }

err= rxi2c(bufi2c,nb,i2c_adr);

while(err != nb) 
{ usleep(dt0*1000);
  //lct++;
  *tt = *tt + dt0  ;
  *ct = *ct + 1    ;
  if (*tt >= dt) 
  { if (i2c_fdebug) gPrintf(SCR,"%s\n","I2C Rx Timeout !");
    error_timeout++;
    return -1;
  }
  
  err= rxi2c(bufi2c,nb,i2c_adr);
}

return err;
}

extern int NBI2C;
extern int i2c_reset_pins[128] ;
extern int i2c_decoder_pins[16];
extern int i2c_adr[128] ;
extern int fdecoder  ;


#define ERROR_CRC   0x83
#define ERROR_SE    0x82
#define ERROR_OTHER 0x81
#define MAX_RETRY_HW  10
#define MAX_RETRY_CRC 10


int i2c_send(uint8_t i2c_adr, char pcb,int len,char *buf, int dt,int f_tx, int f_rx,char * bufi2c)
{ uint16_t mycrc=0;
  uint32_t tr=0,ts=0,trx=0,te=0;
  int err=0,toread;
  int i2c_ptrx2=0 ;
  int ct=0,nb=0;
  int last_len=0,pt=0;
  int tt=0,dt0=0;

  int state=0;
  int retry1_ct=0;
  int retry2_ct=0;
  char buf2[5];
  char buf1[300];
  char *pbuf=NULL;


Tx_Retry:

  if (f_tx)  
  {

  if (state == 1)
  {  pbuf = buf1  ;
     nb = last_len;
     retry1_ct++;
     if (retry1_ct > MAX_RETRY_CRC) 
	 { if (i2c_fdebug)  gPrintf(SCR,"Too much Tx-Retry1 %d\n",retry1_ct); 
           return -1;
         }
	 if (i2c_fdebug) gPrintf(SCR,"Tx-Retry1 %d\n",retry1_ct);
  }

  else if (state == 2)
  { buf2[0]= (char)0x5A;
    buf2[1]= (char)ERROR_CRC ;
    buf2[2]= (char)0;
    mycrc= crc16(buf2,3);
    buf2[3]= 0xFF & mycrc ;
    buf2[4]= 0xFF & (mycrc >> 8);
    pbuf = buf2;
    nb=5;
    retry2_ct++;
    if (retry2_ct > MAX_RETRY_CRC) 
	{ if (i2c_fdebug) gPrintf(SCR,"Too much Tx-Retry2 %d\n",retry2_ct); 
          return -1;
        }
	if (i2c_fdebug) gPrintf(SCR,"Tx-Retry2 %d\n",retry2_ct);
  }

  else // state=0
  {
  buf[0]= (char)0x5A;
  buf[1]= pcb ;
  buf[2]= 0xFF & len;
  mycrc= crc16(buf,len+3);
  buf[3+len]= 0xFF & mycrc ;
  buf[4+len]= 0xFF & (mycrc >> 8);
  memmove(buf1,buf,(int)(5+len));
  nb = last_len = 5+len;
  pbuf = buf1;
  }


  pt=0;
 
  if (i2c_fdebug)
  //if (i2c_fdebug && (state==0) )
  myPrintf("TxI2C",(uint8_t *)pbuf,nb);
 
  if (state == 0) 
	  tr= micros();
  
  while(nb > 0)
  {
  if (nb > MAXI2C)
  {
  err = txi2c(pbuf+pt,MAXI2C,i2c_adr);
  if (err >=0)
  { nb-=MAXI2C;
    pt+=MAXI2C;
  }
  }
  else
  {
  err = txi2c(pbuf+pt,nb,i2c_adr);
  if (err >=0)
	  nb=0;
  }
 
  if (err < 0) 
  { // if (err == -2) return -1;
	if (i2c_fdebug) {gPrintf(SCR,"Tx Error, Retry=%d\n",ct);}
    if (ct > MAX_RETRY_HW)
		return -1;
	ct++;
    usleep(1000);
  }
  }
  }

  ts= micros();

 if (!f_rx) 
 return 0;
 
tt=0;
dt0=10;
i2c_ptrx2=0;
ct=0;
nb=0;

//////////////////////////////////////
while(nb == 0)
{
nb=1;
 
//  (*tt) += dt0  ;
//  (*ct)++;
//  if ((*tt) > dt) return -1;


err= wait_i2c(i2c_adr,dt0,&tt,dt,&ct,bufi2c,nb);
if (err != nb) return -1;

i2c_ptrx2++;
nb--;
if (bufi2c[i2c_ptrx2-1]  == (char)0xA5) 
break;
if (bufi2c[i2c_ptrx2-1] != (char)0x00) 
{
  error_b1++;
  return -1 ;
}

i2c_ptrx2-- ;
usleep(dt0*1000);
tt += dt0   ;
ct++;
}
/////////////////////////////////////////////

trx= micros();

#ifdef P1
nb=2;
err= wait_i2c(i2c_adr,dt0,&tt,dt,&ct,bufi2c+1,nb);
if (err != nb) return -1;
if (bufi2c[1] == (char)0xFF) [ error_b2++; return -1;}
if (bufi2c[2] == (char)0xFF) { error_b3++; return -1;}
#else
nb=1;
err= wait_i2c(i2c_adr,dt0,&tt,dt,&ct,bufi2c+1,nb);
if (err != nb) return -1;
if (bufi2c[1] == (char)0xFF){ error_b2++; return -1;}
nb=1;
err= wait_i2c(i2c_adr,dt0,&tt,dt,&ct,bufi2c+2,nb);
if (err != nb) return -1;
if (bufi2c[2] == (char)0xFF){ error_b3++; return -1;}
nb=2;
#endif

nb=0;
i2c_ptrx2+= 2;
nb= (0xFF & bufi2c[2]) + 2;

while (nb>0)
{

if (nb > MAXI2C)
{ toread = MAXI2C;}

else
{ toread=nb;}

err= wait_i2c(i2c_adr,dt0,&tt,dt,&ct,bufi2c+i2c_ptrx2,toread);
if (err != toread) return -1;

i2c_ptrx2 += toread;
nb -= toread;
}

te= micros();

mycrc = crc16(bufi2c,i2c_ptrx2-2);
if (i2c_fdebug) myPrintf("RxI2C",(uint8_t *)bufi2c,i2c_ptrx2);

if ( (bufi2c[i2c_ptrx2-2] != (char)(0xFF & mycrc) ) || (bufi2c[i2c_ptrx2-1] != (char)(0xFF & (mycrc>>8))) )
{ if (i2c_fdebug) gPrintf(SCR,"Error CRC !\n");
  error_crc++;
  //////////////////////////
  if (f_tx && (state == 0) )
  /////////////////////////
  { state=2;
    usleep(1000);
    goto Tx_Retry;
  }
  return -1;
}

if (bufi2c[0] != (char)0xA5)
{ if (i2c_fdebug) gPrintf(SCR,"Error NAD %d!\n",bufi2c[0]);
  error_pcb++;
  return -1;
}

// 1 0 0 N(R) 0 0 x y (xy=Error code)
if ( ( (bufi2c[1] & (char)0xE0) == (char)0x80 ) && ( (bufi2c[1] & (char)0x3) != (char)0 ))
{error_pcb++;
 if (i2c_fdebug) gPrintf(SCR,"Error PCB %2.2X\n",0xFF & bufi2c[1]);
 /////////////////////////////////////////////////////////////
 if ((bufi2c[1] == (char)ERROR_CRC) && f_tx && (state == 0) )
 /////////////////////////////////////////////////////////////
 { if (state == 0) state=1;
   usleep(1000);
   goto Tx_Retry;
 }
 return -1;
}

if (i2c_fdebug)
{ 
gPrintf(SCR,"i2c_adr:  %d\n",0xFF & i2c_adr);
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
{ int dadr=0;

  if (i2c_fdebug) gPrintf(SCR,"Reset I2C PIN %d\n",pin);

  if ((pin <= 0) && fdecoder)
  { 
   //MUTEX_LOCK(Pmutex[M_SYSTEM+3]);
    MUTEX_LOCK(Pmutex[M_SYSTEM+2]);    

    dadr = -pin ;
    dadr &= 0xF ;

    // A0
	if (dadr & 0x1) digitalWrite(i2c_decoder_pins[2],HIGH);
	else            digitalWrite(i2c_decoder_pins[2],LOW);
	dadr= dadr >> 1;
    // A1
	if (dadr & 0x1) digitalWrite(i2c_decoder_pins[3],HIGH);
	else            digitalWrite(i2c_decoder_pins[3],LOW);
	dadr= dadr >> 1;
	// A2
	if (dadr & 0x1) digitalWrite(i2c_decoder_pins[4],HIGH);
	else            digitalWrite(i2c_decoder_pins[4],LOW);
	dadr= dadr >> 1;
    // A3
	if (dadr & 0x1) digitalWrite(i2c_decoder_pins[5],HIGH);
	else            digitalWrite(i2c_decoder_pins[5],LOW) ;

    // usleep(2);
    //digitalWrite(i2c_decoder_pins[1],LOW);
    //usleep(2);
	
    uswait(1);

    digitalWrite(i2c_decoder_pins[0],LOW);
    usleep(RESET_TIME);
    digitalWrite(i2c_decoder_pins[0],HIGH);
    
    uswait(1);


    digitalWrite(i2c_decoder_pins[2],HIGH);
    digitalWrite(i2c_decoder_pins[3],HIGH);
    digitalWrite(i2c_decoder_pins[4],HIGH);
    digitalWrite(i2c_decoder_pins[5],HIGH);


    //digitalWrite(i2c_decoder_pins[1],HIGH);
    usleep(START_TIME);

    // MUTEX_UNLOCK(Pmutex[M_SYSTEM+3]);
    MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);
    
    usleep(GUARD_TIME);
   
  }

  else
  {

  MUTEX_LOCK(Pmutex[M_SYSTEM+2]);    

  digitalWrite (pin,LOW)   ; 
  usleep(RESET_TIME);
  digitalWrite (pin, HIGH) ;
  usleep(START_TIME);
  MUTEX_UNLOCK(Pmutex[M_SYSTEM+2]);    
  
  usleep(GUARD_TIME);
  }
  

    
return 0;
}




int i2c_start()
{ int err,i;
    
  #ifdef I2C_MODE1
  char *filename = (char*)"/dev/i2c-1";
  gPrintf(SCR,"I2C MODE1\n");
  file_i2c = open(filename, O_RDWR);
  if (file_i2c < 0)
  { if (i2c_fdebug) gPrintf(SCR,"Error..can't open i2c buf\n");
     return -1;
  }
  #else
  gPrintf(SCR,"I2C MODE2\n");
  #endif


 if (getuid()) 
 { if (i2c_fdebug)
   gPrintf(SCR, "%s", "not root\n");
   err= wiringPiSetupSys() ;
 }

 else 
 { if (i2c_fdebug)
   gPrintf(SCR, "%s", "root\n");
   err= wiringPiSetupGpio();

   for(i=0;i<NBI2C;i++)
   { if (i2c_reset_pins[i2c_adr[i]]>0)
     {
     if (i2c_fdebug) gPrintf(SCR,"Pin %d => output\n",i2c_reset_pins[i2c_adr[i]]);
     pinMode(i2c_reset_pins[i2c_adr[i]], OUTPUT) ;
     digitalWrite (i2c_reset_pins[i2c_adr[i]], HIGH) ;
     }
   }
     
   
   //  0 = E, The selected output is enabled by a low on the enable input (E). 
   //  A high on E inhibits selection of any output
   //  All Outputs = 0 for 4514; All Outputs = 1 for 4515
   
   // 1 = LE, When Latch Enable (LE) is high, 
   // the output follows changes in the inputs. 
  
   if (fdecoder)
   { for(i=0;i<6;i++)
     { if (i2c_fdebug) gPrintf(SCR,"Pin %d => output\n",i2c_decoder_pins[i]);
       pinMode(i2c_decoder_pins[i], OUTPUT) ;
       if (i==1) digitalWrite (i2c_decoder_pins[i], HIGH)  ;
       else      digitalWrite (i2c_decoder_pins[i], HIGH) ;
     }
   }

 }

   
   for(i=0;i<NBI2C;i++)
   { if (i2c_reset_pins[i2c_adr[i]]>0) 
     digitalWrite(i2c_reset_pins[i2c_adr[i]],HIGH);
   }
     
   
   if (fdecoder)
   { for(i=0;i<6;i++)
     if (i==1) digitalWrite (i2c_decoder_pins[i], HIGH) ;
     else      digitalWrite (i2c_decoder_pins[i], HIGH);
   }
       
   usleep(GUARD_TIME);
   
   int k=0;
   
   for (k=0;k<NBI2C;k++)
   { for (i=0;i<1;i++) 
     I2C_Reset_Pin(i2c_reset_pins[i2c_adr[k]]); 
   }

  return err;
}

#endif
