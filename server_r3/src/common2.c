/* common2.c */
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

#include "common2.h"
#include <time.h>

extern int THREAD_setup(void);
extern int gPrintf(int id,char *fmt, ...);

int cache_nb_digits=3;
int cache_enable=1   ;
int cache_timeout=300;
int cache_for_ever=0;
char tlscache[128]= {"./" };



EC_KEY * f1ecc(SSL *ssl,int is_export,int keylength)
{  int      nid;
   EC_KEY  *ecdh;

    /*
     * Elliptic-Curve Diffie-Hellman parameters are either "named curves"
     * from RFC 4492 section 5.1.1, or explicitly described curves over
     * binary fields. OpenSSL only supports the "named curves", which provide
     * maximum interoperability.
     */

    nid = OBJ_sn2nid("sect193r1");
    if (nid == 0)
		return NULL;

    ecdh = EC_KEY_new_by_curve_name(nid);
    if (ecdh == NULL) 
	return NULL ;

	return ecdh;
}


void makename(SSL_SESSION * s, char *name)
{ int i;
  char fname[512];

  fname[0]=0;
  name[0]=0;

  if (s== NULL) return;

  sprintf(name,"%s_id_",tlscache);

  for(i=0;i<(int)s->session_id_length;i++)
  sprintf(&fname[strlen(fname)],"%2.2X",0xFF & s->session_id[i]);

  i=0;
  if ((int)strlen(fname) > cache_nb_digits)
  i=  (int)strlen(fname)- cache_nb_digits;

  strcat(name,&fname[i]);

   
  sprintf(&name[strlen(name)],".pem");
}

void makename2(char *id, int len, char *name)
{ int i;
  char fname[512];

  fname[0]=0;
  name[0]=0;

  sprintf(name,"%s_id_",tlscache);
  
  for(i=0;i<len;i++)
  sprintf(&fname[strlen(fname)],"%2.2X",0xFF & id[i]);

  i=0;
  if ((int)strlen(fname) > cache_nb_digits)
  i=  (int)strlen(fname)-cache_nb_digits;

  strcat(name,&fname[i]);

  sprintf(&name[strlen(name)],".pem");
}


void printsession(SSL_SESSION * s)
{  int i;
   struct tm *newtime;
   time_t t;
   
 
    if (s == NULL) 
		
	{ gPrintf(0,"NULL\n");return;}

	gPrintf(0,"MasterSecret_Length: %d\n",s->master_key_length);
	for (i=0;i<(int)s->master_key_length;i++)gPrintf(0,"%2.2X",0xFF & s->master_key[i]);
	gPrintf(0,"\n");
	gPrintf(0,"session_id_Length: %d\n",s->session_id_length);
	for (i=0;i<(int)s->session_id_length;i++)gPrintf(0,"%2.2X",0xFF & s->session_id[i]);
	gPrintf(0,"\n");
	gPrintf(0,"Timeout: %d\n",s->timeout);

	/* Obtain coordinated universal time: */

     t=(time_t) s->time ;

     newtime = localtime(&t);

     gPrintf(0, "time: %s\n", asctime( newtime ) );





}
//acquire a lock for the file named according to the session id
//open file named according to session id
//encrypt and write the SSL_SESSION object to the file
//release the lock

// Serveur, écriture de la session dans le cache


// If the callback returns 0, the session will be immediately removed again.


/////////////////Ecriture de la session dans le cache//////////////////////////////////////////
/////////////////La session TLS est deja ouverte///////////////////////////////////////////////

int new_session_cb(SSL * ssl, SSL_SESSION * session)
{   char name[512]="cache.pem" ;
     FILE *f=NULL;

     if (cache_enable != 1)
       return 1 ;
    
	makename(session,name);
    gPrintf(0,"Writting session in cache: %s\n",name);
    f= fopen(name,"w+b");
	
	if (f != NULL)
    {  // printsession(session);
	  session->timeout = cache_timeout ; //300 ;// * 3600 ; 5 minutes
	  PEM_write_SSL_SESSION(f,session) ;
      fclose(f);
	}

return 1;
}
int new_session_cb2(SSL * ssl, SSL_SESSION * session)
{   char name[512]="cache2.pem" ;
     FILE *f=NULL;
    
	 //makename(session,name);
	
    gPrintf(0,"new_session: %s\n",name);
    f= fopen(name,"w+b");
	
	if (f != NULL)
    { // printsession(session);
	  // session->timeout = 300 * 3600 ;

	  PEM_write_SSL_SESSION(f,session);
      fclose(f);
	}

return 1;
}



//  acquire a lock for the file named according to the session id
//  remove the file named according to the session id
//  release the lock

// The remove_session_cb() is called, whenever the SSL engine removes a session from the internal cache. 
// This happens when the session is removed because it is expired or when a connection was not shutdown cleanly. 
// It also happens for all sessions in the internal session cache when SSL_CTX_free is called.

/////////Fin de la session TLS, la Session en cache expire (timeout) ou incident//////


void remove_session_cb(SSL_CTX * ssl, SSL_SESSION * session)
{   char name[512]="cache.pem" ; 
    //FILE *f;

    if (cache_enable != 1)
       return ;

	//!!!!!!!!!!!!!!!!!!!Always Called!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	makename(session,name);
    gPrintf(0,"Removing session (no action) in cache: %s\n",name);

	//f= fopen(name,"w+b");
	//if (f != NULL)  fclose(f);
		  
	// printsession(session);
    
	return ;
}

//  acquire a lock for the file named according to the session id in the 2nd arg
//  read and decrypt contents of file and create a new SSL_SESSION object
//  release the lock
//  set the integer referenced by the fourth parameter to 0
//  return the new session object

//===============================================
// Serveur: récupère une session_id dans le cache
//===============================================

// The get_session_cb() is only called on SSL/TLS servers with the session id proposed by the client. 
// The get_session_cb() is always called, also when session caching was disabled. 
// The get_session_cb() is passed the ssl connection, the session id of length length at the memory location data. 
// With the parameter copy the callback can require the SSL engine to increment the reference count of the SSL_SESSION object,
// Normally the reference count is not incremented and therefore the session must not be explicitly freed with SSL_SESSION_free.

//////////////////////////////////APPEL au Cache////////////////////////////////

SSL_SESSION *get_session_cb(SSL *ssl, unsigned char *id, int lenid, int *copy  )
{ 
FILE *f=NULL;
SSL_SESSION * session=NULL;
char name[512]="cache.pem" ; 
time_t t,dt;
   
makename2(id,lenid,name);

//gPrintf(0,"get_session: %s\n",name);

*copy = 0;

if (cache_enable != 1)
return NULL;

    f= fopen(name,"r+b");

	if (f == NULL) 
	{ gPrintf(0,"session# %s not found in cache...\n",name);
	  return NULL;
	}

	// If the file if empty the return value is NULL
	 session = PEM_read_SSL_SESSION(f,NULL,NULL,NULL);

	 if (session == NULL)
	 { gPrintf(0,"session# %s not found in cache (NULL)...\n",name);
	   fclose(f);
	   return NULL;
	 }

	 if (lenid == session->session_id_length)
	 { if (memcmp(id,session->session_id,lenid) != 0)
	   {  gPrintf(0,"session# %s not found in cache (wrong id)...\n",name);
		  fclose(f);
	      return NULL;
	   }
	 }
	 else
	 { gPrintf(0,"session# %s not found in cache (wrong id)...\n",name);
	   fclose(f);
	   return NULL;
	 }

   	// Pour cache infini
	if (cache_for_ever == 1)
	{ time(&t);
	  session->time= (int)t  ;
	  dt=0;
	}
	else
	{ time(&t);
	  dt= t-session->time;
	  if (dt >= cache_timeout)
	  { gPrintf(0,"session# %s found in cache is out of date (%d)...\n", name,dt);
        fclose(f);
	    return NULL;
	  }

	}


    if (cache_for_ever == 1)
	gPrintf(0,"session# %s found in cache for ever...\n", name);
	else
	gPrintf(0,"session# %s found in cache (age %d)...\n",name,dt);
    
	fclose(f);


	return(session);

}

void handle_error(const char *file, int lineno, const char *msg)
{
    gPrintf(0,"** %s:%i %s\n", file, lineno, msg);
    //ERR_print_errors_fp(stderr);

	#ifdef WIN32  
	    ExitThread(0);
	    
    #else 
    exit(-1);
    #endif
}
    


int init_OpenSSL(void)
{ int err;

  err= THREAD_setup();

    if ( (err == 0) || !SSL_library_init())
	{
        fprintf(stderr, "** OpenSSL initialization failed!\n");
        return -1 ;
    }

    SSL_load_error_strings();
    return 0;
}




int verify_callback(int ok, X509_STORE_CTX *store)
{
    char data[256];
 
    if (!ok)
    {
        X509 *cert = X509_STORE_CTX_get_current_cert(store);
        int  depth = X509_STORE_CTX_get_error_depth(store);
        int  err = X509_STORE_CTX_get_error(store);
 
        gPrintf(0,"-Error with certificate at depth: %i\n", depth);
        X509_NAME_oneline(X509_get_issuer_name(cert), data, 256);
        gPrintf(0,"  issuer   = %s\n", data);
        X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
        gPrintf(0,"  subject  = %s\n", data);
        gPrintf(0,"  err %i:%s\n", err, X509_verify_cert_error_string(err));
    }
 
    return ok;
}

long post_connection_check(SSL *ssl, char *host)
{
    X509      *cert;
    X509_NAME *subj;
    char      data[256];
    int       extcount;
    int       ok = 0;
 
    /* Checking the return from SSL_get_peer_certificate here is not strictly
     * necessary.  With our example programs, it is not possible for it to return
     * NULL.  However, it is good form to check the return since it can return NULL
     * if the examples are modified to enable anonymous ciphers or for the server
     * to not require a client certificate.
     */
    if (!(cert = SSL_get_peer_certificate(ssl)) || !host)
        goto err_occured;
    if ((extcount = X509_get_ext_count(cert)) > 0)
    {
        int i;
 
        for (i = 0;  i < extcount;  i++)
        {
            char              *extstr;
            X509_EXTENSION    *ext;
 
            ext = X509_get_ext(cert, i);
            extstr = (char*) OBJ_nid2sn(OBJ_obj2nid(X509_EXTENSION_get_object(ext)));
 
            if (!strcmp(extstr, "subjectAltName"))
            {
                int                  j;
                unsigned char        *data;
                STACK_OF(CONF_VALUE) *val;
                CONF_VALUE           *nval;
                const X509V3_EXT_METHOD    *meth;
                void                 *ext_str = NULL;
 
                if (!(meth = X509V3_EXT_get(ext)))
                    break;
                data = ext->value->data;

#if (OPENSSL_VERSION_NUMBER > 0x00907000L)
                if (meth->it)
                  ext_str = ASN1_item_d2i(NULL, &data, ext->value->length,
                                          ASN1_ITEM_ptr(meth->it));
                else
                  ext_str = meth->d2i(NULL, &data, ext->value->length);
#else
                ext_str = meth->d2i(NULL, &data, ext->value->length);
#endif
                val = meth->i2v(meth, ext_str, NULL);
                for (j = 0;  j < sk_CONF_VALUE_num(val);  j++)
                {
                    nval = sk_CONF_VALUE_value(val, j);
                    if (!strcmp(nval->name, "DNS") && !strcmp(nval->value, host))
                    {
                        ok = 1;
                        break;
                    }
                }
            }
            if (ok)
                break;
        }
    }
 
    if (!ok && (subj = X509_get_subject_name(cert)) &&
        X509_NAME_get_text_by_NID(subj, NID_commonName, data, 256) > 0)
    {
        data[255] = 0;
        if (strcmp(data, host) != 0)
            goto err_occured;
    }
 
    X509_free(cert);
    return SSL_get_verify_result(ssl);
 
err_occured:
    if (cert)
        X509_free(cert);
    return X509_V_ERR_APPLICATION_VERIFICATION;
}

void seed_prng(void)
{
  RAND_load_file("urandom", 1024);
}
