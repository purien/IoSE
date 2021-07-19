/* reentrant2.c */
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

#include "reentrant2.h"

#if defined(WIN32_CC)

    #define NBCC 400
    CRITICAL_SECTION CC[NBCC];
    int ptcc[NBCC];

LPCRITICAL_SECTION setupcc()
{ int i;
  for (i=0;i<NBCC;i++)
	  if (ptcc[i] == 0)
	  { ptcc[i]=1000+i;
        InitializeCriticalSection(&CC[i]);
        return &CC[i] ;
	  }

	  return NULL;
}

void cleanupcc(LPCRITICAL_SECTION x)
{ int i;
  for (i=0;i<NBCC;i++)
  { if (x == &CC[i])
    { DeleteCriticalSection(x);
      ptcc[i]=0;
	  return;
    }
  }
}


#endif

/* This array will store all of the mutexes available to OpenSSL. */
static MUTEX_TYPE *mutex_buf = NULL;

static void locking_function(int mode, int n, const char * file, int line)
{
  if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(mutex_buf[n]);
  else
    MUTEX_UNLOCK(mutex_buf[n]);
}

static unsigned long id_function(void)
{
  return ((unsigned long)THREAD_ID);
}

struct CRYPTO_dynlock_value
{
  MUTEX_TYPE mutex;
};

static struct CRYPTO_dynlock_value * dyn_create_function(const char *file, int line)
{
  struct CRYPTO_dynlock_value *value;

  value = (struct CRYPTO_dynlock_value *)malloc(sizeof(struct CRYPTO_dynlock_value));
  if (!value)  return NULL;
  
  MUTEX_SETUP(value->mutex);
  return value;
}

static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l,const char *file, int line)
{
  if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(l->mutex);
  else
    MUTEX_UNLOCK(l->mutex);
}

static void dyn_destroy_function(struct CRYPTO_dynlock_value *l,
				 const char *file, int line)
{
  MUTEX_CLEANUP(l->mutex);
  free(l);
}

MUTEX_TYPE *Pmutex;

int MutexSetup(int nb)
{
  int i;
  Pmutex = (MUTEX_TYPE *)malloc(nb * TYPESIZE);
  if (!Pmutex)
  return 0;
  for (i =0; i < nb ; i++)
  MUTEX_SETUP(Pmutex[i]);
  return 1;
}

int Mutex_cleanup(int nb)
{
  int i;
  if (!Pmutex)
    return 0;
 
  for (i = 0; i < nb; i++)
    MUTEX_CLEANUP(Pmutex[i]);
  
  free(Pmutex);
  mutex_buf = NULL;
  return 1;
}


int THREAD_setup(void)
{
  int i, nb;
   
  nb= CRYPTO_num_locks();

  mutex_buf = (MUTEX_TYPE *)malloc(TYPESIZE * CRYPTO_num_locks() );
  if (!mutex_buf)
    return 0;
  for (i = 0; i < CRYPTO_num_locks( ); i++)
  MUTEX_SETUP(mutex_buf[i]);


  CRYPTO_set_id_callback(id_function);
  CRYPTO_set_locking_callback(locking_function);
    /* The following three CRYPTO_... functions are the OpenSSL functions
       for registering the callbacks we implemented above */
  CRYPTO_set_dynlock_create_callback(dyn_create_function);
  CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
  CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
  return 1;
}

int THREAD_cleanup(void)
{
  int i;
  if (!mutex_buf)
    return 0;
  CRYPTO_set_id_callback(NULL);
  CRYPTO_set_locking_callback(NULL);
  CRYPTO_set_dynlock_create_callback(NULL);
  CRYPTO_set_dynlock_lock_callback(NULL);
  CRYPTO_set_dynlock_destroy_callback(NULL);
  for (i = 0; i < CRYPTO_num_locks( ); i++)
    MUTEX_CLEANUP(mutex_buf[i]);
  free(mutex_buf);
  mutex_buf = NULL;
  return 1;
}
