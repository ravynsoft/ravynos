/* Creating and controlling threads (native Windows implementation).
   Copyright (C) 2005-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2005.
   Based on GCC's gthr-win32.h.  */

#include <config.h>

/* Specification.  */
#include "windows-thread.h"

#include <errno.h>
#include <process.h>
#include <stdlib.h>

#include "windows-once.h"
#include "windows-tls.h"

/* The Thread-Local Storage (TLS) key that allows to access each thread's
   'struct glwthread_thread_struct *' pointer.  */
static DWORD self_key = (DWORD)-1;

/* Initializes self_key.  This function must only be called once.  */
static void
do_init_self_key (void)
{
  self_key = TlsAlloc ();
  /* If this fails, we're hosed.  */
  if (self_key == (DWORD)-1)
    abort ();
}

/* Initializes self_key.  */
static void
init_self_key (void)
{
  static glwthread_once_t once = GLWTHREAD_ONCE_INIT;
  glwthread_once (&once, do_init_self_key);
}

/* This structure contains information about a thread.
   It is stored in TLS under key self_key.  */
struct glwthread_thread_struct
{
  /* Fields for managing the handle.  */
  HANDLE volatile handle;
  CRITICAL_SECTION handle_lock;
  /* Fields for managing the exit value.  */
  BOOL volatile detached;
  void * volatile result;
  /* Fields for managing the thread start.  */
  void * (*func) (void *);
  void *arg;
};

/* Return a real HANDLE object for the current thread.  */
static HANDLE
get_current_thread_handle (void)
{
  HANDLE this_handle;

  /* GetCurrentThread() returns a pseudo-handle, i.e. only a symbolic
     identifier, not a real handle.  */
  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
                        GetCurrentProcess (), &this_handle,
                        0, FALSE, DUPLICATE_SAME_ACCESS))
    abort ();
  return this_handle;
}

glwthread_thread_t
glwthread_thread_self (void)
{
  glwthread_thread_t thread;

  if (self_key == (DWORD)-1)
    init_self_key ();
  thread = TlsGetValue (self_key);
  if (thread == NULL)
    {
      /* This happens only in threads that have not been created through
         glthread_create(), such as the main thread.  */
      for (;;)
        {
          thread =
            (struct glwthread_thread_struct *)
            malloc (sizeof (struct glwthread_thread_struct));
          if (thread != NULL)
            break;
          /* Memory allocation failed.  There is not much we can do.  Have to
             busy-loop, waiting for the availability of memory.  */
          Sleep (1);
        }

      thread->handle = get_current_thread_handle ();
      InitializeCriticalSection (&thread->handle_lock);
      thread->detached = FALSE; /* This can lead to a memory leak.  */
      thread->result = NULL; /* just to be deterministic */
      TlsSetValue (self_key, thread);
    }
  return thread;
}

/* The main function of a freshly creating thread.  It's a wrapper around
   the FUNC and ARG arguments passed to glthread_create_func.  */
static unsigned int WINAPI
wrapper_func (void *varg)
{
  struct glwthread_thread_struct *thread =
    (struct glwthread_thread_struct *) varg;

  EnterCriticalSection (&thread->handle_lock);
  /* Create a new handle for the thread only if the parent thread did not yet
     fill in the handle.  */
  if (thread->handle == NULL)
    thread->handle = get_current_thread_handle ();
  LeaveCriticalSection (&thread->handle_lock);

  if (self_key == (DWORD)-1)
    init_self_key ();
  TlsSetValue (self_key, thread);

  /* Run the thread.  Store the exit value if the thread was not terminated
     otherwise.  */
  thread->result = thread->func (thread->arg);

  /* Process the TLS destructors.  */
  glwthread_tls_process_destructors ();

  if (thread->detached)
    {
      /* Clean up the thread, like thrd_join would do.  */
      DeleteCriticalSection (&thread->handle_lock);
      CloseHandle (thread->handle);
      free (thread);
    }

  return 0;
}

int
glwthread_thread_create (glwthread_thread_t *threadp, unsigned int attr,
                         void * (*func) (void *), void *arg)
{
  struct glwthread_thread_struct *thread =
    (struct glwthread_thread_struct *)
    malloc (sizeof (struct glwthread_thread_struct));
  if (thread == NULL)
    return ENOMEM;
  thread->handle = NULL;
  InitializeCriticalSection (&thread->handle_lock);
  thread->detached = (attr & GLWTHREAD_ATTR_DETACHED ? TRUE : FALSE);
  thread->result = NULL; /* just to be deterministic */
  thread->func = func;
  thread->arg = arg;

  {
    unsigned int thread_id;
    HANDLE thread_handle;

    thread_handle = (HANDLE)
      _beginthreadex (NULL, 100000, wrapper_func, thread, 0, &thread_id);
      /* calls CreateThread with the same arguments */
    if (thread_handle == NULL)
      {
        DeleteCriticalSection (&thread->handle_lock);
        free (thread);
        return EAGAIN;
      }

    EnterCriticalSection (&thread->handle_lock);
    if (thread->handle == NULL)
      thread->handle = thread_handle;
    else
      /* thread->handle was already set by the thread itself.  */
      CloseHandle (thread_handle);
    LeaveCriticalSection (&thread->handle_lock);

    *threadp = thread;
    return 0;
  }
}

int
glwthread_thread_join (glwthread_thread_t thread, void **retvalp)
{
  if (thread == NULL)
    return EINVAL;

  if (thread == glwthread_thread_self ())
    return EDEADLK;

  if (thread->detached)
    return EINVAL;

  if (WaitForSingleObject (thread->handle, INFINITE) == WAIT_FAILED)
    return EINVAL;

  if (retvalp != NULL)
    *retvalp = thread->result;

  DeleteCriticalSection (&thread->handle_lock);
  CloseHandle (thread->handle);
  free (thread);

  return 0;
}

int
glwthread_thread_detach (glwthread_thread_t thread)
{
  if (thread == NULL)
    return EINVAL;

  if (thread->detached)
    return EINVAL;

  thread->detached = TRUE;
  return 0;
}

void
glwthread_thread_exit (void *retval)
{
  glwthread_thread_t thread = glwthread_thread_self ();
  thread->result = retval;
  glwthread_tls_process_destructors ();
  _endthreadex (0); /* calls ExitThread (0) */
  abort ();
}
