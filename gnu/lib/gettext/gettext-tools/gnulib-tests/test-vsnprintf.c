/* Test of vsnprintf() function.
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

#include <stdio.h>

#include "signature.h"
SIGNATURE_CHECK (vsnprintf, int, (char *, size_t, char const *, va_list));

#include <stdarg.h>
#include <string.h>

#include "macros.h"

static int
my_snprintf (char *buf, int size, const char *format, ...)
{
  va_list args;
  int ret;

  va_start (args, format);
  ret = vsnprintf (buf, size, format, args);
  va_end (args);
  return ret;
}

int
main (int argc, char *argv[])
{
  char buf[8];
  int size;
  int retval;

  retval = my_snprintf (NULL, 0, "%d", 12345);
  ASSERT (retval == 5);

  for (size = 0; size <= 8; size++)
    {
      memcpy (buf, "DEADBEEF", 8);
      retval = my_snprintf (buf, size, "%d", 12345);
      ASSERT (retval == 5);
      if (size < 6)
        {
          if (size > 0)
            {
              ASSERT (memcmp (buf, "12345", size - 1) == 0);
              ASSERT (buf[size - 1] == '\0' || buf[size - 1] == '0' + size);
            }
#if !CHECK_VSNPRINTF_POSIX
          if (size > 0)
#endif
            ASSERT (memcmp (buf + size, &"DEADBEEF"[size], 8 - size) == 0);
        }
      else
        {
          ASSERT (memcmp (buf, "12345\0EF", 8) == 0);
        }
    }

  /* Test the support of the POSIX/XSI format strings with positions.  */
  {
    char result[100];
    retval = my_snprintf (result, sizeof (result), "%2$d %1$d", 33, 55);
    ASSERT (strcmp (result, "55 33") == 0);
    ASSERT (retval == strlen (result));
  }

  return 0;
}
