/* pcscemulator.h */
/* Copyright (C) 2017 Pascal Urien (pascal.urien@gmail.com)
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

#define NOPCSC

#ifndef NOPCSC
#include <winscard.h>
#endif

#define NO_PCSC_RELEASE
#define HANDLE_BASE_GRID 1024
#define HANDLE_BASE_I2C  3072

// #define UNICODE_NAME

#ifdef NOPCSC

#ifndef WIN32
typedef long   DWORD;
#endif

typedef unsigned long   DWORD2   ;
typedef unsigned long * LPDWORD2 ;
typedef long            LONG2    ;
typedef unsigned long   ULONG2   ;

#define WINSCARDAPI2
#define WINAPI2 
#define IN2 
#define OUT2 
typedef unsigned long long  SCARDHANDLE2 ;
typedef unsigned long long  SCARDCONTEXT2;
typedef SCARDCONTEXT2 *PSCARDCONTEXT2, *LPSCARDCONTEXT2;
typedef SCARDHANDLE2  *PSCARDHANDLE2,  *LPSCARDHANDLE2;

typedef char* LPCBYTE2;
typedef char* LPBYTE2;
typedef void* LPCVOID2;

typedef char* LPCSTR2;
typedef char* LPSTR2 ;
typedef char* LPTSTR2;


typedef int SCARD_IO_REQUEST2;
typedef SCARD_IO_REQUEST2* LPCSCARD_IO_REQUEST2 ;
typedef SCARD_IO_REQUEST2* LPSCARD_IO_REQUEST2  ;




#define SCARD2_PROTOCOL_T0    1
#define SCARD2_PROTOCOL_T1    2

#define SCARD2_S_SUCCESS 0

#define SCARD2_STATE_PRESENT   10
#define SCARD2_SHARE_EXCLUSIVE 11
#define SCARD2_SHARE_SHARED    12
#define SCARD2_ABSENT 13
#define SCARD2_RESET_CARD 14
#define SCARD2_SCOPE_SYSTEM 1000



#define SCARD2_UNPOWER_CARD 100
#define SCARD2_LEAVE_CARD   101

#define SCARD2_ATTR_VENDOR_IFD_TYPE 200
#define SCARD2_ATTR_VENDOR_IFD_SERIAL_NO 201
#define SCARD2_CLASS_IFD_PROTOCOL 203
#define SCARD2_ATTR_VENDOR_NAME 204

DWORD2 SCARD2_ATTR_VALUE(DWORD2 A, ULONG2 B);


#else

#ifndef WIN32
#define WINSCARDAPI PCSC_API
#define WINAPI 
#define IN 
#define OUT 
#endif

#define DWORD2    DWORD
#define LPDWORD2  LPDWORD
#define LONG2     LONG
#define ULONG2    ULONG

#define WINSCARDAPI2 WINSCARDAPI
#define WINAPI2 WINAPI
#define IN2 IN
#define OUT2 OUT
#define SCARDHANDLE2 SCARDHANDLE
#define LPCSCARD_IO_REQUEST2 LPCSCARD_IO_REQUEST
#define LPSCARD_IO_REQUEST2 LPSCARD_IO_REQUEST
#define LPCBYTE2 LPCBYTE
#define LPBYTE2  LPBYTE
#define LPCVOID2 LPCVOID
#define LPSCARDCONTEXT2 LPSCARDCONTEXT
#define SCARDCONTEXT2 SCARDCONTEXT
#define LPCSTR2 LPCSTR
#define LPSTR2  LPSTR
#define LPTSTR2 LPTSTR
#define LPSCARDHANDLE2 LPSCARDHANDLE

#define SCARD_IO_REQUEST2     SCARD_IO_REQUEST
#define LPCSCARD_IO_REQUEST2  LPCSCARD_IO_REQUEST
#define LPSCARD_IO_REQUEST2   LPSCARD_IO_REQUEST
#define SCARD2_ATTR_VALUE     SCARD_ATTR_VALUE

#define SCARD2_PROTOCOL_T0 SCARD_PROTOCOL_T0  
#define SCARD2_PROTOCOL_T1 SCARD_PROTOCOL_T1
#define SCARD2_S_SUCCESS   SCARD_S_SUCCESS 

#define SCARD2_STATE_PRESENT   SCARD_STATE_PRESENT  
#define SCARD2_SHARE_EXCLUSIVE SCARD_SHARE_EXCLUSIVE 
#define SCARD2_SHARE_SHARED    SCARD_SHARE_SHARED  
#define SCARD2_ABSENT          SCARD_ABSENT 
#define SCARD2_RESET_CARD      SCARD_RESET_CARD 
#define SCARD2_SCOPE_SYSTEM    SCARD_SCOPE_SYSTEM 
#define SCARD2_UNPOWER_CARD    SCARD_UNPOWER_CARD 
#define SCARD2_LEAVE_CARD      SCARD_LEAVE_CARD 

#define SCARD2_ATTR_VENDOR_IFD_TYPE      SCARD_ATTR_VENDOR_IFD_TYPE
#define SCARD2_ATTR_VENDOR_IFD_SERIAL_NO SCARD_ATTR_VENDOR_IFD_SERIAL_NO
#define SCARD2_CLASS_IFD_PROTOCOL        SCARD_CLASS_IFD_PROTOCOL 
#define SCARD2_ATTR_VENDOR_NAME          SCARD_ATTR_VENDOR_NAME
#define SCARD2_ATTR_VALUE                SCARD_ATTR_VALUE

           
#endif



WINSCARDAPI2 LONG2 WINAPI2
SCardTransmit2(
    IN2 SCARDHANDLE2 hCard,
    IN2 LPCSCARD_IO_REQUEST2 pioSendPci,
    IN2 LPCBYTE2 pbSendBuffer,
    IN2 DWORD2 cbSendLength,
    IN2 OUT2 LPSCARD_IO_REQUEST2 pioRecvPci,
    OUT2 LPBYTE2 pbRecvBuffer,
    IN2 OUT2 LPDWORD2 pcbRecvLength);

WINSCARDAPI2 LONG2 WINAPI2
SCardEstablishContext2(
    IN2  DWORD2 dwScope,
    IN2  LPCVOID2 pvReserved1,
    IN2  LPCVOID2 pvReserved2,
    OUT2 LPSCARDCONTEXT2 phContext);

WINSCARDAPI2 LONG2 WINAPI2
SCardReleaseContext2(
    IN2      SCARDCONTEXT2 hContext);
    


WINSCARDAPI2 LONG2 WINAPI2
SCardListReadersA2(
    IN2      SCARDCONTEXT2 hContext,
    IN2      LPCSTR2 mszGroups,
    OUT2     LPSTR2 mszReaders,
    IN2 OUT2 LPDWORD2 pcchReaders);


WINSCARDAPI2 LONG2 WINAPI2
SCardConnectA2(
    IN2      SCARDCONTEXT2 hContext,
    IN2      LPCSTR2 szReader,
    IN2      DWORD2 dwShareMode,
    IN2      DWORD2 dwPreferredProtocols,
    OUT2     LPSCARDHANDLE2 phCard,
    OUT2     LPDWORD2 pdwActiveProtocol);

WINSCARDAPI2 LONG2 WINAPI2
SCardReconnect2(
    IN2      SCARDHANDLE2 hCard,
    IN2      DWORD2   dwShareMode,
    IN2      DWORD2   dwPreferredProtocols,
    IN2      DWORD2   dwInitialization,
    OUT2     LPDWORD2 pdwActiveProtocol);


WINSCARDAPI2 LONG2 WINAPI2
SCardDisconnect2(
    IN2      SCARDHANDLE2 hCard,
    IN2      DWORD2 dwDisposition);

WINSCARDAPI2 LONG2 WINAPI2
SCardState2(
    IN2  SCARDHANDLE2 hCard,
    OUT2 LPDWORD2 pdwState,
    OUT2 LPDWORD2 pdwProtocol,
    OUT2 LPBYTE2 pbAtr,
    OUT2 LPDWORD2 pcbAtrLen);


#ifdef UNICODE_NAME
WINSCARDAPI2 LONG2 WINAPI2
SCardListReadersW2(
    IN2       SCARDCONTEXT2 hContext,
    IN2       LPCWSTR2 mszGroups,
    OUT2      LPWSTR2 mszReaders,
    IN2 OUT2  LPDWORD2 pcchReaders);



WINSCARDAPI2 LONG2 WINAPI2
SCardConnectW2(
    IN2      SCARDCONTEXT2 hContext,
    IN2      LPCWSTR2 szReader,
    IN2      DWORD2 dwShareMode,
    IN2      DWORD2 dwPreferredProtocols,
    OUT2     LPSCARDHANDLE2 phCard,
    OUT2     LPDWORD2 pdwActiveProtocol);

#endif

WINSCARDAPI2 LONG2 WINAPI2
SCardGetAttrib2(
    IN2 SCARDHANDLE2 hCard,
    IN2 DWORD2 dwAttrId,
    OUT2 LPBYTE2 pbAttr,
    IN2 OUT2 LPDWORD2 pcbAttrLen);
    
    