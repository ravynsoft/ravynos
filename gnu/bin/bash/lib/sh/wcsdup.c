/* wcsdup.c - duplicate wide character string */

/* Copyright (C) 2006 Free Software Foundation, Inc.

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

#if !defined (HAVE_WCSDUP) && defined (HANDLE_MULTIBYTE)

#include <stdc.h>
#include <wchar.h>
#include <bashansi.h>
#include <xmalloc.h>

wchar_t *
wcsdup (ws)
     const wchar_t *ws;
{
  wchar_t *ret;
  size_t len;

  len = wcslen (ws);
  ret = xmalloc ((len + 1) * sizeof (wchar_t));
  if (ret == 0)
    return ret;
  
  return (wcscpy (ret, ws));
}
#endif /* !HAVE_WCSDUP && HANDLE_MULTIBYTE */
