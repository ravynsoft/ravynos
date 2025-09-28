/* Test of vasnprintf() and asnprintf() functions.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include "vasnprintf.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"

static void
test_function (char * (*my_asnprintf) (char *, size_t *, const char *, ...))
{
  char buf[8];
  int size;

  for (size = 0; size <= 8; size++)
    {
      size_t length = size;
      char *result = my_asnprintf (NULL, &length, "%d", 12345);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, "12345") == 0);
      ASSERT (length == 5);
      free (result);
    }

  for (size = 0; size <= 8; size++)
    {
      size_t length;
      char *result;

      memcpy (buf, "DEADBEEF", 8);
      length = size;
      result = my_asnprintf (buf, &length, "%d", 12345);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, "12345") == 0);
      ASSERT (length == 5);
      if (size < 5 + 1)
        ASSERT (result != buf);
      ASSERT (memcmp (buf + size, &"DEADBEEF"[size], 8 - size) == 0);
      if (result != buf)
        free (result);
    }

  /* Note: This test assumes IEEE 754 representation of 'double' floats.  */
  for (size = 0; size <= 8; size++)
    {
      size_t length;
      char *result;

      memcpy (buf, "DEADBEEF", 8);
      length = size;
      result = my_asnprintf (buf, &length, "%2.0f", 1.6314159265358979e+125);
      ASSERT (result != NULL);
      /* The exact result and the result on glibc systems is
         163141592653589790215729350939528493057529598899734151772468186268423257777068536614838678161083520756952076273094236944990208
         On Cygwin, the result is
         163141592653589790215729350939528493057529600000000000000000000000000000000000000000000000000000000000000000000000000000000000
         On HP-UX 11.31 / hppa and IRIX 6.5, the result is
         163141592653589790000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
       */
      ASSERT (strlen (result) == 126);
      ASSERT (memcmp (result, "163141592653589790", 18) == 0);
      ASSERT (length == 126);
      if (size < 126 + 1)
        ASSERT (result != buf);
      ASSERT (memcmp (buf + size, &"DEADBEEF"[size], 8 - size) == 0);
      if (result != buf)
        free (result);
    }
}

static char *
my_asnprintf (char *resultbuf, size_t *lengthp, const char *format, ...)
{
  va_list args;
  char *ret;

  va_start (args, format);
  ret = vasnprintf (resultbuf, lengthp, format, args);
  va_end (args);
  return ret;
}

static void
test_vasnprintf ()
{
  test_function (my_asnprintf);
}

static void
test_asnprintf ()
{
  test_function (asnprintf);
}

int
main ()
{
  test_vasnprintf ();
  test_asnprintf ();
  return 0;
}
