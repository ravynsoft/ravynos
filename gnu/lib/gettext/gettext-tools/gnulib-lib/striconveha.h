/* Character set conversion with error handling and autodetection.
   Copyright (C) 2002, 2005, 2007-2023 Free Software Foundation, Inc.
   Written by Bruno Haible.

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

#ifndef _STRICONVEHA_H
#define _STRICONVEHA_H

/* This file uses _GL_ATTRIBUTE_MALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdlib.h>

#include "iconveh.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Convert an entire string from one encoding to another, using iconv.
   The original string is at [SRC,...,SRC+SRCLEN-1].
   The "from" encoding can also be a name defined for autodetection.
   If TRANSLITERATE is true, transliteration will attempted to avoid conversion
   errors, for iconv implementations that support this.  Usually you'll choose
   TRANSLITERATE = true if HANDLER != iconveh_error.
   If OFFSETS is not NULL, it should point to an array of SRCLEN integers; this
   array is filled with offsets into the result, i.e. the character starting
   at SRC[i] corresponds to the character starting at (*RESULTP)[OFFSETS[i]],
   and other offsets are set to (size_t)(-1).
   *RESULTP and *LENGTH should initially be a scratch buffer and its size,
   or *RESULTP can initially be NULL.
   May erase the contents of the memory at *RESULTP.
   Return value: 0 if successful, otherwise -1 and errno set.
   If successful: The resulting string is stored in *RESULTP and its length
   in *LENGTHP.  *RESULTP is set to a freshly allocated memory block, or is
   unchanged if no dynamic memory allocation was necessary.  */
extern int
       mem_iconveha (const char *src, size_t srclen,
                     const char *from_codeset, const char *to_codeset,
                     bool transliterate,
                     enum iconv_ilseq_handler handler,
                     size_t *offsets,
                     char **resultp, size_t *lengthp);

/* Convert an entire string from one encoding to another, using iconv.
   The original string is the NUL-terminated string starting at SRC.
   Both the "from" and the "to" encoding must use a single NUL byte at the
   end of the string (i.e. not UCS-2, UCS-4, UTF-16, UTF-32).
   The "from" encoding can also be a name defined for autodetection.
   If TRANSLITERATE is true, transliteration will attempted to avoid conversion
   errors, for iconv implementations that support this.  Usually you'll choose
   TRANSLITERATE = true if HANDLER != iconveh_error.
   Allocate a malloced memory block for the result.
   Return value: the freshly allocated resulting NUL-terminated string if
   successful, otherwise NULL and errno set.  */
extern char *
       str_iconveha (const char *src,
                     const char *from_codeset, const char *to_codeset,
                     bool transliterate,
                     enum iconv_ilseq_handler handler)
       _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;


/* In the above, FROM_CODESET can also be one of the following values:
      "autodetect_utf8"         supports ISO-8859-1 and UTF-8
      "autodetect_jp"           supports EUC-JP, ISO-2022-JP-2 and SHIFT_JIS
      "autodetect_kr"           supports EUC-KR and ISO-2022-KR
   More names can be defined for autodetection.  */

/* Registers an encoding name for autodetection.
   TRY_IN_ORDER is a NULL terminated list of encodings to be tried.
   Returns 0 upon success, or -1 (with errno set) in case of error.
   Particular errno values: ENOMEM.  */
extern int
       uniconv_register_autodetect (const char *name,
                                    const char * const *try_in_order);


#ifdef __cplusplus
}
#endif


#endif /* _STRICONVEHA_H */
