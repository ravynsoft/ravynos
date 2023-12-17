/* anytostr.c -- convert integers to printable strings

   Copyright (C) 2001, 2006, 2008-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert */

/* Tell gcc not to warn about the (i < 0) test, below.  */
#if (__GNUC__ == 4 && 3 <= __GNUC_MINOR__) || 4 < __GNUC__
# pragma GCC diagnostic ignored "-Wtype-limits"
#elif defined __clang__
# pragma clang diagnostic ignored "-Wtautological-compare"
#endif

#include <config.h>

#include "inttostr.h"

/* Convert I to a printable string in BUF, which must be at least
   INT_BUFSIZE_BOUND (INTTYPE) bytes long.  Return the address of the
   printable string, which need not start at BUF.  */

_GL_ATTRIBUTE_NODISCARD char *
anytostr (inttype i, char *buf)
{
  char *p = buf + INT_STRLEN_BOUND (inttype);
  *p = 0;

  if (i < 0)
    {
      do
        *--p = '0' - i % 10;
      while ((i /= 10) != 0);

      *--p = '-';
    }
  else
    {
      do
        *--p = '0' + i % 10;
      while ((i /= 10) != 0);
    }

  return p;
}
