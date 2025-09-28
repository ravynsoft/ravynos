/* Test of <stdio.h> substitute.
   Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.

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

/* Check that the various SEEK_* macros are defined.  */
int sk[] = { SEEK_CUR, SEEK_END, SEEK_SET };

/* Check that the _PRINTF_NAN_LEN_MAX macro is defined.  */
int pnlm[] = { _PRINTF_NAN_LEN_MAX };

/* Check that NULL can be passed through varargs as a pointer type,
   per POSIX 2008.  */
static_assert (sizeof NULL == sizeof (void *));

/* Check that the types are all defined.  */
fpos_t t1;
off_t t2;
size_t t3;
ssize_t t4;
va_list t5;

#include <string.h>

#include "signed-nan.h"
#include "signed-snan.h"
#include "macros.h"

int
main (void)
{
  {
    double value1;
    char buf[64];

    value1 = positive_NaNd();
    sprintf (buf, "%g", value1);
    ASSERT (strlen (buf) <= _PRINTF_NAN_LEN_MAX);

    value1 = negative_NaNd();
    sprintf (buf, "%g", value1);
    ASSERT (strlen (buf) <= _PRINTF_NAN_LEN_MAX);
  }
#if defined DBL_EXPBIT0_WORD && defined DBL_EXPBIT0_BIT
  /* Check the value of _PRINTF_NAN_LEN_MAX.  */
  {
    double value1;
    memory_double value2;
    char buf[64];

    value1 = positive_SNaNd();
    sprintf (buf, "%g", value1);
    ASSERT (strlen (buf) <= _PRINTF_NAN_LEN_MAX);

    value1 = negative_SNaNd();
    sprintf (buf, "%g", value1);
    ASSERT (strlen (buf) <= _PRINTF_NAN_LEN_MAX);

    value2.value = positive_NaNd ();
    #if DBL_EXPBIT0_BIT == 20
    value2.word[DBL_EXPBIT0_WORD] ^= 0x54321;
    #endif
    sprintf (buf, "%g", value2.value);
    ASSERT (strlen (buf) <= _PRINTF_NAN_LEN_MAX);

    value2.value = negative_NaNd ();
    #if DBL_EXPBIT0_BIT == 20
    value2.word[DBL_EXPBIT0_WORD] ^= 0x54321;
    #endif
    sprintf (buf, "%g", value2.value);
    ASSERT (strlen (buf) <= _PRINTF_NAN_LEN_MAX);
  }
#endif

  return 0;
}
