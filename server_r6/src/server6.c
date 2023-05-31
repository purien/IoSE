/* server6.c */
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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>


#include "common2.h"
#include "reentrant2.h"
#include "mutuex.h"


#ifndef WIN32
 
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/poll.h>
 #include <sys/ioctl.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 //#include <pcsclite.h>
 #define DWORD long
 #define HWND int  
  
#else
#include <windows.h>
#endif

#ifdef WIN32
// #include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
// #include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

extern void sleep_ms(int milliseconds );

THREAD_CC server_thread(void *arg);

extern int gPrintf(int id,char *fmt, ...);
extern void clist();

#ifndef WIN32
extern int test_sock(int s);
#endif


#define CAFILE     "root.pem"
#define CADIR      "./"
#define CERTFILE   "server.pem"
#define KEYFILE    "serverkey.pem"
#define PASSWORD   "pascal"
#define CIPHER_LIST "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
#define MYSOCKET    "0.0.0.0:8888"

#define MAX_APDU_RESPONSE             4096
#define RACS_RESPONSE_BUFFER_SIZE     8192
#define MAX_SCRIPT_NAME                 64
#define RACS_RESPONSE_BUFFER_MIN_SIZE 1024

//static int sMODE=2 ;
static int sMODE=20 ;

//#define CIPHER_LIST  "AECDH-RC4-SHA"


char cafile[128];
char cadir[128];
char certfile[128];
char keyfile[128];
char password[128];
char cipherlist[128];
char mysocket[128];
int stimeout=600000;

int more_server_tls=1 ;

extern int restricted_seid_list;
extern char * getaid(int num_reader);

int serverdefault()
{
  strcpy(cafile, CAFILE);
  strcpy(cadir,CADIR);
  strcpy(certfile,CERTFILE);
  strcpy(keyfile,KEYFILE);
  strcpy(password,PASSWORD);
  strcpy(cipherlist,CIPHER_LIST);
  strcpy(mysocket,MYSOCKET);

  return 0;

}
 
static int pem_passwd_cb(char *buf,int size,int rwflag, void *passwd)
{ strcpy(buf,password);

	
return((int)strlen(buf));
}

#ifdef OLDS

int select_ecc(char *name,SSL *ssl)
{    int      nid;
     EC_KEY  *ecdh;

    /*
     * Elliptic-Curve Diffie-Hellman parameters are either "named curves"
     * from RFC 4492 section 5.1.1, or explicitly described curves over
     * binary fields. OpenSSL only supports the "named curves", which provide
     * maximum interoperability.
     */

    nid = OBJ_sn2nid((const char *)name);
    if (nid == 0)
		return -1;

    ecdh = EC_KEY_new_by_curve_name(nid);
    if (ecdh == NULL) 
	return -1 ;
   
    SSL_CTX_set_options(ssl->ctx, SSL_OP_SINGLE_ECDH_USE);

    SSL_CTX_set_tmp_ecdh(ssl->ctx, ecdh);

    EC_KEY_free(ecdh);

	return 0;
}

#endif

SSL_CTX *setup_server_ctx(void)
{
    SSL_CTX *ctx;
	//int err;
	
	#ifdef OLDS

	if (sMODE == 1)
	{

    ctx = SSL_CTX_new(SSLv23_method(  ));

	
	SSL_CTX_set_default_passwd_cb(ctx,pem_passwd_cb);

    if (SSL_CTX_use_certificate_chain_file(ctx, certfile) != 1)
        int_error("Error loading certificate from file");
    if (SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM) != 1)
        int_error("Error loading private key from file");
    
	}

	else if (sMODE == 2)
	{
	ctx = SSL_CTX_new(TLSv1_method(  ));

	SSL_CTX_set_default_passwd_cb(ctx,pem_passwd_cb);
    
	if (SSL_CTX_load_verify_locations(ctx, cafile, cadir) != 1)
        int_error("Error loading CA file and/or directory");

    if (SSL_CTX_set_default_verify_paths(ctx) != 1)
        int_error("Error loading default CA file and/or directory");

    if (SSL_CTX_use_certificate_chain_file(ctx,certfile) != 1)
        int_error("Error loading certificate from file");

	if (SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM) != 1)
    int_error("Error loading private key from file");

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,verify_callback);

    SSL_CTX_set_verify_depth(ctx, 4);

	SSL_CTX_sess_set_new_cb(ctx,new_session_cb);

	SSL_CTX_sess_set_remove_cb(ctx,remove_session_cb);

	SSL_CTX_sess_set_get_cb(ctx,get_session_cb);

    SSL_CTX_set_options(ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 |SSL_OP_NO_SSLv3 | SSL_OP_NO_TICKET );
	
	if (SSL_CTX_set_session_id_context(ctx,"SID_CTX",7) != 1)
    int_error("Error Session Caching");

    if (SSL_CTX_set_cipher_list(ctx, cipherlist) != 1)
    int_error("Error setting cipher list (no valid ciphers)");

    }

	else if (sMODE == 3)
	{
	ctx = SSL_CTX_new(TLSv1_method(  ));

	// SSL_CTX_set_default_passwd_cb(ctx,pem_passwd_cb);
  
	SSL_CTX_sess_set_new_cb(ctx,new_session_cb);

	SSL_CTX_sess_set_remove_cb(ctx,remove_session_cb);

	SSL_CTX_sess_set_get_cb(ctx,get_session_cb);

    SSL_CTX_set_options(ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 |SSL_OP_NO_SSLv3 | SSL_OP_NO_TICKET );

	if (SSL_CTX_set_session_id_context(ctx,"SID_CTX",7) != 1)
    int_error("Error Session Caching");


    if (SSL_CTX_set_cipher_list(ctx, cipherlist) != 1)
    int_error("Error setting cipher list (no valid ciphers)");

	SSL_CTX_set_tmp_ecdh_callback(ctx,f1ecc) ;

    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE);

	}
	

	else if (sMODE == 20)
	#endif
	{
    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();      /* load all error messages */

	ctx = SSL_CTX_new(TLSv1_2_method(  ));

	SSL_CTX_set_default_passwd_cb(ctx,pem_passwd_cb);
    
	if (SSL_CTX_load_verify_locations(ctx, cafile, cadir) != 1)
        int_error("Error loading CA file and/or directory");

    if (SSL_CTX_set_default_verify_paths(ctx) != 1)
        int_error("Error loading default CA file and/or directory");

    if (SSL_CTX_use_certificate_chain_file(ctx,certfile) != 1)
        int_error("Error loading certificate from file");

	if (SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM) != 1)
    int_error("Error loading private key from file");

    if(!SSL_CTX_check_private_key(ctx)) 
    int_error("Private key does not match the certificate public key");
    

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,verify_callback);
    SSL_CTX_set_verify_depth(ctx, 2);

	
  // SSL_CTX_sess_set_new_cb(ctx,new_session_cb);
  // SSL_CTX_sess_set_remove_cb(ctx,remove_session_cb);
  // SSL_CTX_sess_set_get_cb(ctx,get_session_cb);

  //err =SSL_CTX_set_options(ctx,SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TICKET | SSL_OP_TLS_ROLLBACK_BUG);
    SSL_CTX_set_options(ctx,SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TICKET | SSL_OP_TLS_ROLLBACK_BUG);
 

 //	if (SSL_CTX_set_session_id_context(ctx,"SID_CTX",7) != 1)
 //   int_error("Error Session Caching");

    if (SSL_CTX_set_cipher_list(ctx, cipherlist) != 1)
    int_error("Error setting cipher list (no valid ciphers)");

    }




	return ctx;
}


static int isDigit(char c)
{ if (((int)c >= (int)'0') && ((int)c<= (int)'9')) return(1);
  if (((int)c >= (int)'A') && ((int)c<= (int)'F')) return(1);
  if (((int)c >= (int)'a') && ((int)c<= (int)'f')) return(1);
  return(0);
}

static int Ascii2bin(char *data_in,char *data_out)
{  	int deb=-1,fin=-1,i,j=0,iCt=0,v,len;//nc,;
    char c;	
    
	len =(int)strlen(data_in);

	for(i=0;i<len;i++)
	{ if      ( (deb == -1) && (isDigit(data_in[i])) )             {iCt=1;deb=i;}
      else if ( (deb != -1) && (iCt==1) && (isDigit(data_in[i])) ) {iCt=2;fin=i;}

      if (iCt == 2)
	  { c= data_in[fin+1];
	    data_in[deb+1]= data_in[fin];
		data_in[deb+2]= 0;
	    //nc = sscanf(&data_in[deb],"%x",&v);
        sscanf(&data_in[deb],"%x",&v);
		data_in[fin+1]=c;

		v &= 0xFF;
		data_out[j++]= v ;
		deb=fin=-1;iCt=0;
	   }
    }

return(j);
}

int DumpBuf(char *buf, int len)
{ int i;
  
  for (i=0;i<len;i++)
  { gPrintf(0,"%2.2X ", buf[i] & 0xFF );
    if (i%16 == 15) gPrintf(0,"\n");
  }
  
  if (i%16 != 15) gPrintf(0,"\n");

  return(0);

}



extern int APDU(int num_reader,char *req, int sreq, char *resp, int*sresp);
extern int TPDU(int num_reader,char *req, int sreq, char *resp, int*sresp, char *sw1, char *fetch, int id);
extern int Get_Reader_Index_Abs(int id);
extern int NBSC    ;
extern int maxslots;
extern int Reader_Nb_On;

extern int checkseid(char *id, int uid, int *seid);
extern char * getlistseid();
extern int getlistmyseid(int uid, char * resp, int *len);

extern int cardreset(int num_reader, int opt,int id);
extern int powerup(int numreader,int id);
extern int powerdown(int numreader,int id);
extern int apdu_firewall(char *apdu, int len, int uid, int seid);

extern char * getsen(int num_reader);

extern int GetUserId(char *name);
extern int closeseid(int sid)   ;
extern int  indexs[]            ;
extern int  setconsole_name(int id, char *name);

int start(int num_reader,char *sen, char *aid, int cwrite);

/*
BEGIN       01
GET-VERSION 02
SET-VERSION 03
LIST	    04
RESET	    05
APDU	    06
SHUTDOWN    07
POWERON     08	
ECHO        09
SEN         10
GET-SEN     11
QUIT        12

+0yz: No Error
-0yz: command Execution Error
-1yz: Unknown Command, the command is not defined by this draft
-2yz: Not imlemented command
-3yz: Illegal command, the command can't be executed
-4yz: Not supported parameter or parameter illegal value
-5yz: Parameter syntax error or parameter missing
-6yz: Unauthorized command
-7yz: Already In Use, a session with this SE is already opened
-8yz: Hardware Error
-9yz: System Error
*/


int process_line(char **token, int nbtoken, char *resp, int resplen, int *request, char *name, int *namelen, int user, int *fappend, int *pline, int sid)
{ int i,err,aid,apdu_req_len,apdu_resp_len,seid ;
  char apdu_req[512]  ;
  char apdu_resp[MAX_APDU_RESPONSE];
  char sw[8],sw1[8],fetch[8];
  char *SW=NULL,*SW1=NULL,*FETCH=NULL;
  char *stoken;
  char *buf   ;
  int  available=0;
  char *AID=NULL;
 

  if ( (*request != 0) && (nbtoken >=1) && (strcmp(token[0], "END") == 0) )
  {  *request=0;
      sprintf(&resp[(int)strlen(resp)],"%s","END\r\n");   
      return 1 ;
  }

  if    (*fappend == 1 );
  else   resp[*pline]=0 ; 
  
  *pline= (int)strlen(resp);
  buf= &resp[*pline];
  *fappend=0;

  available = resplen - *pline;


 	if (strcmp(token[0], "BEGIN") == 0)
	{  if (*request != 0)
	   { sprintf(resp,"BEGIN %s\r\n-301 %03d Duplicate BEGIN\r\nEND\r\n",name,*request-1); 
	     return -2;
	   }
	   else
	   {   *request=1;
	       if (nbtoken == 2) 
		   {
		     *namelen = (int) strlen(token[1]);
			 if (*namelen > MAX_SCRIPT_NAME)
			 { sprintf(resp,"BEGIN %s\r\n-302 %03d Script name > 64\r\nEND\r\n",token[1],*request-1); 
	           return -2;
			 }
             strcpy(name,token[1]) ;
		   } 
		    sprintf(resp,"BEGIN %s\r\n",name) ;
			*pline = (int) strlen(resp);
			sprintf(&resp[*pline],"+001 %03d No error\r\n",*request-1);
			return 0;
	       
	   }
	}


  if (*request == 0)
  {    sprintf(resp,"BEGIN\r\n-300 %03d Invalid command line (%s), waiting for BEGIN\r\nEND\r\n",0,token[0]);
	   return -2;
  }


 if (*request != 0)
 {

  /*
  for (i=1;i<nbtoken;i++)
  { if (strcmp(token[i],"APPEND") == 0)
    { *fappend=1;
      break;
    }
  }
  */
  
  if ( (nbtoken >1) && (strcmp(token[nbtoken-1],"APPEND") == 0) )
  *fappend=1;

 }
 
 
  if (available < RACS_RESPONSE_BUFFER_MIN_SIZE)
  {
    sprintf(resp,"BEGIN %s\r\n-001 %03d memory error\r\nEND\r\n",name,*request-1);
    return -1;
  }


  


  if  (strcmp(token[0], "POWERON") == 0)
  {   
       if (nbtoken < 2)
	   { sprintf(resp,"BEGIN %s\r\n-508 %03d POWERON error, SEID is missing\r\nEND\r\n",name,*request-1);
         return -1;
	   }

   	   aid = checkseid(token[1],user,&seid);
     
	   if (aid < 0)
	   { sprintf(resp,"BEGIN %s\r\n-408 %03d POWERON invalid SEID (%s)\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

	   err = powerup(aid,sid);

	   if (err == -2)
	   { sprintf(resp,"BEGIN %s\r\n-708 %03d POWERON error, SEID %s In Use, SHUTDOWN is required \r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

	  if (err == 2)
	  {  sprintf(buf,"+008 %03d warning SEID %s was already powered, POWERON has been ignored\r\n",*request-1,token[1]);
	     return 0;
	  }


	   if (err != 0)	   
	   { sprintf(resp,"BEGIN %s\r\n-908 %03d POWERON SEID %s system error\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

 


      sprintf(buf,"+008 %03d SEID %s has been powered on\r\n",*request-1,token[1]);
      return 0;
  }


  if  (strcmp(token[0], "SEN") == 0)
  {   
       if (nbtoken < 2)
	   { sprintf(resp,"BEGIN %s\r\n-510 %03d SEN error, SEID is missing\r\nEND\r\n",name,*request-1);
         return -1;
	   }

   	   aid = checkseid(token[1],user,&seid);
     
	   if (aid < 0)
	   { sprintf(resp,"BEGIN %s\r\n-410 %03d SEN invalid SEID (%s)\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

	   err = powerdown(aid,sid);
       
	   if (err != 0)	   
	   { sprintf(resp,"BEGIN %s\r\n-910 %03d SEN %s system error \r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

      if      (nbtoken == 2) err= start(aid,NULL,NULL,1);
      else if (nbtoken == 3) err= start(aid,token[2],NULL,1);
	  else                   err= start(aid,token[2],token[3],1);

    	    
	  if  (err == 0) 
	  {  if (getsen(aid) == NULL)
	     { sprintf(resp,"BEGIN %s\r\n-910 %03d SEN %s system error\r\nEND\r\n",name,*request-1,token[1]);
	       return -1;
	     }
		 else
		 { AID=getaid(aid);
		   if (AID != NULL) 
		   { if (AID[0] != 0) sprintf(buf,"+010 %03d SEN= %s AID= %s\r\n",*request-1,getsen(aid),AID) ; 
		     else             sprintf(buf,"+010 %03d SEN= %s AID= default\r\n",*request-1,getsen(aid));           
	       }
	     }
	  }

	  else
	  { sprintf(resp,"BEGIN %s\r\n-910 %03d SEN %s system error\r\nEND\r\n",name,*request-1,token[1]);
	    return -1;
	  }

      return 0;
  }


  if  (strcmp(token[0], "GET-SEN") == 0)
  {   
       if (nbtoken < 2)
	   { sprintf(resp,"BEGIN %s\r\n-511 %03d GET-SEN error, SEID is missing\r\nEND\r\n",name,*request-1);
         return -1;
	   }

   	   aid = checkseid(token[1],user,&seid);
     
	   if (aid < 0)
	   { sprintf(resp,"BEGIN %s\r\n-511 %03d GET-SEN invalid SEID (%s)\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

       if (getsen(aid) == NULL)
	   { sprintf(resp,"BEGIN %s\r\n-911 %03d SEID %s has no SEN\r\n",name,*request-1,token[1]);
	     return -1;
	   }
	   else
	   { sprintf(buf,"+011 %03d %s\r\n",*request-1,getsen(aid));
	     AID=getaid(aid);
	     if (AID != NULL) 
		 { if (AID[0] != 0) sprintf(buf,"+011 %03d %s [AID= %s]\r\n",*request-1,getsen(aid),AID) ; 
		   else             sprintf(buf,"+011 %03d %s [AID= default]\r\n",*request-1,getsen(aid));           
	     }
	   }
  
      return 0;
  }

  if  (strcmp(token[0], "XEND") == 0)
  {   sprintf(buf,"+012 %03d XEND\r\n",*request-1);
      return 100;
  }


if  (strcmp(token[0], "QUIT") == 0)
  {   sprintf(buf,"+012 %03d QUIT\r\n",*request-1);
      return 100;
  }


  if  (strcmp(token[0], "SHUTDOWN") == 0)
  {   
       if (nbtoken < 2)
	   { sprintf(resp,"BEGIN %s\r\n-507 %03d SHUTDOWN error, SEID is missing\r\nEND\r\n",name,*request-1);
         return -1;
	   }

   	   aid = checkseid(token[1],user,&seid);
     
	   if (aid < 0)
	   { sprintf(resp,"BEGIN %s\r\n-407 %03d SHUTDOWN invalid SEID %s\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

	   err = powerdown(aid,sid);
       
	   if (err != 0)	   
	   { sprintf(resp,"BEGIN %s\r\n-907 %03d SHUTDOWN %s system error\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

      sprintf(buf,"+007 %03d SEID %s has been powered off\r\n",*request-1,token[1]);
      return 0;
  }

  if  (strcmp(token[0], "RESET") == 0)
  {   
       if (nbtoken < 2)
	   { sprintf(resp,"BEGIN %s\r\n-505 %03d RESET error, SEID is missing\r\nEND\r\n",name,*request-1);
         return -1;
	   }

   	   aid = checkseid(token[1],user,&seid);
     
	   if (aid < 0)
	   { sprintf(resp,"BEGIN %s\r\n-405 %03d RESET invalid SEID (%s)\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

	   if ((nbtoken >= 3) && (strcmp(token[2],"WARM")==0) )
	   { err = cardreset(aid,1,sid);
	     if (err ==0) 
			 sprintf(buf,"+005 %03d SEID %s has been resetted (warm)\r\n",*request-1,token[1]);
	   }

	   else
	   { err = cardreset(aid,0,sid);
         if (err ==0) 
			 sprintf(buf,"+005 %03d SEID %s has been resetted\r\n",*request-1,token[1]);
	   }

	   if (err == -2)
	   { sprintf(resp,"BEGIN %s\r\n-705 %03d RESET error, SEID %s already In Use, SHUTDOWN is required \r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

	   if (err == -3)
	   { sprintf(resp,"BEGIN %s\r\n-705 %03d RESET error, SEID %s is powered off, POWERON is required \r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

       if (err != 0)	   
	   { sprintf(resp,"BEGIN %s\r\n-905 %03d RESET system error (%s)\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }

      //sprintf(buf,"+005 %03d SEID %s has been resetted\r\n",*request-1,token[1]);

      return 0;
  }



  if  (strcmp(token[0], "GET-VERSION") == 0)
  {   sprintf(buf,"+002 %03d 0.3\r\n",*request-1);
      return 0;
  }

  if  (strcmp(token[0], "SET-VERSION") == 0)
  {  
	  if (nbtoken < 2)
	   { sprintf(resp,"BEGIN %s\r\n-502 %03d SET-VERSION error, version value is missing\r\nEND\r\n",name,*request-1);
         return -1;
	   }

	  if (strcmp(token[1],"0.3") != 0) 
	   { sprintf(resp,"BEGIN %s\r\n-402 %03d SET-VERSION error, %s is not supported\r\nEND\r\n",name,*request-1,token[1]);
         return -1;
	   }


	  sprintf(buf,"+002 %03d Version %s is running\r\n",*request-1,token[1]);
      return 0;

  }

  /////////////////////////test ECHO/////////////////////////////////////////
  if  (strcmp(token[0], "ECHO") == 0)
  {  
	  if (nbtoken < 2)
	   { sprintf(resp,"BEGIN %s\r\n-509 %03d ECHO error, value is missing\r\nEND\r\n",name,*request-1);
         return -1;
	   }

	  sprintf(buf,"+009 %03d %s\r\n",*request-1,token[1]);
      return 0;

  }


  if  (strcmp(token[0], "LIST") == 0)
  {   
	  sprintf(buf,"+004 %03d ",*request-1);

      if (restricted_seid_list == 0)
		  sprintf(&buf[(int)strlen(buf)],"%s",getlistseid());
 
	  else
	  {   resplen -= (int)strlen(buf);
		  err= getlistmyseid(user,&buf[(int)strlen(buf)],&resplen);
	  }

	  sprintf(&buf[(int)strlen(buf)],"\r\n");
	  
      
	  return 0;
  }

	
   if  (strcmp(token[0], "APDU") == 0)
   {   
	   if (nbtoken < 3)
	   { sprintf(resp,"BEGIN %s\r\n-506 %03d APDU error, too few parameters\r\nEND\r\n",name,*request-1);
         return -1;
	   }

   	   aid = checkseid(token[1],user,&seid);
     
	   if (aid < 0)
	   { sprintf(resp,"BEGIN %s\r\n-406 %03d APDU invalid SEID\r\nEND\r\n",name,*request-1);
         return -1;
	    }

		apdu_req_len=  Ascii2bin(token[2],apdu_req) ;
		if (apdu_req_len <= 0)
		{sprintf(resp,"BEGIN %s\r\n-506 %03d APDU: invalid apdu format\r\nEND\r\n",name,*request-1);
         return -1;
		}
		   
        if (nbtoken > 3)
		{ for (i=3;i<nbtoken;i++)
		  {
            
            stoken = strtok(token[i],"=");

			if (stoken != NULL)
			{ 
			if (strcmp(stoken, "CONTINUE")==0)
			  {	//stoken = strtok(NULL,"");
                stoken= strtok(stoken+strlen(stoken)+1,"");
			    if (stoken == NULL);
				else if (strlen(stoken) != 4);
				else { 
					   err=Ascii2bin(stoken,sw) ;
					   if (err == 2) SW=sw;
				      }
			  }
			
			else if (strcmp(stoken, "FETCH")==0)
			  {	//stoken = strtok(NULL,"");
			    stoken= strtok(stoken+strlen(stoken)+1,"");
			    if (stoken == NULL);
				else if (strlen(stoken) != 8);
				else { 
					   err=Ascii2bin(stoken,fetch) ;
					   if (err == 4) FETCH=fetch;
				      }
			  }

			else if (strcmp(stoken, "MORE")==0)
			  {	//stoken = strtok(NULL,"");
                stoken= strtok(stoken+strlen(stoken)+1,"");
			    if (stoken == NULL);
				else if (strlen(stoken) != 2);
				else { 
					   err=Ascii2bin(stoken,sw1) ;
					   if (err == 1) SW1=sw1     ;
				     }
			  }

			}

			}
        
          }


         err=  apdu_firewall(apdu_req,apdu_req_len,user,seid);
		 if (err <0)
         {	gPrintf(indexs[aid],"%d %s has been firewalled...\n",sid,token[2]);
			if (err == -2)
            sprintf(resp,"BEGIN %s\r\n-706 %03d APDU error, SEID %s, no AID has been selected\r\nEND\r\n",name,*request-1,token[1]);
			else if (err == -3)
            sprintf(resp,"BEGIN %s\r\n-706 %03d APDU error, SEID %s, APDU has been firewalled\r\nEND\r\n",name,*request-1,token[1]);
  		    else
			sprintf(resp,"BEGIN %s\r\n-706 %03d SELECT error, SEID %s, invalid AID, APDU has been firewalled\r\nEND\r\n",name,*request-1,token[1]);
            return -1;
		 }


         apdu_resp_len= (int)sizeof(apdu_resp);
		 err= TPDU(aid,apdu_req,apdu_req_len,apdu_resp,&apdu_resp_len,SW1,FETCH,sid);

		 if (err == -2)
		 {	sprintf(resp,"BEGIN %s\r\n-706 %03d APDU error, SEID %s already In Use, SHUTDOWN required \r\nEND\r\n",name,*request-1,token[1]);
            return -1;
		 }

		 if (err == -3)
		 {	sprintf(resp,"BEGIN %s\r\n-706 %03d APDU error, SEID %s is powered off, POWERON is required \r\nEND\r\n",name,*request-1,token[1]);
            return -1;
		 }

         if (err == -5)
		 {	sprintf(resp,"BEGIN %s\r\n-706 %03d APDU error, SEID %s response overflow \r\nEND\r\n",name,*request-1,token[1]);
            return -2;
		 }

         
		 if ( (err != 0) || (apdu_resp_len < 2) )
		 {	sprintf(resp,"BEGIN %s\r\n-806 %03d APDU execution error\r\nEND\r\n",name,*request-1);
            return -1;
		 }

		 if (SW != NULL)
		 { 
		 if ( (SW[0] != apdu_resp[apdu_resp_len-2]) || (SW[1] != apdu_resp[apdu_resp_len-1]) )
		 { sprintf(resp,"BEGIN %s\r\n-006 %03d APDU wrong status\r\nEND\r\n",name,*request-1);
		   return -1;
		 }
		 }
           
		 
		 if (available < (RACS_RESPONSE_BUFFER_MIN_SIZE + (2*apdu_resp_len))  )
         {
           sprintf(resp,"BEGIN %s\r\n-706 %03d APDU Memory Error\r\nEND\r\n",name,*request-1);
           return -1;
         }

		 sprintf(buf,"+006 %03d ",*request-1);  

		 for (i=0;i<apdu_resp_len;i++)
    	 sprintf(&buf[(int)strlen(buf)],"%02X",0xFF & apdu_resp[i]);

         sprintf(&buf[(int)strlen(buf)],"\r\n");
		 return 0;
		 }


    sprintf(resp,"BEGIN %s\r\n-900 %03d %s unknown command...\r\nEND\r\n",name,*request-1,token[0]);
    return -2;

}






//typedef   struct { int sessionid; SSL *ssl; } PARAMS;

extern int startnewconsole(char *name);
extern int closeconsole(int index);
extern HWND gethWnd(int id);

int close_session_console=1;
int racs_verbose=0;
int racs_log=1;
int close_session_delay=0;
int session_console_tile=0;

extern int tile();

char strace[128]= {"./" };
extern int more_main   ;


int do_server_loop(SSL* ssl, int sid)
{
    int  err,len   ;
 	int nwritten   ;
    char buf[17000],c,resp[RACS_RESPONSE_BUFFER_SIZE];

	int i,ic=-1;

    char *token=NULL ;
    //SSL *ssl=NULL;
	char seps[] = {" \r\n"};
	char *list[10];
	int nba=0,nb=0,resplen,fappend=0,pline=0 ;

	char name[64]="";
	int request=0,namelen,flush=0; 
	int sock;

    struct timeval timeout;
    
    #ifdef WIN32
    fd_set a_fd_set ;
    #endif

    //struct timeb timebuffer1;
    //struct timeb timebuffer2;
    //int t1=0,t2=0;
	int dtm=0;
	int rmore=0;

    time_t tv1,tv2;

    X509      *cert;
    X509_NAME *subj;
	char CommonName[256];
	char sn[300];

	int uid,n=0;//,sessionid;
	//PARAMS *params;

	//params = (PARAMS *)p        ;
	//ssl =  params->ssl    ;
	//sessionid = params->sessionid;
	//free (p);

     struct sockaddr_in sin;
     char ip[32];
	 char fname[MAX_SCRIPT_NAME+1];
	 int port=0;
     FILE *f = NULL ;
     
	 time_t now;
     struct tm tm_now;
     //char s_now[sizeof "JJ/MM/AAAA HH:MM:SS"];
     char s_now[128];
	 
	 int fdata=0;
	 int fquit=0;
	 int ret=0;
	 
	 #ifndef WIN32
	 struct pollfd fds[1];
	 #endif

    /* lire l'heure courante */
    now = time(NULL);
    /* la convertir en heure locale */
    tm_now = *localtime (&now);
    strftime (s_now, sizeof s_now, "%d/%m/%Y %H:%M:%S", &tm_now);
 

	 if (racs_log == 1)
     {	 sprintf(fname,"%sracs_log_%d.txt",strace,sid);
		 f= fopen(fname,"w+");
	 }

	 
	cert = SSL_get_peer_certificate(ssl);
	if (cert == NULL) 
	{   gPrintf(0,"No Certificate Error");
		return 0;
	}
	

	//X509_get_subject_name() returns the subject name of certificate x. 
	//The returned value is an internal pointer which MUST NOT be freed.
	subj = X509_get_subject_name(cert);

    if (X509_NAME_get_text_by_NID(subj, NID_commonName, CommonName, (int)sizeof(CommonName)) <= 0)
	{ X509_free(cert); 
	  gPrintf(0,"Certificate Format Error");
      goto goodbye;
	   //return 0; 
	}
	
    //The reference count of the X509 object is incremented by one, 
	//so that it will not be destroyed when the session containing 
	//the peer certificate is freed. The X509 object must be explicitly 
	//freed using X509_free()

	X509_free(cert);
	uid = GetUserId(CommonName);

    //ssl->session->session_id	

	if (racs_verbose == 1)
	{ sprintf(sn,"Session %d Users %s",sid,CommonName);
	  ic= startnewconsole((char *)&sn[0]);
      if (session_console_tile == 1)
	    tile();
	}


	namelen= (int)sizeof(name) ;
	resplen= (int)sizeof(resp) ;
	name[0]=0;
	resp[0]=0;
	fappend=0;
	pline=0;
	request=0;
	flush=0;
	
	
   //err= BIO_get_fd(ssl->wbio, &sock);
   sock= SSL_get_fd(ssl);
   timeout.tv_sec  = 1  ; // secondes
   timeout.tv_usec = 0  ;

  
    //ftime(&timebuffer1); 
    //t1 =  (int)((timebuffer1.time % 3600)*1000) +   (int)timebuffer1.millitm   ;
    tv1= time(NULL)*1000;

   ip[0]=0;
   len = (int)sizeof(sin);
   if (getsockname(sock, (struct sockaddr *)&sin, &len) != 0); //perror("Error on getsockname");
   else
   { strcpy(ip, inet_ntoa(sin.sin_addr)); // IP = 0.0.0.0
     port = sin.sin_port;
   }

  
	if (ic >=0)
		gPrintf(ic,"(%s) Client (%s:%d) CN=%s is connected (uid=%d, sid=%d)...\n",s_now,ip,port,CommonName,uid,sid);
	else
        gPrintf(0,"(%s) Client (%s:%d) CN=%s is connected (uid=%d, sid=%d)...\n",s_now,ip,port,CommonName,uid,sid);

    if (f != NULL)
		fprintf(f,"(%s) Client (%s:%d) CN=%s is connected (uid=%d, sid=%d)\r\n",s_now,ip,port,CommonName,uid,sid);


	while(more_server_tls)
	{ 
    
    if ((int)sizeof(buf)-1-nb <= 0)
		break;
     
	 rmore=1;

	 while(rmore)
	 { 
     fdata=0;
	 
	 #ifndef WIN32
	 
	 memset(fds, 0 , sizeof(fds));
     fds[0].fd = sock ;
	 fds[0].events = POLLIN;
	 err = poll(fds,1, 1000*(int)timeout.tv_sec);
	 if (err < 0) 
		 goto goodbye;
	 if (err == 0); // timeout
	 else if(fds[0].revents != POLLIN) 
		 goto goodbye;
	 else  if (fds[0].fd == sock) 
		 {fdata=1;rmore=0;} //data received
	 else 
		 goto goodbye;
	 
	 #else
	 FD_ZERO(&a_fd_set)       ;
     FD_SET(sock,&a_fd_set)   ;
     err = select (1+sock,&a_fd_set,NULL,NULL,&timeout);
	 if (err < 0) goto goodbye;
     if ( FD_ISSET(sock, &a_fd_set ) ) {fdata=1;rmore=0;} //data
	 #endif

	 if (!fdata) //timeout
	 {   
          //ftime(&timebuffer2);	
          //t2 =  (int)((timebuffer2.time % 3600)*1000) +   (int)timebuffer2.millitm   ;
          
		  tv2= time(NULL)*1000;
		  dtm = (int)(tv2-tv1); // ms
    
	      //if (dtm <0) dtm += 3600000; 

		  // gPrintf(0,"s=%d:t=%d %d\n",sid,dtm,SSL_pending(ssl) );
		  if (ic >=0)
		  {   sprintf(sn,"Session %d Users %s time %d",sid,CommonName,dtm/1000); 
              #ifdef WIN32
			  //SetConsoleTitle(sn);
              setconsole_name(ic,sn);
		      // SetWindowText(gethWnd(ic),sn);
              #endif
		  }


		  if (dtm > stimeout)
		  {  gPrintf(0,"Timeout (sid=%d)\n",sid);
		     goto goodbye;
		  }

	 } //!fdata
	 } //while(rmore)
	 	 


	ret= err = SSL_read(ssl, &buf[nb], (int)sizeof(buf)-1-nb);
    
	if (err <= 0) 
	  break ;
	

	buf[err]=0 ;
	
	if (f != NULL)
		fprintf(f,"%s",buf);

	len=err;
	nb+=len;

	for(i=0;i<nb;i++)
	{ if (buf[i] == (char)'\n')
	  { 
		c= buf[i+1]; buf[i+1]=0 ;
        if (ic >=0) gPrintf(ic,"%s",buf);
	    nba=0;
	    token = strtok(buf,seps);
		
		if (token == NULL) 
			goto goodbye ;

	     while(token != NULL)
	     {if (nba >= (int)sizeof(list)) 
		   goto goodbye ;
	      list[nba++]= token ;
	      //token = strtok(NULL,seps);
		  token= strtok(token+1+strlen(token),seps);
	     }

		 //=========================line=============================================
		 resplen= sizeof(resp);
		 if      ( (request == 0) && (flush==1) && (strcmp(list[0],"END")== 0) ) err=flush=0;
         else if (flush ==0)
		 {	 
			 err= process_line(list,nba,resp,resplen,&request,name,&namelen,uid,&fappend,&pline,sid);

		 //  0 ligne de commande OK, reponse OK, pas d'envoi
		 //  1 reponse OK envoi du message reponse, fin de requete (END)
		 //  100 ligne de commande OK, reponse OK, pas d'envoi deconnexion apres END
		 // -2 erreur fatale => fin de session, deconnexion
		 // -1 flush, lignes restantes ingnorees jusqu'a END, pas de deconnexion 
		
		 if (err == 100) {err=0;fquit=1;}
	     if   (request == 1) flush=0;
		 if   (request != 0) request++;
		 if ( (request != 0) && (err == -1) ) flush=1; 
		 
		 }
		  //=======================End of Line=======================================
         n=err;

		 buf[i+1]=c;
		 if (nb != (i+1)) memmove(buf,&buf[i+1],nb-1 -i-1 +1);
		 nb = nb - i - 1;
		 i=0;
	
        if ( (err == 1) || (err != 0) )
		{  	
           if (fquit && (err==1)) fquit=2;

           if (ic >=0)
			 gPrintf(ic,"\n%s",resp);
	     
		   if (f != NULL)
			   fprintf(f,"%s",resp);

		   resplen = (int)strlen(resp);
           for (nwritten = 0;  nwritten < resplen;  nwritten += err)
           {
            err = SSL_write(ssl, resp + nwritten, resplen - nwritten);
   	        if (err <= 0)   
				goto goodbye ;
            }

		   request=0;
		   namelen=(int)strlen(name);
		   name[0]=0;
           resp[0]=0;
           resplen= (int)sizeof(resp);
		   fappend=0;
           pline=0;

		   if (fquit == 2) goto goodbye;
		}

		 if (n == -2) // fatal error
			   goto goodbye ;


	  
	} // line has been processed

 
	} // looking for a new line


	} // Read a new TLS message

goodbye:
	// shutdown all powered seid
    n=closeseid(sid);
  
	now = time (NULL);
    tm_now = *localtime (&now);
    strftime (s_now, sizeof(s_now), "%d/%m/%Y %H:%M:%S", &tm_now);

	gPrintf(0,"(%s) End of session (sid=%d), %d secure elements has been powered off\n",s_now,sid,n);

	if (f != NULL)
	{   fprintf(f,"(%s) End of session (sid=%d), %d secure elements has been powered off\r\n",s_now,sid,n);
		fclose(f);
		f=NULL;
	}

	if ( (ic >=0) && (close_session_console == 1) )
	{	if (close_session_delay > 0) sleep_ms(close_session_delay); 
	    closeconsole(ic);
	}
	
	     
    //Note that SSL_shutdown() must not be called 
	//if a previous fatal error has occurred on a connection 
	//i.e. if SSL_get_error() has returned SSL_ERROR_SYSCALL or SSL_ERROR_SSL.
    
	// In order to complete the bidirectional shutdown handshake, 
	// the peer needs to send back a close_notify alert. 
	// The SSL_RECEIVED_SHUTDOWN flag will be set after receiving and processing it.

	err= SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN ;
    err= SSL_get_error(ssl,ret);
	if ( (err == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL) )
	return 0;

    if (SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN)
		return 1;
	else 
		return 0;
	
}

int nbsession=0;

void addx(int *x,int n,int *r)
 { static int y=0;
   MUTEX_LOCK(Pmutex[M_SYSTEM]);
   *x = *x + n  ;
   if (n>=0) y++;
   if (r != NULL) *r=y;
   MUTEX_UNLOCK(Pmutex[M_SYSTEM]);
}

THREAD_CC server_thread(void *arg)
{   //int err;
    //PARAMS *params;
    SSL *ssl = (SSL *)arg;
	int sid,err;
	int err1=0;
	int sock=0;


#ifndef WIN32
    pthread_detach(pthread_self(  ));
#endif

     if (SSL_accept(ssl) <= 0)
	   { // Perte de connection TCP/IP
		gPrintf(0, "SSL server_thread, error accepting SSL connection (wrong certificate ...)\n");
        // SSL_shutdown(ssl);
        err= SSL_clear(ssl);
        SSL_free(ssl);
        //ERR_remove_state(0); 
        ERR_remove_thread_state(NULL) ;
	
	    #ifdef WIN32
        ExitThread(0);
        #else
        pthread_exit(NULL);
		#endif
	
		return 0;
	   }

	//////////////////////////////////
    //err1= BIO_get_fd(ssl->wbio, &sock);
	//err1++;
	sock=SSL_get_fd(ssl);
    //////////////////////////////////

    addx(&nbsession,1,&sid);

	//if (sid == 193)
	//	sid=92;

	gPrintf(0,"RACS session #%d (sid) opened (total #sessions %d)\n",sid, nbsession);
	
	err=do_server_loop(ssl,sid);


	if (err) 
	{ err= SSL_shutdown(ssl);}
    else     
	{ err= SSL_clear(ssl) ;}
    
	SSL_free(ssl);

	
    #ifndef WIN32
 	if (test_sock(sock) == 1) 
	err= shutdown(sock,1);	
	err= shutdown(sock,2);
	//err=0;	 
	if (err != 0)
	gPrintf(0,"SSL close error= %d\n",errno) ; 
	//sleep_ms(wait_timeout);
    #endif
	
	

	//////////////////////////////////
    #ifdef WIN32
	err= closesocket(sock);
    #else
    close(sock);
    #endif
	//////////////////////////////////
    
	//ERR_remove_state(0); 
    ERR_remove_thread_state(NULL) ;

    addx(&nbsession,-1,NULL);
	gPrintf(0,"RACS session #%d (sid) closed (total #sessions %d)\n", sid, nbsession);


#ifdef WIN32
ExitThread(0);
#else
pthread_exit(NULL);
#endif
	   
return(0);
}


THREAD_CC server_threada(void *arg)
{   //int err;
    //PARAMS *params;
    SSL *ssl = (SSL *)arg;
	int sid,err;
	int err1=0;
	int sock=0;


#ifndef WIN32
    pthread_detach(pthread_self());
#endif

     if (SSL_accept(ssl) <= 0)
	   { // Perte de connection TCP/IP
		gPrintf(0, "SSL server_thread, error accepting SSL connection (wrong certificate ...)\n");
        SSL_shutdown(ssl);
        err= SSL_clear(ssl);
        // SSL_free(ssl);
        // ERR_remove_state(0); 
        ERR_remove_thread_state(NULL) ;
	
	    #ifdef WIN32
        ExitThread(0);
        #else
        pthread_exit(NULL);
		#endif
	
		return 0;
	   }

	//////////////////////////////////
    //err1= BIO_get_fd(ssl->wbio, &sock);
	sock=SSL_get_fd(ssl);
	//////////////////////////////////

    addx(&nbsession,1,&sid);

	//if (sid == 193)
	//	sid=92;

	gPrintf(0,"RACS session #%d (sid) opened (total #sessions %d)\n",sid, nbsession);
	
	err=do_server_loop(ssl,sid);

	if (err) 
	{ err= SSL_shutdown(ssl);}
    //else     
	{ err= SSL_clear(ssl) ;}
    
	//SSL_free(ssl);

	
    #ifndef WIN32
 	if (test_sock(sock) == 1) 
	err= shutdown(sock,1);	
	err= shutdown(sock,2);
	//err=0;	 
	if (err != 0)
	gPrintf(0,"SSL close error= %d\n",errno) ; 
	//sleep_ms(wait_timeout);
    #endif
	
	

	//////////////////////////////////
    #ifdef WIN32
	err= closesocket(sock);
    #else
    close(sock);
    #endif
	//////////////////////////////////
    
	//ERR_remove_state(0); 
    ERR_remove_thread_state(NULL) ;

    addx(&nbsession,-1,NULL);
	gPrintf(0,"RACS session #%d (sid) closed (total #sessions %d)\n", sid, nbsession);


#ifdef WIN32
ExitThread(0);
#else
pthread_exit(NULL);
#endif
	   
return(0);
}








static SSL_CTX * ctx_server=NULL ;



THREAD_CC server_thread3(void *arg)
{   int sock=*((int *)arg);
    SSL *ssl=NULL ;
	int sid=0,err=0;
	int err1=0;

#ifndef WIN32
    pthread_detach(pthread_self(  ));
#endif


        ssl = SSL_new(ctx_server) ;
		if (ssl == NULL)
		{ gPrintf(0, "RACS server, error creating SSL context\n");
		  goto endt3;
		}
    
	 SSL_set_fd(ssl,sock)   ;
	 addx(&nbsession,1,&sid);

     if (SSL_accept(ssl) <= 0)
	   { // Perte de connection TCP/IP
		gPrintf(0, "SSL server_thread, error accepting SSL connection (wrong certificate ...)\n");
        //SSL_shutdown(ssl)  ;
		goto end_session   ;
       }

  
	gPrintf(0,"RACS session #%d (sid) opened (total #sessions %d)\n",sid, nbsession);
	
    err=do_server_loop(ssl,sid);
 
	if (err) 
	err= SSL_shutdown(ssl);
         
	//err= SSL_clear(ssl) ;
  	//SSL_free(ssl);

end_session:	
	
    #ifndef WIN32
 	if (test_sock(sock) == 1) 
	err= shutdown(sock,1);	
	err= shutdown(sock,2);
	if (err != 0)
	gPrintf(0,"SSL close error= %d\n",errno) ; 
	//sleep_ms(wait_timeout);
    #endif
	

	//////////////////////////////////
    #ifdef WIN32
	err= shutdown(sock,1);
    err= shutdown(sock,2);
    //if (err != 0) gPrintf(0,"shutdow error = %d\n",errno);
	err= closesocket(sock);
    #else
	//close(sock) ;//;=> crash sometimes
    #endif
	//////////////////////////////////
    
	err= SSL_clear(ssl) ;
  	SSL_free(ssl);

		
	//ERR_remove_thread_state(NULL) ;

    addx(&nbsession,-1,NULL);
	gPrintf(0,"RACS session #%d (sid) closed (total #sessions %d)\n", sid, nbsession);

endt3:


#ifdef WIN32
ExitThread(0);
#else
pthread_exit(NULL);
#endif
	   
return(0);
}

static BIO *acc =NULL;

int stop_ssl_server()
{if (acc) BIO_free(acc);
 acc= NULL;
 return 0;
}

THREAD_CC daemon_thread2a(void *arg);

#define NSSL 128
#define NSSLMASK (NSSL-1)
int _codet2(void)
{ SSL * ssl[NSSL] ;
  THREAD_TYPE tid;
  int i,err;
  unsigned short port= 443;
  char host[200];
  int s;
  struct sockaddr_in addr;
  int nsclient=0;
  unsigned int lenaddr;
  int sock;
  
 
#ifndef WIN32
//int pid;
int reuse=1,ret;
#endif
    
    //nbsession=0;
	//y=0;

    for (i=0;i<(int)strlen(mysocket);i++)
	{	if (mysocket[i]==(char)':')
	    { sscanf(&mysocket[i+1],"%hu",&port);
	      mysocket[i]= 0 ;
		  strcpy(host,mysocket);
		  mysocket[i]= (char)':'; 
	      break;
	    }
	}

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr (host); //htonl(INADDR_ANY);

    s = (int) socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) 	
	{ 
	perror("Unable to create socket");
	exit(EXIT_FAILURE);
    }

    #ifndef WIN32
	ret= setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(void *)&reuse,sizeof(reuse));
	ret= setsockopt(s, SOL_SOCKET, SO_REUSEPORT,(void *)&reuse,sizeof(reuse));
	ret++;
    #endif

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
	{
	perror("Unable to bind");
	exit(EXIT_FAILURE);
    }

    if (listen(s, 10) < 0) {
	perror("RACS Server Unable to listen");
	exit(EXIT_FAILURE);
    }

    lenaddr = (int)sizeof(addr)    ;
    ctx_server = setup_server_ctx();

	for(i=0;i<NSSL;i++)
	{ ssl[i] = SSL_new(ctx_server);
	  if (ssl[i] == NULL)
	  { perror("RACS Server SSL Error");
	    exit(EXIT_FAILURE);
	  }
     SSL_clear(ssl[i]);
	 err=SSL_get_state(ssl[i]);
	}
  
    while(1)
    {   
        
		gPrintf(0,"RACS server ready on port %s:%u (total #sessions %d)\n",host,port,nbsession);
        sock = (int)accept(s,(struct sockaddr*)&addr, &lenaddr);
       
		if (sock < 0) 
		{   gPrintf(0,"RACS Server error, unable to accept client\n");
		    break;
		}
        
		err= SSL_get_state(ssl[nsclient]);
		if (err != (int)0x6000)
		{ shutdown(sock,2);
		  #ifdef WIN32
	      closesocket(sock);
          #else
	      close(sock);
          #endif
          nsclient++;
		  nsclient = nsclient & NSSLMASK;
		  continue;
		}
        SSL_clear(ssl[nsclient]);
        SSL_set_fd(ssl[nsclient],sock);		
		THREAD_CREATE(tid, (void *)server_threada,(void*)(ssl[nsclient]));
        nsclient++;
		nsclient = nsclient & NSSLMASK;
    }
   
    if (ctx_server  != NULL) SSL_CTX_free(ctx_server);
    EVP_cleanup();

    #ifdef WIN32
	closesocket(s);
    #else
	close(s);
    #endif

	gPrintf(0,"End of RACS Server Thread\n");
    more_main=0;
	return 0;
}

int codet2(void)
{int sclient[NSSL] ;
  static THREAD_TYPE tid;
  int i;
  unsigned short port= 443;
  char host[200];
  int s;
  struct sockaddr_in addr;
  int nsclient=0;
  unsigned int lenaddr;
  
 
#ifndef WIN32
//int pid;
int reuse=1,ret;
#endif
    
    //nbsession=0;
	//y=0;

    for (i=0;i<(int)strlen(mysocket);i++)
	{	if (mysocket[i]==(char)':')
	    { sscanf(&mysocket[i+1],"%hu",&port);
	      mysocket[i]= 0 ;
		  strcpy(host,mysocket);
		  mysocket[i]= (char)':'; 
	      break;
	    }
	}

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr (host); //htonl(INADDR_ANY);

    s = (int) socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) 	
	{ 
	perror("Unable to create socket");
	exit(EXIT_FAILURE);
    }

    #ifndef WIN32
	ret= setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(void *)&reuse,sizeof(reuse));
	ret= setsockopt(s, SOL_SOCKET, SO_REUSEPORT,(void *)&reuse,sizeof(reuse));
	ret++;
    #endif

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
	{
	perror("Unable to bind");
	exit(EXIT_FAILURE);
    }

    if (listen(s, 10) < 0) {
	perror("RACS Server Unable to listen");
	exit(EXIT_FAILURE);
    }

    lenaddr = (int)sizeof(addr)    ;
    ctx_server = setup_server_ctx();

  
    while(1)
    {   
        
		gPrintf(0,"RACS server ready on port %s:%u (total #sessions %d)\n",host,port,nbsession);
        sclient[nsclient] = (int)accept(s,(struct sockaddr*)&addr, &lenaddr);
       
		if (sclient[nsclient] < 0) 
		{   gPrintf(0,"RACS Server error, unable to accept client\n");
		    break;
		}

		THREAD_CREATE(tid, (void *)server_thread3,(void*)(&sclient[nsclient]));
        nsclient++;
		nsclient = nsclient & NSSLMASK;
    }
   
    if (ctx_server  != NULL) SSL_CTX_free(ctx_server);
    EVP_cleanup();

    #ifdef WIN32
	closesocket(s);
    #else
	close(s);
    #endif

	gPrintf(0,"End of RACS Server Thread\n");
    more_main=0;
	return 0;
}

THREAD_CC daemon_thread2(void *arg)
{ int sclient[128] ;
  static THREAD_TYPE tid;
  int i;
  unsigned short port= 443;
  char host[200];
  int s;
  struct sockaddr_in addr;
  int nsclient=0;
  unsigned int lenaddr;
  
 
#ifndef WIN32
//int pid;
int reuse=1,ret;
pthread_detach(pthread_self(  ));
#endif
    
    //nbsession=0;
	//y=0;

    for (i=0;i<(int)strlen(mysocket);i++)
	{	if (mysocket[i]==(char)':')
	    { sscanf(&mysocket[i+1],"%hu",&port);
	      mysocket[i]= 0 ;
		  strcpy(host,mysocket);
		  mysocket[i]= (char)':'; 
	      break;
	    }
	}

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr (host); //htonl(INADDR_ANY);

    s = (int) socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) 	
	{ 
	perror("Unable to create socket");
	exit(EXIT_FAILURE);
    }

    #ifndef WIN32
	ret= setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(void *)&reuse,sizeof(reuse));
	ret= setsockopt(s, SOL_SOCKET, SO_REUSEPORT,(void *)&reuse,sizeof(reuse));
	ret++;
    #endif

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
	{
	perror("Unable to bind");
	exit(EXIT_FAILURE);
    }

    if (listen(s, 10) < 0) {
	perror("RACS Server Unable to listen");
	exit(EXIT_FAILURE);
    }

    lenaddr = (int)sizeof(addr)    ;
    ctx_server = setup_server_ctx();

  
    while(1)
    {   
        
		gPrintf(0,"RACS server ready on port %s:%u (total #sessions %d)\n",host,port,nbsession);
        sclient[nsclient] = (int)accept(s,(struct sockaddr*)&addr, &lenaddr);
       
		if (sclient[nsclient] < 0) 
		{   gPrintf(0,"RACS Server error, unable to accept client\n");
		    break;
		}

		THREAD_CREATE(tid, (void *)server_thread3,(void*)(&sclient[nsclient]));
        nsclient++;
		nsclient = nsclient & 0x7F;
    }
   
    if (ctx_server  != NULL) SSL_CTX_free(ctx_server);
    EVP_cleanup();

    #ifdef WIN32
	closesocket(s);
    #else
	close(s);
    #endif

	gPrintf(0,"End of RACS Server Thread\n");
    more_main=0;


#ifdef WIN32
ExitThread(0);
#else
pthread_exit(NULL);
#endif
return(0);

}





THREAD_CC daemon_thread2a(void *arg)
{ static SSL         *ssl[128];
  static SSL_CTX     *ctx;
  static THREAD_TYPE tid;
  int i;
  unsigned short port= 443;
  char host[200];
  int s;
  struct sockaddr_in addr;
  int sclient  ;
  int nsclient=0;
  unsigned int lenaddr;
  
 
#ifndef WIN32
//int pid;
int reuse=1,ret;
pthread_detach(pthread_self(  ));
#endif
    
    //nbsession=0;
	//y=0;

    for (i=0;i<(int)strlen(mysocket);i++)
	{	if (mysocket[i]==(char)':')
	    { sscanf(&mysocket[i+1],"%hu",&port);
	      mysocket[i]= 0 ;
		  strcpy(host,mysocket);
		  mysocket[i]= (char)':'; 
	      break;
	    }
	}

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr (host); //htonl(INADDR_ANY);

    s = (int) socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) 	
	{ 
	perror("Unable to create socket");
	exit(EXIT_FAILURE);
    }

    #ifndef WIN32
	ret= setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(void *)&reuse,sizeof(reuse));
	ret= setsockopt(s, SOL_SOCKET, SO_REUSEPORT,(void *)&reuse,sizeof(reuse));
	ret++;
    #endif

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
	{
	perror("Unable to bind");
	exit(EXIT_FAILURE);
    }

    if (listen(s, 10) < 0) {
	perror("RACS Server Unable to listen");
	exit(EXIT_FAILURE);
    }

    lenaddr = (int)sizeof(addr);
    ctx = setup_server_ctx()   ;

  
    while(1)
    {
		gPrintf(0,"RACS server ready on port %s:%u (total #sessions %d)\n",host,port,nbsession);
        sclient = (int)accept(s,(struct sockaddr*)&addr, &lenaddr);
       
		if (sclient < 0) 
		{   gPrintf(0,"RACS Server error, unable to accept client\n");
		    break;
		}

        ssl[nsclient] = SSL_new(ctx) ;
		if (ssl[nsclient] == NULL)
		{ gPrintf(0, "RACS server, error creating SSL context\n");
		  //continue;
		  break;
		}

        SSL_set_fd(ssl[nsclient],sclient);
		
		/* Done in server_thread
        if (SSL_accept(ssl) <= 0) 
		{   gPrintf(0, "SSL server, error creating SSL context\n");
		    SSL_free(ssl);
            continue;            
		}
		*/


		THREAD_CREATE(tid, (void *)server_thread,ssl[nsclient]);
        nsclient++;
		nsclient = nsclient & 0x7F;
    }
   
    if (ctx  != NULL) SSL_CTX_free(ctx);
    EVP_cleanup();

    #ifdef WIN32
	closesocket(s);
    #else
	close(s);
    #endif

	gPrintf(0,"End of RACS Server Thread\n");
    more_main=0;


#ifdef WIN32
ExitThread(0);
#else
pthread_exit(NULL);
#endif
return(0);

}


THREAD_CC daemon_thread(void *arg)
{ static BIO  *client    ;
  static SSL         *ssl[128];
  static SSL_CTX     *ctx;
  static THREAD_TYPE tid;
  long err=0;
  int sock=0,ls=0;
  int sclient=0;
 

#ifndef WIN32
pthread_detach(pthread_self(  ));
#endif
    
 
    //nbsession=0;
	//y=0;
   
   
    ctx = setup_server_ctx();
    
    // BIO_set_nbio_accept() sets the accept socket to blocking mode (the default) if n is 0 
    // or non blocking mode if n is 1.
    
    // a_bio=BIO_new_accept(host_port);
    // s_bio=BIO_new_ssl(ssl_ctx,0); // server
    
	acc = BIO_new_accept(mysocket);

    if (!acc)
	{    gPrintf(0,"RACS server, error creating server socket\n");
	     goto TheEnd;
	}
 
    err= BIO_set_nbio_accept(acc,0);
    err+=1;
	 
    if (BIO_do_accept(acc) <= 0)
	{ gPrintf(0,"RACS server, error binding server socket\n");
	  goto TheEnd;
	}

	   
    //for (;;)
	while(more_main)
    {
		gPrintf(0,"RACS server ready on port %s (total #sessions %d s=%d)\n",mysocket,nbsession,ls);

        if (BIO_do_accept(acc) <= 0)
        {
			gPrintf(0,"RACS server, error accepting connection err=%d\n",errno);
            break;
			// continue;
		}

		// TCP port xxx connexion 


	client = BIO_pop(acc);

    if (!(ssl[sclient] = SSL_new(ctx)))
	{
            gPrintf(0, "RACS server, error creating SSL context\n");
            break;
			//continue;
	}

       SSL_set_bio(ssl[sclient], client, client);
    
       sock=SSL_get_fd(ssl[sclient]);
       //err= BIO_get_fd(ssl[sclient]->wbio, &sock);
       ls=sock;
        
       #ifndef WIN32
       //if (sock > 1023) break;
       #endif


       THREAD_CREATE(tid, (void *)server_thread, ssl[sclient]);
	   sclient++;
	   sclient &= 0x7F;

    }

TheEnd:
    
	if (ctx != NULL)
	{ SSL_CTX_free(ctx); ctx=NULL;}
    if (acc != NULL)
	{ BIO_free(acc); acc=NULL;}

	gPrintf(0,"End of RACS Server Thread\n");
    more_main=0;



#ifdef WIN32
ExitThread(0);
#else
pthread_exit(NULL);
#endif

return(0);

}


