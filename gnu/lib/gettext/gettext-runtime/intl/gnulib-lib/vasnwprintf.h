/* vswprintf with automatic memory allocation.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _VASNWPRINTF_H
#define _VASNWPRINTF_H

/* Get va_list.  */
#include <stdarg.h>

/* Get wchar_t, size_t.  */
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Write formatted output to a string dynamically allocated with malloc().
   You can pass a preallocated buffer for the result in RESULTBUF and its
   size in *LENGTHP; otherwise you pass RESULTBUF = NULL.
   If successful, return the address of the string (this may be = RESULTBUF
   if no dynamic memory allocation was necessary) and set *LENGTHP to the
   number of resulting bytes, excluding the trailing NUL.  Upon error, set
   errno and return NULL.

   When dynamic memory allocation occurs, the preallocated buffer is left
   alone (with possibly modified contents).  This makes it possible to use
   a statically allocated or stack-allocated buffer, like this:

          wchar_t buf[100];
          size_t len = sizeof (buf) / sizeof (wchar_t);
          wchar_t *output = vasnwprintf (buf, &len, format, args);
          if (output == NULL)
            ... error handling ...;
          else
            {
              ... use the output string ...;
              if (output != buf)
                free (output);
            }
  */
extern wchar_t * asnwprintf (wchar_t *resultbuf, size_t *lengthp,
                             const wchar_t *format, ...);
extern wchar_t * vasnwprintf (wchar_t *resultbuf, size_t *lengthp,
                              const wchar_t *format, va_list args);

#ifdef __cplusplus
}
#endif

#endif /* _VASNWPRINTF_H */
