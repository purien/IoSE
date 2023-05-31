/* windowsglue.c */
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

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#endif


#ifndef WIN32
#define HWND void*
#else
#include <windows.h>
#endif

#include "reentrant2.h"
#include "mutuex.h"
#include "screen.h"

int NC=140       ; // max character perline
int wBG= 0xFFFFFF; // Background
int wPEN= 0  ;     // Pen color
int wSIZE= 18;     // Fonte Size

int system_console=1    ; // use system console
int system_console2=1   ; // use system console
int reader_console=1    ; // use console for reader
int is_external_grid=1  ; // use implementa grid
int autostart=1         ; // auto start
int startdelay=0        ; // delay for autostart ms

int startnewconsole(char *name);
int closeconsole(int index);
HWND gethWnd(int id);
int setid(int deb);
int tile();

// static HWND hWnd[NB_SCREEN];
static HWND hWnd[W_READER+1];
 

// Return an id for a console
// 0 system
// 1 system2
// 2...1+nbReaderOn                   SmartCardReader
// 2+NbReaderOn..... 2+NbReaderOn + MAX_DISPLAY server
// return -1 if no resource is available
extern int  Reader_Nb_On ;
// static int  ct=0;


int startnewconsole(char *name)
{ //int index= -1;
  int index= W_READER;
  
  // index= setid(W_READER + Reader_Nb_On) ;
  // if (index <0) return -1;
  
  //hWnd[index]= (HWND)-1;
  
  return index;


}


// Close a console
int closeconsole(int index)
{ //HWND hwnd=NULL;

    //hwnd= gethWnd(index);
 	//if (hwnd != NULL)
    //hWnd[index] = NULL;

	return 0;
}



HWND gethWnd(int id)
{ if ( (id>=0) && (id<NB_SCREEN ) ) return hWnd[id] ;
  return NULL;
}

 int setid(int deb)
 { int i;

  MUTEX_LOCK(Pmutex[M_SYSTEM]);


    for(i=deb;i<NB_SCREEN;i++)
	{ if (hWnd[i] == NULL)
     {hWnd[i]= (HWND)-1;
	  MUTEX_UNLOCK(Pmutex[M_SYSTEM]);
      return i ;
     }
    }

  MUTEX_LOCK(Pmutex[M_SYSTEM]);

 return -1;
 }





	
int tile()
{
	return 0;
}


int setconsole_name(int id, char *name)
{  
#ifdef  WIN32
   SetConsoleTitle(name);
#endif

return 0;
}
