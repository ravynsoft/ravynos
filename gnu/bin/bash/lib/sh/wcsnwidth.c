/* wcsnwidth.c - compute display width of wide character string, up to max
		 specified width, return length. */

/* Copyright (C) 2012 Free Software Foundation, Inc.

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

#if defined (HANDLE_MULTIBYTE)

#include <stdc.h>
#include <wchar.h>
#include <bashansi.h>

/* Return the number of wide characters that will be displayed from wide string
   PWCS.  If the display width exceeds MAX, return the number of wide chars
   from PWCS required to display MAX characters on the screen. */
int
wcsnwidth(pwcs, n, max)
     const wchar_t *pwcs;
     size_t n, max;
{
  wchar_t wc, *ws;
  int len, l;

  len = 0;
  ws = (wchar_t *)pwcs;
  while (n-- > 0 && (wc = *ws++) != L'\0')
    {
      l = wcwidth (wc);
      if (l < 0)
	return (-1);
      else if (l == max - len)
        return (ws - pwcs);
      else if (l > max - len)
        return (--ws - pwcs);
      len += l;
    }
  return (ws - pwcs);
}
#endif
