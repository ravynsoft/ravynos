/* Error handling during reading and writing of PO files.
   Copyright (C) 2004, 2006, 2012 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2004.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _PO_ERROR_H
#define _PO_ERROR_H

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || __STRICT_ANSI__
#  define __attribute__(Spec) /* empty */
# endif
/* The __-protected variants of 'format' and 'printf' attributes
   are accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#  define __format__ format
#  define __printf__ printf
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif 


/* Both functions must work like the GNU error(), error_at_line() functions.
   In particular,
     - The functions must not return if the status code is nonzero.
     - The functions must increment the error_message_count variable declared
       in error.h.  */

extern DLL_VARIABLE
       void (*po_error) (int status, int errnum,
                         const char *format, ...)
#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 1) || __GNUC__ > 3
       __attribute__ ((__format__ (__printf__, 3, 4)))
#endif
       ;
extern DLL_VARIABLE
       void (*po_error_at_line) (int status, int errnum,
                                 const char *filename, unsigned int lineno,
                                 const char *format, ...)
#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 1) || __GNUC__ > 3
       __attribute__ ((__format__ (__printf__, 5, 6)))
#endif
       ;

/* Both functions must work like the xerror.h multiline_warning(),
   multiline_error() functions.  In particular,
     - multiline_error must increment the error_message_count variable declared
       in error.h if prefix != NULL.  */

extern DLL_VARIABLE
       void (*po_multiline_warning) (char *prefix, char *message);
extern DLL_VARIABLE
       void (*po_multiline_error) (char *prefix, char *message);


#ifdef __cplusplus
}
#endif

#endif /* _PO_ERROR_H */
