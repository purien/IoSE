/* grid.c */
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

#define _CRT_SECURE_NO_DEPRECATE 1

#include <stdlib.h>
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
#else
  #include <winsock.h>
#endif


extern int  indexs[200]                    ;
extern int  Get_Reader_Index_Abs(int id)   ;
extern int  gPrintf(int id,char *fmt, ... );

static int Ascii2bin(char *data_in,char *data_out);
static int isDigit(char c);

char gridserver[128]="tcpipgrid.net";
unsigned short gridport=2310;
int  maxslots=1   ;
int  startslot=24 ;
char board[32]="7" ;
int  NBSC=0       ;
int  slotid[512]  ;

extern long DTM;

#define MAX_MSG 2048

int verbose2=1     ;
char cacheHost[256];
int  cacheIP=0     ;

char atrgrid[128][64];
int  lenatrgrid[128];

int ParseContent(char* Content, char* out);


int is_grid(int num)
{ if (NBSC == 0)
  return 0 ;
  if ( (num>= 0) && (num<NBSC) )
	  return 1;
  return 0;
}

int Recv(int s, char *buf, int size, int flags)
{ int err=0,len=0;
  char c;
  size--;

	while(1)
	{  err= recv(s,&buf[len],size-len,flags);
       if (err <=0) 
	   {   
		   return err;
		   //break ;
	   }
	   len+=err;
	   c = buf[len-1];
	   if ( c == (char)'\n') break;
	   if (len >= size)      
	   {   
		   return -1 ;
		   //break;
	   }
	}

    buf[len]=0 ;
	return len ;

}

int SetConnectAddress(struct sockaddr_in *sin,unsigned short port,char *host)
{ 
 struct hostent *phe ;
	 
 sin->sin_family      = AF_INET;
 sin->sin_port        = htons(port);
 	 
 if ( (cacheIP !=0) && (strcmp(host,cacheHost)==0) )
 {  sin->sin_addr.s_addr = cacheIP;
 }
 
 else
 {
 sin->sin_addr.s_addr = inet_addr (host);
 cacheIP=0;
 cacheHost[0]=0;

 if (sin->sin_addr.s_addr == INADDR_NONE)
 {	  phe = gethostbyname (host);
	  if (phe == NULL) return(0);
      
      else     
      memcpy(&(sin->sin_addr),phe->h_addr,4);
	  
	  if (sin->sin_addr.s_addr == INADDR_NONE)
	  return(0); 
	  
 }

 cacheIP= sin->sin_addr.s_addr ;
 strcpy(cacheHost,host);

 }
return(1);
 
}

int ParseAPDU(char* in, char* Response, int len)
{
	int nb, i, j, k;
	char tmp[16];
	char tmp_resp[MAX_MSG];


		k=0; 
		for (i=0; i<len; i++)
		{
			if (in[i] != (char)'*') 
			{
				tmp_resp[k] = in[i];
				k++;
			}
			else 
			{
				strncpy(tmp,&in[i+1],2);
				tmp[2]=0;
				sscanf(tmp,"%x",&nb);
				

				for (j=0; j<nb; j++) 
					strncpy(tmp_resp+k+(j*2),&in[i+3],2); 
				i+=4;
				k+=(2*nb);
				
			}
		}

   	    tmp_resp[k]=0;
		strcpy(Response, tmp_resp);
		
		return 	(int)strlen(Response);
}



int asciiSendTCP(int idp,char* req, char* resp)
{
	struct sockaddr_in sin,csin   ; 
	int len, err=0, namelen;
	int client;

	client = (int)socket (AF_INET,SOCK_STREAM,0); 

    csin.sin_family = AF_INET   ;  
    csin.sin_port   = 0 ;  
    csin.sin_addr.s_addr =  INADDR_ANY;  
 
    err = bind (client,(struct sockaddr *) &csin, sizeof (csin));

    namelen = sizeof(csin);
    err = getsockname(client,(struct sockaddr *) &csin, &namelen);
 
    sin.sin_family = AF_INET   ;  
    sin.sin_port = htons(gridport) ;  
    sin.sin_addr.s_addr =  inet_addr("127.0.0.1") ;


   if (!SetConnectAddress(&sin,(unsigned short)gridport,gridserver))
   {
	   gPrintf(idp,"DNS error for grid server...\n");
	   return -1 ;
   }

    err= connect(client,(struct sockaddr *) &sin,sizeof(struct sockaddr));

	if (err != 0)
	{ gPrintf(idp,"Connection to Grid Server Failed !!!\n");
	  return -1 ;
	}


	memset(resp, 0, MAX_MSG);
	err = Recv(client,resp,MAX_MSG,0);

	if (err <= 0)
	{ gPrintf(idp,"Error while receving data from the grid !!!\n");
	  return -1;
	}

    resp[err]=0;

    if (verbose2)
		gPrintf(idp,resp);
	
	memset(resp, 0, MAX_MSG);
	len = (int)strlen(req);

    if (verbose2) 
		gPrintf(idp,req);
 
    err = send(client,req,len,0)     ;
    
	if (err <= 0)
	{ gPrintf(idp,"Error while sending data to the grid !!!\n");
	  return -1 ;
	}

    err = Recv(client,resp,MAX_MSG,0);
	
	if (err <= 0)
	{ gPrintf(idp,"Error while receving data from the grid !!!\n");
	  return -1 ;
	}

    resp[err]=0;
   

    if (verbose2)
		gPrintf(idp,resp);


	 shutdown(client,2) ;
     #ifndef WIN32
     close(client);
     #else
	 closesocket(client);
     #endif

	

	return 0;
}


int asciiGridSend(int nbcard, int client, char* req, char* resp)
{	int len=0,err=0;   
	int idp;

    idp= indexs[Get_Reader_Index_Abs(nbcard-1)];

	len = (int)strlen(req);

    if (verbose2)
		gPrintf(idp,req);
 
    err = send(client,req,len,0)     ;
	if (err <= 0) return -1;

    err = Recv(client,resp,MAX_MSG,0);
	if (err <= 0) return -1 ;

	resp[err]=0;

    if (verbose2)
		gPrintf(idp,resp);

	return 1;

}

int ParseContent(char* Content, char* out)
{
	int i,count=0;
	int first=0, dboard=0,k;
	char c;
    int myboard=7,nb=0,nib,nc=0;
	char *token=NULL;
	
    token = strtok(Content,":\r\n"); 
    if (token == NULL) return 0;

	while(1)
	{ 
	   if (token[0] != (char)'.') return 0  ;
	   nb= sscanf(token+1,"%d",&dboard)   ;
	   if (nb !=1 ) return 0;
	   first  = dboard % 100;
	   dboard = dboard/100  ;
       gPrintf(0,"Board= %d, FirstSlotId= %s, ",dboard, token+1);
       token = strtok(NULL,":\r\n"); 
       if (token == NULL) return 0;
	   gPrintf(0,"NumberOfSlots= %s\nSlots= ",token);
       token = strtok(NULL,":\r\n"); 
       if (token == NULL) return 0;
	   
	   count= (int)strlen(token);
       
	   for(i=0;i<count;i++)
	   { c =token[i+1] ;
	     token[i+1]=0  ;
	     nb = sscanf(token,"%x",&nib);
	     if (nb != 1 ) return 0;
		 for(k=0;k<4;k++)
		 {  if ( (nib & (1<< (3-k))) != 0)
		    { gPrintf(0,"%d ",100*dboard+(first+k));
		      nc++;
		    }
		 }
         token[i+1]=c;
		 first +=   4;
       } 

       token = strtok(NULL,":\r\n"); 
       if (token == NULL) return 0;
       gPrintf(0,"ChangeFlag= %s \n",token);
	   
	   token = strtok(NULL,":\r\n"); 
       if (token == NULL) return nc;
	   
	   break;

	}


	return 0;
}






int InitializeGrid()
{ char resp[MAX_MSG];
  char out[MAX_MSG];
  char data[MAX_MSG];
  int err=0,nc;

  if (maxslots == 0)
  { NBSC=0;
    return(NBSC);
  }

  if (NBSC == 0)
  {
  memset(resp, 0, MAX_MSG);
  memset(out,  0, MAX_MSG);
  memset(data, 0, MAX_MSG); 
  
  err= asciiSendTCP(0, "RESET\n", resp);
  if (err != 0)
  { NBSC=0;
    return NBSC;
  }
  
  memset(resp, 0, MAX_MSG);

  err= asciiSendTCP(0,"CONTENT\n", resp);
  if (err != 0)
  { NBSC=0;
    return NBSC;
  }
  
  resp[3]='.';
  nc=ParseContent(resp+3, out);

  if (nc <= 0) 
  {	  gPrintf(0,"No card detected !!!\n");
      NBSC=0;
      return NBSC;
  }

  NBSC = nc;
  gPrintf(0,"%d cards detected in the grid !!!\n",NBSC);

  memset(resp, 0, MAX_MSG);
  err= asciiSendTCP(0, "QUIT\n", resp);
  if (err != 0)
  { NBSC=0;
    return NBSC;
  }

  }
  
  return(NBSC)  ;
}


int griderr(int client)
{
  shutdown(client,2) ;
  
  #ifndef WIN32
  close(client);
  #else
  closesocket(client);
  #endif

  return -1;
}



int ConnectGridSc(int nbCard, int * sc)
{
	struct sockaddr_in sin,csin   ; 
	int  err, namelen;
	int client;  
	
	char req[MAX_MSG];
	char resp[MAX_MSG];
	char asciiNbCard[16];
	
	int idp;
    idp= indexs[Get_Reader_Index_Abs(nbCard-1)];

	// nbCard= 1...n

    client = (int) socket (AF_INET,SOCK_STREAM,0); 
	*sc= client;

    csin.sin_family = AF_INET   ;  
    csin.sin_port   = 0 ;  
    csin.sin_addr.s_addr =  INADDR_ANY;  
 
    err = bind (client,(struct sockaddr *) &csin, sizeof (csin));	
	if (err != 0)
	{ gPrintf(idp,"Socket Bind Error !!!\n");
	  return -1;
	}

    namelen = sizeof(csin);
    err = getsockname(client, (struct sockaddr *) &csin, &namelen);
 
    sin.sin_family = AF_INET   ;  
    sin.sin_port = htons(gridport) ;  
    sin.sin_addr.s_addr =  inet_addr("127.0.0.1") ;


   if (!SetConnectAddress(&sin,(unsigned short)gridport,gridserver))
   {
	   gPrintf(idp,"DNS error for grid server...\n");
	   return -1;
   }

    err= connect(client,(struct sockaddr *) &sin,sizeof(struct sockaddr) );

	if (err != 0)
	{ gPrintf(idp,"Connection to Grid Server Failed !!!\n");
	  return -1;
	}
    
	err = Recv(client,resp,MAX_MSG,0);
	

	if (err <= 0) 
		return griderr(client) ;
	resp[err]=0;
    
	if (verbose2)
		gPrintf(idp,resp);


	memset(asciiNbCard, 0, 5);
	memset(req, 0, MAX_MSG);
	strcpy(req, "FREE ");
	
	sprintf(asciiNbCard,"%d",slotid[nbCard-1]);
	strcat(req, asciiNbCard);
	strcat(req, "\n");
    
	if (!asciiGridSend(nbCard,client, req, resp))
		return griderr(client) ;

	
	strcpy(req, "USE ");
	
  	sprintf(asciiNbCard,"%d",slotid[nbCard-1]);
	strcat(req, asciiNbCard);
	strcat(req, "\n");
	
	if (!asciiGridSend(nbCard,client, req, resp))
		return griderr(client) ;

    err= Ascii2bin(resp+4,atrgrid[nbCard-1]);
	lenatrgrid[nbCard-1] = err;

	return 1;

}
  
int DeconnectGridSc(int nbCard, int * sc)
{ int client,err;	

  char req[MAX_MSG];
  char resp[MAX_MSG];
  char asciiNbCard[16];	

  client = *sc ;

  memset(asciiNbCard, 0, 5);
  memset(req, 0, MAX_MSG);

  strcpy(req, "FREE ");

  sprintf(asciiNbCard,"%d",slotid[nbCard-1]);

  strcat(req, asciiNbCard);
  strcat(req, "\n");

  err= asciiGridSend(nbCard,client, req, resp);
  
  shutdown(client,2) ;
  #ifndef WIN32
  close(client);
  #else
  closesocket(client);
  #endif

  

  return (0);

}
  


int SendGridSc(int *sc, char* APDU, DWORD APDUlen, char* Response, DWORD* Rlen, int nbCard, int port)
{
	int len, err;
	char  req[MAX_MSG];
	char resp[MAX_MSG];
	char asciiNbCard[16];
    char hexa[3];
	int client;
	int i,v;  
	struct timeb timebuffer1;
    struct timeb timebuffer2;
    long t1,t2,dtm;
	char *ptr;
    
	int idp;
	int fsim=0;

    idp= indexs[Get_Reader_Index_Abs(nbCard-1)];
  
	client = *sc; 

	memset(req,0,sizeof(req));
    memset(resp,0,sizeof(resp));

    ptr=req;

   if ((APDU[0]==(char)0) && (APDU[1]==(char)0xA4) && (APDU[2]==(char)0x04)&& (APDU[3]==(char)0x00) )
	   fsim=1;

	if (fsim) strcpy(ptr, "apdu ");
	else      strcpy(ptr, "tpdu ");

	sprintf(asciiNbCard,"%d",slotid[nbCard-1]);
	
	strcat(ptr, asciiNbCard);
	strcat(ptr, " ");

	if (!fsim)      
	{   if ( (APDUlen == (DWORD)5) && (APDU[4]!=(char)0x00) )
		strcat(ptr, "r ");
	    else
        strcat(ptr, "w ");
	}

	for(i=0;i<(int)APDUlen;i++)
	{  v =0xFF & (int)APDU[i];
	   sprintf(hexa,"%02X",v);
	   strcat(ptr,hexa);
	}
  
	
	if (verbose2)
	{	
		gPrintf(idp,"%s\n",req);
	}

	strcat(ptr, "\n");
	len = (int) strlen(req);

  
    ftime(&timebuffer1);
   
	err = send(client,req,len,0)     ;
	if (err <= 0) 
    { 
	  gPrintf(idp,"Error sending Data to server\n");
 	  return(-1);
    }
    err = Recv(client,resp,MAX_MSG,0);
	if (err <= 0) 
    { return -1;
	}
 
	ftime(&timebuffer2);	
   
    t1 =  (int)((timebuffer1.time % 3600)*1000) +   (int)timebuffer1.millitm   ;
    t2 =  (int)((timebuffer2.time % 3600)*1000) +   (int)timebuffer2.millitm   ;
    dtm = (t2-t1);
    if (dtm <0) dtm += 3600000 ;

	DTM += dtm;
    if (verbose2)
	gPrintf(idp,">> %d ms\n",dtm);
 	resp[err]=0;
	
	if (resp[0] == '0')
	{
		ParseAPDU(resp, req, (int)strlen(resp));

        if (verbose2)
        gPrintf(idp,"%s",req);
      
		*Rlen = Ascii2bin(req+4, Response);
 

		 return 1;
	}

	else 
	{	if (verbose2)
	    gPrintf(idp,"%s",resp);
	    
		*Rlen = 0 ;
         return -1;
	}

	
	return(-1);
}

//======================
// Usefull procedures
//======================
int isDigit(char c)
{ if (((int)c >= (int)'0') && ((int)c<= (int)'9')) return(1);
  if (((int)c >= (int)'A') && ((int)c<= (int)'F')) return(1);
  if (((int)c >= (int)'a') && ((int)c<= (int)'f')) return(1);
  return(0);
}

int Ascii2bin(char *Data_In,char *data_out)
{  	int deb=-1,fin=-1,i,j=0,nc,iCt=0,v,len;
    char c;	
	char data_in[MAX_MSG] ;
    
	len =(int)strlen(Data_In);

	strcpy(data_in,Data_In);

	for(i=0;i<len;i++)
	{ if      ( (deb == -1) && (isDigit(data_in[i])) )             {iCt=1;deb=i;}
      else if ( (deb != -1) && (iCt==1) && (isDigit(data_in[i])) ) {iCt=2;fin=i;}

      if (iCt == 2)
	  { c= data_in[fin+1];
	    data_in[deb+1]= data_in[fin];
		data_in[deb+2]= 0;
	    nc = sscanf(&data_in[deb],"%x",&v);
		data_in[fin+1]=c;

		v &= 0xFF;
		data_out[j++]= v ;
		deb=fin=-1;iCt=0;
	   }
    }



return(j);
}


