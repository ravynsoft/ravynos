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

#include <config.h>

/* Specification.  */
#include "ostream.h"

#include <stdio.h>

struct ostream
{
fields:
};

#if !HAVE_INLINE

void
ostream_write_str (ostream_t stream, const char *string)
{
  ostream_write_mem (stream, string, strlen (string));
}

#endif

ptrdiff_t
ostream_printf (ostream_t stream, const char *format, ...)
{
  va_list args;
  char *temp_string;
  ptrdiff_t ret;

  va_start (args, format);
  ret = vasprintf (&temp_string, format, args);
  va_end (args);
  if (ret >= 0)
    {
      if (ret > 0)
        ostream_write_str (stream, temp_string);
      free (temp_string);
    }
  return ret;
}

ptrdiff_t
ostream_vprintf (ostream_t stream, const char *format, va_list args)
{
  char *temp_string;
  ptrdiff_t ret = vasprintf (&temp_string, format, args);
  if (ret >= 0)
    {
      if (ret > 0)
        ostream_write_str (stream, temp_string);
      free (temp_string);
    }
  return ret;
}
