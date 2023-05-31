/* pcscemulator.c */
/* Copyright (C) 2017-2022 Pascal Urien (pascal.urien@gmail.com)
 * All rights reserved.
 *
 * This software is an implementation of the internet draft
 * https://tools.ietf.org/html/draft-urien-core-racs-00
 * "Remote APDU Call Secure (RACS)" by Pascal Urien.
 * The implementation was written so as to conform with this draft.
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


#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <time.h>
#include <sys/timeb.h>
#include <memory.h>

#include "pcscemulator2.h"
#include "grid.h"
#include "i2c.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

static DWORD2 ptcol = SCARD2_PROTOCOL_T0;
static int sc[2048];

#ifdef NOPCSC
DWORD2 SCARD2_ATTR_VALUE(DWORD2 A, ULONG2 B){return 0;}
#endif


int isGridSc(LPSCARDHANDLE2 phCard)
{
    if (NBSC == 0)
		return -1;

	if ((int)*phCard <= 0)
		return -1;

	if ( ((int)*phCard >= (int)HANDLE_BASE_GRID) && ((int)*phCard < (int)(HANDLE_BASE_GRID+1024)) )
		return 		
		  (int)((int)*phCard - (int)HANDLE_BASE_GRID);

	return -1;

}

// return I2C address
int isI2cSc(LPSCARDHANDLE2 phCard)
{
	if (NBI2C == 0)
		return -1;

	if ((int)*phCard <= 0)
		return -1;

	if ( ((int)*phCard >= (int)HANDLE_BASE_I2C) && ((int)*phCard < (int)(HANDLE_BASE_I2C+1024)) )
		return 		
        (int)((int)*phCard - (int)HANDLE_BASE_I2C);


	return -1;

}






int GetGridSc(char* szReader)
{  int nb=-1; 
   int len;
   char name[32]; 

   len= (int)strlen(szReader);
   
   if (len < 5)  return -1;
   if (len > 31) return -1;

   strcpy(name,szReader);

   name[4]=(char)0;

   if (strcmp(name,"grid") != 0) return -1;
	
   sscanf(&name[5],"%d",&nb);
    
   return nb;

}

// return i2c_address
int GetI2cSc(char* szReader)
{  int nb=-1; 
   int len;
   char name[32];

   len=(int)strlen(szReader);

   if (len < 4) return -1;
   if (len >31) return -1;
   
   strcpy(name,szReader);

    name[3]=(char)0;

    if (strcmp(name,"i2c") != 0) return -1;
	
	sscanf(&name[4],"%d",&nb);
    
	return nb;

}



#ifdef UNICODE_NAME

int GetGridScW(wchar_t * szReader)
{  int nb=-1; 
   wchar_t c;


	if ((int)wcslen(szReader) < 5)
		return -1;

    c= szReader[4];
	szReader[4]=(char)0;

    if (wcscmp(szReader,L"grid") != 0)
	{szReader[4]=c;
	 return -1;
	}
    szReader[4]=c;
	swscanf(&szReader[4],L"%d",&nb);
    
	return nb;
}

int GetI2cScW(wchar_t * szReader)
{  int nb=-1; 
   wchar_t c;


	if ((int)wcslen(szReader) < 4)
		return -1;

    c= szReader[3];
	szReader[3]=(char)0;

    if (wcscmp(szReader,L"i2c") != 0)
	{szReader[3]=c;
	 return -1;
	}
    szReader[3]=c;
	swscanf(&szReader[3],L"%d",&nb);
    
	return nb;

}
#endif



WINSCARDAPI2 LONG2 WINAPI2
SCardEstablishContext2(
    IN2  DWORD2 dwScope,
    IN2  LPCVOID2 pvReserved1,
    IN2  LPCVOID2 pvReserved2,
    OUT2 LPSCARDCONTEXT2 phContext)
{ 
	LONG2 stat= SCARD2_S_SUCCESS;

    #ifndef NOPCSC
	stat = SCardEstablishContext(dwScope,pvReserved1,pvReserved2,phContext);
    #endif

return stat;
}


WINSCARDAPI2 LONG2 WINAPI2
SCardReleaseContext2(
IN2 SCARDCONTEXT2 hContext)
{
  LONG2 stat=0;
  
  #ifndef NOPCSC
  stat= SCardReleaseContext(hContext);
  #endif

  return stat;
}



WINSCARDAPI2 LONG2 WINAPI2
SCardListReadersA2(
    IN2       SCARDCONTEXT2 hContext,
    IN2       LPCSTR2 mszGroups,
    OUT2      LPSTR2  mszReaders,
    IN2 OUT2  LPDWORD2 pcchReaders	)
{ 
int i;
LONG2 stat=0;
DWORD2 ptr=0;

#ifndef WIN32
#ifndef NOPCSC
LPTSTR list;
DWORD  wlist;
SCARDCONTEXT hContext;
#endif
#endif


NBSC = InitializeGrid()  ;
NBSC = MIN(maxslots,NBSC);

for (i=1;i<=NBSC;i++)
{
sprintf(&mszReaders[ptr],"grid%03d",i);
ptr += 8;
}


NBI2C= i2c_init();

for (i=1;i<=NBI2C;i++)
{
sprintf(&mszReaders[ptr],"i2c%03d",i2c_adr[i-1]);
ptr += 7;
}
*pcchReaders = *pcchReaders - ptr;

mszReaders[ptr]=0;

#ifndef NOPCSC

#ifndef WIN32


 stat = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
 if (stat != SCARD_S_SUCCESS)
 {
     return (SCARD_S_SUCCESS);
 }

#ifdef SCARD_AUTOALLOCATE
 wlist = SCARD_AUTOALLOCATE;

 stat = SCardListReaders(hContext, NULL, (LPTSTR)&list, &wlist);
 if (stat != SCARD_S_SUCCESS)
 {
     return (SCARD_S_SUCCESS);
 }
#else
 stat = SCardListReaders(hContext, NULL, NULL, &wlist);
  if (stat != SCARD_S_SUCCESS)
 {
     return (SCARD_S_SUCCESS);
 }

 mszReaders = calloc(dwReaders, sizeof(char));
 stat = SCardListReaders(hContext, NULL, (LPTSTR)&list, &wlist);
  if (stat != SCARD_S_SUCCESS)
 {
     return (SCARD_S_SUCCESS);
     
 }
#endif

memcpy(mszReaders+ptr,list,wlist)  ;
*pcchReaders = *pcchReaders - wlist;

#ifdef SCARD_AUTOALLOCATE
 stat = SCardFreeMemory(hContext, list);
 #else
 free(list);
#endif

stat = SCardReleaseContext(hContext);

#else
stat = SCardListReaders(hContext,mszGroups,mszReaders+ptr,pcchReaders);
#endif

#else
#endif

return stat;

}


#ifdef UNICODE_NAME


WINSCARDAPI2 LONG2 WINAPI2
SCardListReadersW2(
    IN2       SCARDCONTEXT2 hContext,
    IN2       LPCWSTR2 mszGroups,
    OUT2      LPWSTR2 mszReaders,
    IN2 OUT2  LPDWORD2 pcchReaders	)
{ 
int i;
LONG2 stat=0;
DWORD2 ptr=0;

NBSC=  InitializeGrid();
NBSC = MIN(maxslots,NBSC);

for (i=1;i<=NBSC;i++)
{
wsprintf(&mszReaders[ptr],L"grid%03d",i);
ptr += 16;
}

NBI2C= i2c_init();

for (i=1;i<=NBI2C;i++)
{
wsprintf(&mszReaders[ptr],L"i2c%03d",i2c_adr[i-1]);
ptr += 14;
}

*pcchReaders = *pcchReaders - ptr;

#ifndef NOPCSC
stat = SCardListReadersW(hContext,mszGroups,mszReaders+ptr,pcchReaders);
#else
#endif

return stat;

}

#endif


long DTM=0;

WINSCARDAPI2 LONG2 WINAPI2
SCardTransmit2(
    IN2  SCARDHANDLE2 hCard,
    IN2  LPCSCARD_IO_REQUEST2 pioSendPci,
    IN2  LPCBYTE2 pbSendBuffer,
    IN2  DWORD2 cbSendLength,
    IN2  OUT2 LPSCARD_IO_REQUEST2 pioRecvPci,
    OUT2 LPBYTE2 pbRecvBuffer,
    IN2 OUT2 LPDWORD2 pcbRecvLength)

{  LONG2 stat= SCARD2_S_SUCCESS;
   int nb1=-1,nb2=-1,err; 
   #ifndef NOPCSC
   struct timeb timebuffer1;
   struct timeb timebuffer2;
   #endif
   long t1=0,t2=0,dtm=0;
   //BYTE more[] = {(BYTE)0x00, (BYTE)0xC0, (BYTE)0x00, (BYTE)0x00,(BYTE)0x00};
   int todo=1,lenr=0 ;
   DWORD2 len;

   len = *pcbRecvLength;

   while(todo)
   {  
	   dtm=0;
	   todo=0;

   // Retourne le #du slot (index) dans la grille => startslot...statslot+nbslots-1 dans la grille implementa
   nb1 = isGridSc(&hCard);
   nb2 = isI2cSc(&hCard);

   if ( (nb1 <= 0) && (nb2 <= 0) )
   {
    #ifndef NOPCSC

    ftime(&timebuffer1);
    stat = SCardTransmit(hCard,pioSendPci,pbSendBuffer,cbSendLength,pioRecvPci, pbRecvBuffer,pcbRecvLength);	
    ftime(&timebuffer2);	
   
    t1 =  (int)((timebuffer1.time % 3600)*1000) +   (int)timebuffer1.millitm   ;
    t2 =  (int)((timebuffer2.time % 3600)*1000) +   (int)timebuffer2.millitm   ;
    dtm = (t2-t1);
    
	if (dtm <0) 
		dtm += 3600000 ;

	DTM+= dtm;
    //Printf(">> %d ms\n",dtm);
   #else
   return -1;
   #endif

   }

   else if (nb1 >0)
   {   err = SendGridSc(&sc[nb1-1],(char *)pbSendBuffer,cbSendLength,(char *)pbRecvBuffer,pcbRecvLength,nb1,0);
       if (err <0) 
		   return -1;
   }

   else if (nb2 > 0)
   {  err= i2c_senda(nb2,(char *)pbSendBuffer,cbSendLength,(char *)pbRecvBuffer,pcbRecvLength);
      if (err <0)
		   return -1;

   }

   /*
   if ( (*pcbRecvLength == (DWORD)2) && (*pbRecvBuffer == (BYTE)0x61)  )
   { memmove((void *)pbSendBuffer, (void *)more, 5);
     cbSendLength=5;
	 memmove((void*)(pbSendBuffer+4), (void *)(pbRecvBuffer+1),1);
	 *pcbRecvLength = len;
	 todo=1;
   }
   */

   }
   
   
   return stat;
  

}
// extern int SerialApdu(HANDLE handle,char *req, int rlen, char *resp, int *plen);

//static char more[]= {(char)0xA0,(char)0xC0,(char)0x00,(char)0x00,(char)00};


WINSCARDAPI2 LONG2 WINAPI2
SCardConnectA2(
    IN2      SCARDCONTEXT2 hContext,
    IN2      LPCSTR2 szReader,
    IN2      DWORD2 dwShareMode,
    IN2      DWORD2 dwPreferredProtocols,
    OUT2     LPSCARDHANDLE2 phCard,
    OUT2     LPDWORD2 pdwActiveProtocol)
{  LONG2 stat= SCARD2_S_SUCCESS;
   int nb=-1,err=0;
   
   nb = GetGridSc((char*)szReader);
   if (nb >0)
   { *phCard = (SCARDHANDLE2)(HANDLE_BASE_GRID+nb) ;
     *pdwActiveProtocol=SCARD2_PROTOCOL_T0 ;
     err= ConnectGridSc(nb,&sc[nb-1] );
 	 if (err<=0) 
		 return -1;
    
     return(stat);
   }

   nb = GetI2cSc((char*)szReader); // I2c_address
   if (nb >0)
   { // *phCard = (SCARDHANDLE)(HANDLE_BASE_I2C+nb-I2CADR);
     *phCard = (SCARDHANDLE2)(HANDLE_BASE_I2C+ nb) ;
     *pdwActiveProtocol=SCARD2_PROTOCOL_T0 ;
     err= i2c_on(nb);
 	 if (err <0 ) 
		 return -1;
   
     return(stat);
   }

   #ifndef NOPCSC
   stat= SCardConnect(hContext,szReader,dwShareMode,dwPreferredProtocols,phCard,pdwActiveProtocol);
   #else
   return -1;
   #endif

return(stat);
}

#ifdef UNICODE_NAME

WINSCARDAPI2 LONG2 WINAPI2
SCardConnectW2(
    IN2      SCARDCONTEXT2 hContext,
    IN2      LPCWSTR2 szReader,
    IN2      DWORD2 dwShareMode,
    IN2      DWORD2 dwPreferredProtocols,
    OUT2     LPSCARDHANDLE2 phCard,
    OUT2     LPDWORD2 pdwActiveProtocol)

{  LONG2 stat= SCARD2_S_SUCCESS;
   int nb1=-1,nb2=-1,err=0;

   nb1 = GetGridScW((wchar_t *)szReader);
   nb2 = GetI2cSc((wchar_t *)szReader);
  
   if (nb1 >0)
   { *phCard = (SCARDHANDLE)(HANDLE_BASE_GRID+nb1-startslot);
     err= ConnectGridSc(nb1,&sc[nb1-1]);
	 if (err<=0)  return -1;
	 *pdwActiveProtocol=SCARD2_PROTOCOL_T0 ;
   }

   else if (nb2 >0)
   { //*phCard = (SCARDHANDLE)(HANDLE_BASE_I2C+nb2-I2CADR);
     *phCard = (SCARDHANDLE)(HANDLE_BASE_I2C+nb2);
     err= i2c_on(nb2);
 	 if (err <0 ) return -1;
     *pdwActiveProtocol=SCARD2_PROTOCOL_T0 ;
   }

   else
   #ifndef NOPCSC
   stat= SCardConnectW2(hContext,szReader,dwShareMode,dwPreferredProtocols,phCard,pdwActiveProtocol);
   #else 
   return -1;
   #endif

return(stat);
}


#endif




WINSCARDAPI2 LONG2 WINAPI2
SCardReconnect2(
    IN2      SCARDHANDLE2 hCard,
    IN2      DWORD2  dwShareMode,
    IN2      DWORD2  dwPreferredProtocols,
    IN2      DWORD2  dwInitialization,
    OUT2     LPDWORD2 pdwActiveProtocol)
{ LONG2 stat = SCARD2_S_SUCCESS ;
  int nb1=-1;
  int nb2=-1;

  nb1= isGridSc(&hCard);
  nb2= isI2cSc(&hCard) ;

 #ifndef NOPCSC
 if ( (nb1 <=0 ) && (nb2 <=0) )
 stat= SCardReconnect(hCard,dwShareMode,dwPreferredProtocols,dwInitialization,pdwActiveProtocol);
 #else
 #endif


return stat ;
}


WINSCARDAPI2 LONG2 WINAPI2
SCardDisconnect2(
    IN2      SCARDHANDLE2 hCard,
    IN2      DWORD2 dwDisposition)
{ 
LONG2 stat=SCARD2_S_SUCCESS;
int nb1=-1;
int nb2=-1;

nb1= isGridSc(&hCard);
nb2= isI2cSc(&hCard);


if ( (nb1 <= 0) &&  (nb2 <= 0) ) 
#ifndef NOPCSC
stat= SCardDisconnect(hCard,dwDisposition);
#else
return -1;
#endif

else if (nb1 > 0)
DeconnectGridSc(nb1, &sc[nb1-1]);

else if (nb2 >0)
stat = (LONG2)i2c_off(nb2);

return stat;
}

static char atr[] = {(char)0x3B,(char)0x07,(char)'G',(char)'R',(char)'I',(char)'D',(char)'0', (char)'0',(char)'0',(char)0};


WINSCARDAPI2 LONG2 WINAPI2
SCardGetAttrib2(
    IN2 SCARDHANDLE2 hCard,
    IN2 DWORD2 dwAttrId,
    OUT2 LPBYTE2 pbAttr,
    IN2 OUT2 LPDWORD2 pcbAttrLen)
{
    LONG2 stat= SCARD2_S_SUCCESS ;
	int nb1=-1,nb2=-1;

	nb1= isGridSc(&hCard);
	nb2= isI2cSc(&hCard);

    if ( (nb1 <= 0) &&  (nb2 <= 0) )
    
    #ifndef NOPCSC
    stat = SCardGetAttrib(hCard,dwAttrId,pbAttr,pcbAttrLen);
    #else
	return -1;
    #endif

	else
	return -1;


	return stat;
}

extern char atrgrid[128][64] ;
extern int  lenatrgrid[128]  ;

WINSCARDAPI2 LONG2 WINAPI2
SCardState2(
    IN2 SCARDHANDLE2 hCard,
    OUT2 LPDWORD2 pdwState,
    OUT2 LPDWORD2 pdwProtocol,
    OUT2 LPBYTE2 pbAtr,
    OUT2 LPDWORD2 pcbAtrLen)    
    {
        
	LONG2 stat= SCARD2_S_SUCCESS ;
	int nb1=-1,nb2=-1;
    #ifndef NOPCSC
    char myreader[200];
    #endif
	DWORD2 size=200;

	nb1= isGridSc(&hCard);
    nb2= isI2cSc(&hCard);

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if ( (nb1 <= 0) && (nb2 <= 0) )
    #ifndef NOPCSC
    // stat= SCARD_S_SUCCESS ;    
    //stat = SCardState(hCard,pdwState,pdwProtocol,pbAtr,pcbAtrLen);
    stat = SCardStatus(hCard,myreader,&size,pdwState,pdwProtocol,pbAtr,pcbAtrLen);
    #else
	return -1;
    #endif

	else if (nb1 >0)
	{ 
	sprintf(&atr[6],"%03d",nb1);
    *pdwState    =  SCARD2_STATE_PRESENT ;
    *pdwProtocol =  ptcol   ;
	*pcbAtrLen   = sizeof(atr)-1   ;
	 memmove(pbAtr,atr,sizeof(atr));

     *pcbAtrLen   = (DWORD2) lenatrgrid[nb1-1];
     memmove(pbAtr,atrgrid[nb1-1],lenatrgrid[nb1-1]) ;

	}

	else if (nb2 >0)
	{ (*pcbAtrLen) = (DWORD2) i2c_atr(nb2,pbAtr);
	  if (*pcbAtrLen <=0) stat=-1;
	  else
	  { *pdwState    =  SCARD2_STATE_PRESENT ;
        *pdwProtocol =  ptcol   ;
	  }
	}

return stat ;
}




