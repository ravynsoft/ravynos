/* Convert wide character to unibyte character.
   Copyright (C) 2008, 2010-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2008.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <wchar.h>

#include <stdio.h>
#include <stdlib.h>

int
wctob (wint_t wc)
{
  char buf[64];

  if (!(MB_CUR_MAX <= sizeof (buf)))
    abort ();
  /* Handle the case where WEOF is a value that does not fit in a wchar_t.  */
  if (wc == (wchar_t)wc)
    if (wctomb (buf, (wchar_t)wc) == 1)
      return (unsigned char) buf[0];
  return EOF;
}
