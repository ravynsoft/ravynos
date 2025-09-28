/* Yielding the processor to other threads and processes.
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

/* This file contains a primitive for yielding the processor to other threads.
     extern void gl_thread_yield (void);
 */

#ifndef _GLTHREAD_YIELD_H
#define _GLTHREAD_YIELD_H

#include <errno.h>

/* ========================================================================= */

#if USE_ISOC_THREADS || USE_ISOC_AND_POSIX_THREADS

/* Use the ISO C threads library.  */

# include <threads.h>

# ifdef __cplusplus
extern "C" {
# endif

# define gl_thread_yield() \
    thrd_yield ()

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if USE_POSIX_THREADS

/* Use the POSIX threads library.  */

# include <sched.h>

# ifdef __cplusplus
extern "C" {
# endif

# define gl_thread_yield() \
    sched_yield ()

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if USE_WINDOWS_THREADS

# define WIN32_LEAN_AND_MEAN  /* avoid including junk */
# include <windows.h>

# ifdef __cplusplus
extern "C" {
# endif

# define gl_thread_yield() \
    Sleep (0)

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if !(USE_ISOC_THREADS || USE_POSIX_THREADS || USE_ISOC_AND_POSIX_THREADS || USE_WINDOWS_THREADS)

/* Provide dummy implementation if threads are not supported.  */

# define gl_thread_yield() 0

#endif

/* ========================================================================= */

#endif /* _GLTHREAD_YIELD_H */
