/* Abstract output stream data type.
   Copyright (C) 2006, 2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifndef _OSTREAM_H
#define _OSTREAM_H

#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "moo.h"


/* Describes the scope of a flush operation.  */
typedef enum
{
  /* Flushes buffers in this ostream_t.
     Use this value if you want to write to the underlying ostream_t.  */
  FLUSH_THIS_STREAM = 0,
  /* Flushes all buffers in the current process.
     Use this value if you want to write to the same target through a
     different file descriptor or a FILE stream.  */
  FLUSH_THIS_PROCESS = 1,
  /* Flushes buffers in the current process and attempts to flush the buffers
     in the kernel.
     Use this value so that some other process (or the kernel itself)
     may write to the same target.  */
  FLUSH_ALL = 2
} ostream_flush_scope_t;


/* An output stream is an object to which one can feed a sequence of bytes.  */

struct ostream
{
methods:

  /* Write a sequence of bytes to a stream.  */
  void write_mem (ostream_t stream, const void *data, size_t len);

  /* Bring buffered data to its destination.  */
  void flush (ostream_t stream, ostream_flush_scope_t scope);

  /* Close and free a stream.  */
  void free (ostream_t stream);
};

#ifdef __cplusplus
extern "C" {
#endif

/* Write a string's contents to a stream.  */
extern void ostream_write_str (ostream_t stream, const char *string);

/* Writes formatted output to a stream.
   Returns the size of formatted output, or a negative value in case of an
   error.  */
extern ptrdiff_t ostream_printf (ostream_t stream, const char *format, ...)
#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 1) || __GNUC__ > 3
  __attribute__ ((__format__ (__printf__, 2, 3)))
#endif
  ;
extern ptrdiff_t ostream_vprintf (ostream_t stream,
                                  const char *format, va_list args)
#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 1) || __GNUC__ > 3
  __attribute__ ((__format__ (__printf__, 2, 0)))
#endif
  ;

#if HAVE_INLINE

#define ostream_write_str ostream_write_str_inline
static inline void
ostream_write_str (ostream_t stream, const char *string)
{
  ostream_write_mem (stream, string, strlen (string));
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* _OSTREAM_H */
