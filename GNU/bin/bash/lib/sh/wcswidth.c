/* wcswidth.c - compute display width of wide character string */

/* Copyright (C) 2010 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#if defined (HANDLE_MULTIBYTE) && !defined (HAVE_WCSWIDTH)

#include <stdc.h>
#include <wchar.h>
#include <bashansi.h>

int
wcswidth(pwcs, n)
     const wchar_t *pwcs;
     size_t n;
{
  wchar_t wc;
  int len, l;
			
  len = 0;
  while (n-- > 0 && (wc = *pwcs++) != L'\0')
    {
      if ((l = wcwidth(wc)) < 0)
	return (-1);
      len += l;
    }
  return (len);
}
#endif
