/* Charset conversion with out-of-memory checking.
   Copyright (C) 2001-2007, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible and Simon Josefsson.

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

#ifndef _XSTRICONVEH_H
#define _XSTRICONVEH_H

/* This file uses _GL_ATTRIBUTE_MALLOC, HAVE_ICONV.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdlib.h>

/* Get the 'enum iconv_ilseq_handler' and iconveh_t types, and the
   iconveh_open, iconveh_close declarations.  */
#include "striconveh.h"


#ifdef __cplusplus
extern "C" {
#endif


#if HAVE_ICONV

/* Convert an entire string from one encoding to another, using iconv.
   The original string is at [SRC,...,SRC+SRCLEN-1].
   CD points to the conversion descriptor from FROMCODE to TOCODE, created by
   the function iconveh_open().
   If OFFSETS is not NULL, it should point to an array of SRCLEN integers; this
   array is filled with offsets into the result, i.e. the character starting
   at SRC[i] corresponds to the character starting at (*RESULTP)[OFFSETS[i]],
   and other offsets are set to (size_t)(-1).
   *RESULTP and *LENGTH should initially be a scratch buffer and its size,
   or *RESULTP can initially be NULL.
   May erase the contents of the memory at *RESULTP.
   Upon memory allocation failure, report the error and exit.
   Return value: 0 if successful, otherwise -1 and errno set.
   If successful: The resulting string is stored in *RESULTP and its length
   in *LENGTHP.  *RESULTP is set to a freshly allocated memory block, or is
   unchanged if no dynamic memory allocation was necessary.  */
extern int
       xmem_cd_iconveh (const char *src, size_t srclen,
                        const iconveh_t *cd,
                        enum iconv_ilseq_handler handler,
                        size_t *offsets,
                        char **resultp, size_t *lengthp);

/* Convert an entire string from one encoding to another, using iconv.
   The original string is the NUL-terminated string starting at SRC.
   CD points to the conversion descriptor from FROMCODE to TOCODE, created by
   the function iconveh_open().
   Both the "from" and the "to" encoding must use a single NUL byte at the end
   of the string (i.e. not UCS-2, UCS-4, UTF-16, UTF-32).
   Allocate a malloced memory block for the result.
   Upon memory allocation failure, report the error and exit.
   Return value: the freshly allocated resulting NUL-terminated string if
   successful, otherwise NULL and errno set.  */
extern char *
       xstr_cd_iconveh (const char *src,
                        const iconveh_t *cd,
                        enum iconv_ilseq_handler handler)
       _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;

#endif

/* Convert an entire string from one encoding to another, using iconv.
   The original string is at [SRC,...,SRC+SRCLEN-1].
   If OFFSETS is not NULL, it should point to an array of SRCLEN integers; this
   array is filled with offsets into the result, i.e. the character starting
   at SRC[i] corresponds to the character starting at (*RESULTP)[OFFSETS[i]],
   and other offsets are set to (size_t)(-1).
   *RESULTP and *LENGTH should initially be a scratch buffer and its size,
   or *RESULTP can initially be NULL.
   May erase the contents of the memory at *RESULTP.
   Upon memory allocation failure, report the error and exit.
   Return value: 0 if successful, otherwise -1 and errno set.
   If successful: The resulting string is stored in *RESULTP and its length
   in *LENGTHP.  *RESULTP is set to a freshly allocated memory block, or is
   unchanged if no dynamic memory allocation was necessary.  */
extern int
       xmem_iconveh (const char *src, size_t srclen,
                     const char *from_codeset, const char *to_codeset,
                     enum iconv_ilseq_handler handler,
                     size_t *offsets,
                     char **resultp, size_t *lengthp);

/* Convert an entire string from one encoding to another, using iconv.
   The original string is the NUL-terminated string starting at SRC.
   Both the "from" and the "to" encoding must use a single NUL byte at the
   end of the string (i.e. not UCS-2, UCS-4, UTF-16, UTF-32).
   Allocate a malloced memory block for the result.
   Upon memory allocation failure, report the error and exit.
   Return value: the freshly allocated resulting NUL-terminated string if
   successful, otherwise NULL and errno set.  */
extern char *
       xstr_iconveh (const char *src,
                     const char *from_codeset, const char *to_codeset,
                     enum iconv_ilseq_handler handler)
       _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;


#ifdef __cplusplus
}
#endif


#endif /* _XSTRICONVEH_H */
