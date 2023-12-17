/* Creating and controlling threads.
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
   Based on GCC's gthr-posix.h, gthr-posix95.h, gthr-win32.h.  */

/* This file contains primitives for creating and controlling threads.

   Thread data type: gl_thread_t.

   Creating a thread:
       thread = gl_thread_create (func, arg);
   Or with control of error handling:
       err = glthread_create (&thread, func, arg);
       extern int glthread_create (gl_thread_t *result,
                                   void *(*func) (void *), void *arg);

   Querying and changing the signal mask of a thread (not supported on all
   platforms):
       gl_thread_sigmask (how, newmask, oldmask);
   Or with control of error handling:
       err = glthread_sigmask (how, newmask, oldmask);
       extern int glthread_sigmask (int how, const sigset_t *newmask, sigset_t *oldmask);

   Waiting for termination of another thread:
       gl_thread_join (thread, &return_value);
   Or with control of error handling:
       err = glthread_join (thread, &return_value);
       extern int glthread_join (gl_thread_t thread, void **return_value_ptr);

   Getting a reference to the current thread:
       current = gl_thread_self ();
       extern gl_thread_t gl_thread_self (void);

   Getting a reference to the current thread as a pointer, for debugging:
       ptr = gl_thread_self_pointer ();
       extern void * gl_thread_self_pointer (void);

   Terminating the current thread:
       gl_thread_exit (return_value);
       extern _Noreturn void gl_thread_exit (void *return_value);

   Requesting custom code to be executed at fork() time (not supported on all
   platforms):
       gl_thread_atfork (prepare_func, parent_func, child_func);
   Or with control of error handling:
       err = glthread_atfork (prepare_func, parent_func, child_func);
       extern int glthread_atfork (void (*prepare_func) (void),
                                   void (*parent_func) (void),
                                   void (*child_func) (void));
   Note that even on platforms where this is supported, use of fork() and
   threads together is problematic, see
     <https://lists.gnu.org/r/bug-gnulib/2008-08/msg00062.html>
 */


#ifndef _GLTHREAD_THREAD_H
#define _GLTHREAD_THREAD_H

/* This file uses _Noreturn, HAVE_THREADS_H, HAVE_PTHREAD_ATFORK.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <errno.h>
#include <stdlib.h>

#if !defined c11_threads_in_use
# if HAVE_THREADS_H && USE_POSIX_THREADS_FROM_LIBC
#  define c11_threads_in_use() 1
# elif HAVE_THREADS_H && USE_POSIX_THREADS_WEAK
#  include <threads.h>
#  pragma weak thrd_exit
#  define c11_threads_in_use() (thrd_exit != NULL)
# else
#  define c11_threads_in_use() 0
# endif
#endif

/* ========================================================================= */

#if USE_ISOC_THREADS

/* Use the ISO C threads library.  */

# include <threads.h>

# ifdef __cplusplus
extern "C" {
# endif

/* -------------------------- gl_thread_t datatype -------------------------- */

typedef struct thrd_with_exitvalue *gl_thread_t;
extern int glthread_create (gl_thread_t *threadp,
                            void *(*func) (void *), void *arg);
# define glthread_sigmask(HOW, SET, OSET) \
    pthread_sigmask (HOW, SET, OSET)
extern int glthread_join (gl_thread_t thread, void **return_value_ptr);
extern gl_thread_t gl_thread_self (void);
# define gl_thread_self_pointer() \
    (void *) gl_thread_self ()
extern _Noreturn void gl_thread_exit (void *return_value);
# define glthread_atfork(PREPARE_FUNC, PARENT_FUNC, CHILD_FUNC) 0

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if USE_POSIX_THREADS || USE_ISOC_AND_POSIX_THREADS

/* Use the POSIX threads library.  */

# include <pthread.h>

/* Get intptr_t.  */
# include <stdint.h>

/* On IRIX, pthread_atfork is declared in <unistd.h>, not in <pthread.h>.  */
# if defined __sgi
#  include <unistd.h>
# endif

# if USE_POSIX_THREADS_WEAK
/* Compilers other than GCC need to see the declaration of pthread_sigmask
   before the "#pragma weak pthread_sigmask" below.  */
#  include <signal.h>
# endif

# ifdef __cplusplus
extern "C" {
# endif

# if PTHREAD_IN_USE_DETECTION_HARD

/* The pthread_in_use() detection needs to be done at runtime.  */
#  define pthread_in_use() \
     glthread_in_use ()
extern int glthread_in_use (void);

# endif

# if USE_POSIX_THREADS_WEAK

/* Use weak references to the POSIX threads library.  */

/* Weak references avoid dragging in external libraries if the other parts
   of the program don't use them.  Here we use them, because we don't want
   every program that uses libintl to depend on libpthread.  This assumes
   that libpthread would not be loaded after libintl; i.e. if libintl is
   loaded first, by an executable that does not depend on libpthread, and
   then a module is dynamically loaded that depends on libpthread, libintl
   will not be multithread-safe.  */

/* The way to test at runtime whether libpthread is present is to test
   whether a function pointer's value, such as &pthread_mutex_init, is
   non-NULL.  However, some versions of GCC have a bug through which, in
   PIC mode, &foo != NULL always evaluates to true if there is a direct
   call to foo(...) in the same function.  To avoid this, we test the
   address of a function in libpthread that we don't use.  */

#  ifndef pthread_sigmask /* Do not declare rpl_pthread_sigmask weak.  */
#   pragma weak pthread_sigmask
#  endif

#  pragma weak pthread_join
#  ifndef pthread_self
#   pragma weak pthread_self
#  endif
#  pragma weak pthread_exit
#  if HAVE_PTHREAD_ATFORK
#   pragma weak pthread_atfork
#  endif

#  if !PTHREAD_IN_USE_DETECTION_HARD
#   pragma weak pthread_mutexattr_gettype
#   define pthread_in_use() \
      (pthread_mutexattr_gettype != NULL || c11_threads_in_use ())
#  endif

# else

#  if !PTHREAD_IN_USE_DETECTION_HARD
#   define pthread_in_use() 1
#  endif

# endif

/* -------------------------- gl_thread_t datatype -------------------------- */

/* This choice of gl_thread_t assumes that
     pthread_equal (a, b)  is equivalent to  ((a) == (b)).
   This is the case on all platforms in use in 2008.  */
typedef pthread_t gl_thread_t;
# define glthread_create(THREADP, FUNC, ARG) \
    (pthread_in_use () ? pthread_create (THREADP, NULL, FUNC, ARG) : ENOSYS)
# define glthread_sigmask(HOW, SET, OSET) \
    (pthread_in_use () ? pthread_sigmask (HOW, SET, OSET) : 0)
# define glthread_join(THREAD, RETVALP) \
    (pthread_in_use () ? pthread_join (THREAD, RETVALP) : 0)
# ifdef PTW32_VERSION
   /* In pthreads-win32, pthread_t is a struct with a pointer field 'p' and
      other fields.  */
#  define gl_thread_self() \
     (pthread_in_use () ? pthread_self () : gl_null_thread)
#  define gl_thread_self_pointer() \
     (pthread_in_use () ? pthread_self ().p : NULL)
extern const gl_thread_t gl_null_thread;
# elif defined __MVS__
   /* On IBM z/OS, pthread_t is a struct with an 8-byte '__' field.
      The first three bytes of this field appear to uniquely identify a
      pthread_t, though not necessarily representing a pointer.  */
#  define gl_thread_self() \
     (pthread_in_use () ? pthread_self () : gl_null_thread)
#  define gl_thread_self_pointer() \
     (pthread_in_use () ? *((void **) pthread_self ().__) : NULL)
extern const gl_thread_t gl_null_thread;
# else
#  define gl_thread_self() \
     (pthread_in_use () ? pthread_self () : (pthread_t) 0)
#  define gl_thread_self_pointer() \
     (pthread_in_use () ? (void *) (intptr_t) (pthread_t) pthread_self () : NULL)
# endif
# define gl_thread_exit(RETVAL) \
    (void) (pthread_in_use () ? (pthread_exit (RETVAL), 0) : 0)

# if HAVE_PTHREAD_ATFORK
#  define glthread_atfork(PREPARE_FUNC, PARENT_FUNC, CHILD_FUNC) \
     (pthread_in_use () ? pthread_atfork (PREPARE_FUNC, PARENT_FUNC, CHILD_FUNC) : 0)
# else
#  define glthread_atfork(PREPARE_FUNC, PARENT_FUNC, CHILD_FUNC) 0
# endif

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if USE_WINDOWS_THREADS

# define WIN32_LEAN_AND_MEAN  /* avoid including junk */
# include <windows.h>

# include "windows-thread.h"

# ifdef __cplusplus
extern "C" {
# endif

/* -------------------------- gl_thread_t datatype -------------------------- */

typedef glwthread_thread_t gl_thread_t;
# define glthread_create(THREADP, FUNC, ARG) \
    glwthread_thread_create (THREADP, 0, FUNC, ARG)
# define glthread_sigmask(HOW, SET, OSET) \
    /* unsupported */ 0
# define glthread_join(THREAD, RETVALP) \
    glwthread_thread_join (THREAD, RETVALP)
# define gl_thread_self() \
    glwthread_thread_self ()
# define gl_thread_self_pointer() \
    gl_thread_self ()
# define gl_thread_exit(RETVAL) \
    glwthread_thread_exit (RETVAL)
# define glthread_atfork(PREPARE_FUNC, PARENT_FUNC, CHILD_FUNC) 0

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if !(USE_ISOC_THREADS || USE_POSIX_THREADS || USE_ISOC_AND_POSIX_THREADS || USE_WINDOWS_THREADS)

/* Provide dummy implementation if threads are not supported.  */

typedef int gl_thread_t;
# define glthread_create(THREADP, FUNC, ARG) ENOSYS
# define glthread_sigmask(HOW, SET, OSET) 0
# define glthread_join(THREAD, RETVALP) 0
# define gl_thread_self() 0
# define gl_thread_self_pointer() \
    ((void *) gl_thread_self ())
# define gl_thread_exit(RETVAL) (void)0
# define glthread_atfork(PREPARE_FUNC, PARENT_FUNC, CHILD_FUNC) 0

#endif

/* ========================================================================= */

/* Macros with built-in error handling.  */

#ifdef __cplusplus
extern "C" {
#endif

extern gl_thread_t gl_thread_create (void *(*func) (void *arg), void *arg);
#define gl_thread_sigmask(HOW, SET, OSET)     \
   do                                         \
     {                                        \
       if (glthread_sigmask (HOW, SET, OSET)) \
         abort ();                            \
     }                                        \
   while (0)
#define gl_thread_join(THREAD, RETVAL)     \
   do                                      \
     {                                     \
       if (glthread_join (THREAD, RETVAL)) \
         abort ();                         \
     }                                     \
   while (0)
#define gl_thread_atfork(PREPARE, PARENT, CHILD)     \
   do                                                \
     {                                               \
       if (glthread_atfork (PREPARE, PARENT, CHILD)) \
         abort ();                                   \
     }                                               \
   while (0)

#ifdef __cplusplus
}
#endif

#endif /* _GLTHREAD_THREAD_H */
