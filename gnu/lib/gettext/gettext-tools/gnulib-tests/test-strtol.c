/*
 * Copyright (C) 2011-2023 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include <stdlib.h>

#include "signature.h"
#ifndef strtol
SIGNATURE_CHECK (strtol, long, (const char *, char **, int));
#endif

#include <errno.h>

#include "macros.h"

int
main (void)
{
  /* Subject sequence empty or invalid.  */
  {
    const char input[] = "";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0);
    ASSERT (ptr == input);
    ASSERT (errno == 0 || errno == EINVAL);
  }
  {
    const char input[] = " ";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0);
    ASSERT (ptr == input);
    ASSERT (errno == 0 || errno == EINVAL);
  }
  {
    const char input[] = " +";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0);
    ASSERT (ptr == input);
    ASSERT (errno == 0 || errno == EINVAL);
  }
  {
    const char input[] = " -";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0);
    ASSERT (ptr == input);
    ASSERT (errno == 0 || errno == EINVAL);
  }

  /* Simple integer values.  */
  {
    const char input[] = "0";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "+0";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0);
    ASSERT (ptr == input + 2);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "-0";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0);
    ASSERT (ptr == input + 2);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "23";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 23);
    ASSERT (ptr == input + 2);
    ASSERT (errno == 0);
  }
  {
    const char input[] = " 23";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 23);
    ASSERT (ptr == input + 3);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "+23";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 23);
    ASSERT (ptr == input + 3);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "-23";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == -23);
    ASSERT (ptr == input + 3);
    ASSERT (errno == 0);
  }

  /* Large integer values.  */
  {
    const char input[] = "2147483647";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 2147483647);
    ASSERT (ptr == input + 10);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "-2147483648";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == -2147483647 - 1);
    ASSERT (ptr == input + 11);
    ASSERT (errno == 0);
  }
  if (sizeof (long) > sizeof (int))
    {
      const char input[] = "4294967295";
      char *ptr;
      long result;
      errno = 0;
      result = strtol (input, &ptr, 10);
      ASSERT (result == (long) 4294967295UL);
      ASSERT (ptr == input + 10);
      ASSERT (errno == 0);
    }

  /* Hexadecimal integer syntax.  */
  {
    const char input[] = "0x2A";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0L);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0x2A";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 16);
    ASSERT (result == 42L);
    ASSERT (ptr == input + 4);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0x2A";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 0);
    ASSERT (result == 42L);
    ASSERT (ptr == input + 4);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0x";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0L);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0x";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 16);
    ASSERT (result == 0L);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0x";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 0);
    ASSERT (result == 0L);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }

  /* Binary integer syntax.  */
  {
    const char input[] = "0b111010";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0L);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0b111010";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 2);
    ASSERT (result == 58L);
    ASSERT (ptr == input + 8);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0b111010";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 0);
    ASSERT (result == 58L);
    ASSERT (ptr == input + 8);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0b";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 10);
    ASSERT (result == 0L);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0b";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 2);
    ASSERT (result == 0L);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }
  {
    const char input[] = "0b";
    char *ptr;
    long result;
    errno = 0;
    result = strtol (input, &ptr, 0);
    ASSERT (result == 0L);
    ASSERT (ptr == input + 1);
    ASSERT (errno == 0);
  }

  return 0;
}
