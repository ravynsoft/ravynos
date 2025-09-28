/* vasprintf and asprintf with out-of-memory checking.
   Copyright (C) 2002-2004, 2006-2023 Free Software Foundation, Inc.

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

#ifndef _XVASPRINTF_H
#define _XVASPRINTF_H

/* This file uses _GL_ATTRIBUTE_FORMAT, _GL_ATTRIBUTE_MALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get va_list.  */
#include <stdarg.h>

/* Get _GL_ATTRIBUTE_SPEC_PRINTF_STANDARD.  */
#include <stdio.h>

/* Get 'free'.  */
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Write formatted output to a string dynamically allocated with malloc(),
   and return it.  Upon [ENOMEM] memory allocation error, call xalloc_die.
   On some other error
     - [EOVERFLOW] resulting string length is > INT_MAX,
     - [EINVAL] invalid format string,
     - [EILSEQ] error during conversion between wide and multibyte characters,
   return NULL.  */
extern char *xasprintf (const char *format, ...)
       _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
       _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_STANDARD, 1, 2));
extern char *xvasprintf (const char *format, va_list args)
       _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
       _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_STANDARD, 1, 0));

#ifdef __cplusplus
}
#endif

#endif /* _XVASPRINTF_H */
