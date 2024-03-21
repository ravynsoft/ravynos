/* imalloc.h -- internal malloc definitions shared by source files. */

/* Copyright (C) 2001-2020 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Must be included *after* config.h */

#ifndef _IMALLOC_H
#define _IMALLOC_H

#ifdef MALLOC_DEBUG
#define MALLOC_STATS
#define MALLOC_TRACE
#define MALLOC_REGISTER
#define MALLOC_WATCH
#endif

#define MALLOC_WRAPFUNCS

/* If defined, as it is by default, use the lesscore() function to attempt
   to reduce the top of the heap when freeing memory blocks larger than a
   defined threshold. */
#define USE_LESSCORE

/* Generic pointer type. */
#ifndef PTR_T
#  if defined (__STDC__)
#    define PTR_T void *
#  else
#    define PTR_T char *
#  endif
#endif

#if !defined (NULL)
#  define NULL 0
#endif

#if !defined (CPP_STRING)
#  if defined (HAVE_STRINGIZE)
#    define CPP_STRING(x) #x
#  else
#    define CPP_STRING(x) "x"
#  endif /* !HAVE_STRINGIZE */
#endif /* !__STRING */

#if __GNUC__ > 1
#  define FASTCOPY(s, d, n)  __builtin_memcpy (d, s, n)
#else /* !__GNUC__ */
#  if !defined (HAVE_BCOPY)
#    if !defined (HAVE_MEMMOVE)
#      define FASTCOPY(s, d, n)  memcpy (d, s, n)
#    else
#      define FASTCOPY(s, d, n)  memmove (d, s, n)
#    endif /* !HAVE_MEMMOVE */
#  else /* HAVE_BCOPY */
#    define FASTCOPY(s, d, n)  bcopy (s, d, n)
#  endif /* HAVE_BCOPY */
#endif /* !__GNUC__ */

#if !defined (PARAMS)
#  if defined (__STDC__) || defined (__GNUC__) || defined (__cplusplus) || defined (PROTOTYPES)
#    define PARAMS(protos) protos
#  else 
#    define PARAMS(protos) ()
#  endif
#endif

/* Use Duff's device for good zeroing/copying performance.  DO NOT call the
   Duff's device macros with NBYTES == 0. */

#define MALLOC_BZERO(charp, nbytes)					\
do {									\
  if ((nbytes) <= 32) {							\
    size_t * mzp = (size_t *)(charp);					\
    unsigned long mctmp = (nbytes)/sizeof(size_t);			\
    long mcn;								\
    if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp &= 7; }	\
    switch (mctmp) {							\
      case 0: for(;;) { *mzp++ = 0;					\
      case 7:	   *mzp++ = 0;						\
      case 6:	   *mzp++ = 0;						\
      case 5:	   *mzp++ = 0;						\
      case 4:	   *mzp++ = 0;						\
      case 3:	   *mzp++ = 0;						\
      case 2:	   *mzp++ = 0;						\
      case 1:	   *mzp++ = 0; if(mcn <= 0) break; mcn--; }		\
    }									\
  else									\
    memset ((charp), 0, (nbytes));					\
} while(0)

#define MALLOC_ZERO(charp, nbytes) \
do { 								\
  size_t mzsz = (nbytes);					\
  if (mzsz <= 9 * sizeof(mzsz) {				\
    size_t *mz = (size_t *)(charp);				\
    if(mzsz >= 5*sizeof(mzsz)) {	*mz++ = 0;		\
					*mz++ = 0;		\
      if(mzsz >= 7*sizeof(mzsz)) {	*mz++ = 0;		\
					*mz++ = 0;		\
	if(mzsz >= 9*sizeof(mzsz)) {	*mz++ = 0;		\
					*mz++ = 0; }}}		\
					*mz++ = 0;		\
					*mz++ = 0;		\
					*mz = 0;		\
  } else							\
    memset ((charp), 0, mzsz);					\
} while (0)

#define MALLOC_MEMSET(charp, xch, nbytes)				\
do {									\
  if ((nbytes) <= 32) {							\
    register char * mzp = (charp);					\
    unsigned long mctmp = (nbytes);					\
    register long mcn;							\
    if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp &= 7; }	\
    switch (mctmp) {							\
      case 0: for(;;) { *mzp++ = xch;					\
      case 7:	   *mzp++ = xch;					\
      case 6:	   *mzp++ = xch;					\
      case 5:	   *mzp++ = xch;					\
      case 4:	   *mzp++ = xch;					\
      case 3:	   *mzp++ = xch;					\
      case 2:	   *mzp++ = xch;					\
      case 1:	   *mzp++ = xch; if(mcn <= 0) break; mcn--; }		\
    }									\
  } else								\
    memset ((charp), (xch), (nbytes));					\
} while(0)

#define MALLOC_MEMCPY(dest,src,nbytes)					\
do {									\
  if ((nbytes) <= 32) {							\
    size_t* mcsrc = (size_t*) src;					\
    size_t* mcdst = (size_t*) dest;					\
    unsigned long mctmp = (nbytes)/sizeof(size_t);			\
    long mcn;								\
    if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp &= 7; }	\
    switch (mctmp) {							\
      case 0: for(;;) { *mcdst++ = *mcsrc++;				\
      case 7:	   *mcdst++ = *mcsrc++;					\
      case 6:	   *mcdst++ = *mcsrc++;					\
      case 5:	   *mcdst++ = *mcsrc++;					\
      case 4:	   *mcdst++ = *mcsrc++;					\
      case 3:	   *mcdst++ = *mcsrc++;					\
      case 2:	   *mcdst++ = *mcsrc++;					\
      case 1:	   *mcdst++ = *mcsrc++; if(mcn <= 0) break; mcn--; }	\
  } else								\
    memcpy ((dest), (src), (nbytes))					\
} while(0)

#if defined (SHELL)
#  include "bashintl.h"
#else
#  define _(x)	x
#endif

#include <signal.h>

extern void _malloc_block_signals PARAMS((sigset_t *, sigset_t *));
extern void _malloc_unblock_signals PARAMS((sigset_t *, sigset_t *));

#endif /* _IMALLOC_H */
