/* Test of isblank() function.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2009.  */

#include <config.h>

#include <ctype.h>

#include "signature.h"
SIGNATURE_CHECK (isblank, int, (int));

#include <limits.h>
#include <stdio.h>

#include "macros.h"

int
main ()
{
  unsigned int c;

  /* Verify the property in the "C" locale.
     POSIX specifies in
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap07.html>
     that
       - in all locales, the blank characters include the <space> and <tab>
         characters,
       - in the "POSIX" locale (which is usually the same as the "C" locale),
         the blank characters include only the ASCII <space> and <tab>
         characters.  */
  for (c = 0; c <= UCHAR_MAX; c++)
    ASSERT (!isblank (c) == !(c == ' ' || c == '\t'));
  ASSERT (!isblank (EOF));

  return 0;
}
