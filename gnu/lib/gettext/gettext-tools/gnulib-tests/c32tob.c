/* Convert 32-bit wide character to unibyte character.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2020.  */

#include <config.h>

#define IN_C32TOB
/* Specification.  */
#include <uchar.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#if GL_CHAR32_T_IS_UNICODE
# include "lc-charset-unicode.h"
#endif

#if _GL_WCHAR_T_IS_UCS4
_GL_EXTERN_INLINE
#endif
int
c32tob (wint_t wc)
{
#if HAVE_WORKING_MBRTOC32 && !_GL_WCHAR_T_IS_UCS4
  /* The char32_t encoding of a multibyte character may be different than its
     wchar_t encoding.  */
  if (wc != WEOF)
    {
      mbstate_t state;
      char buf[8];

      mbszero (&state);
      if (c32rtomb (buf, wc, &state) == 1)
        return (unsigned char) buf[0];
    }
  return EOF;
#elif _GL_SMALL_WCHAR_T
  /* In all known encodings, unibyte characters correspond only to
     characters in the BMP.  */
  if (wc != WEOF && (wchar_t) wc == wc)
    return wctob ((wchar_t) wc);
  else
    return EOF;
#else
# if GL_CHAR32_T_IS_UNICODE && GL_CHAR32_T_VS_WCHAR_T_NEEDS_CONVERSION
  if (wc != 0)
    {
      wc = unicode_to_locale_encoding (wc);
      if (wc == 0)
        return EOF;
    }
# endif
  return wctob (wc);
#endif
}
