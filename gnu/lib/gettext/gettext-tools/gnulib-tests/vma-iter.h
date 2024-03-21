/* Iteration over virtual memory areas.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2011.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _VMA_ITER_H
#define _VMA_ITER_H

/* This file uses HAVE_PSTAT_GETPROCVM, HAVE_MQUERY.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Bit mask for the FLAGS parameter of a vma_iterate callback function.  */
#define VMA_PROT_READ    (1<<0)
#define VMA_PROT_WRITE   (1<<1)
#define VMA_PROT_EXECUTE (1<<2)

typedef int (*vma_iterate_callback_fn) (void *data,
                                        uintptr_t start, uintptr_t end,
                                        unsigned int flags);

/* Iterate over the virtual memory areas of the current process.
   If such iteration is supported, the callback is called once for every
   virtual memory area, in ascending order, with the following arguments:
     - DATA is the same argument as passed to vma_iterate.
     - START is the address of the first byte in the area, page-aligned.
     - END is the address of the last byte in the area plus 1, page-aligned.
       Note that it may be 0 for the last area in the address space.
     - FLAGS is a combination of the VMA_* bits.
   If the callback returns 0, the iteration continues.  If it returns 1,
   the iteration terminates prematurely.
   This function may open file descriptors, but does not call malloc().
   Return 0 if all went well, or -1 in case of error.  */
extern int vma_iterate (vma_iterate_callback_fn callback, void *data);

/* The macro VMA_ITERATE_SUPPORTED indicates that vma_iterate is supported on
   this platform.
   Note that even when this macro is defined, vma_iterate() may still fail to
   find any virtual memory area, for example if /proc is not mounted.  */
#if defined __linux__ || defined __ANDROID__ || defined __GNU__ || defined __FreeBSD_kernel__ || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined _AIX || defined __sgi || defined __osf__ || defined __sun || HAVE_PSTAT_GETPROCVM || (defined __APPLE__ && defined __MACH__) || defined _WIN32 || defined __CYGWIN__ || defined __BEOS__ || defined __HAIKU__ || defined __minix || HAVE_MQUERY
# define VMA_ITERATE_SUPPORTED 1
#endif


#ifdef __cplusplus
}
#endif

#endif /* _VMA_ITER_H */
