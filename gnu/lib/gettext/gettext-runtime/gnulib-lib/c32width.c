/* Determine the number of screen columns needed for a 32-bit wide character.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#include <config.h>

#define IN_C32WIDTH
/* Specification.  */
#include <uchar.h>

#include <wchar.h>

#ifdef __CYGWIN__
# include <cygwin/version.h>
#endif

#if GNULIB_defined_mbstate_t
# include "streq.h"
#endif

#include "localcharset.h"

#if GL_CHAR32_T_IS_UNICODE
# include "lc-charset-unicode.h"
#endif

#include "uniwidth.h"

#if _GL_WCHAR_T_IS_UCS4 && !GNULIB_defined_mbstate_t
_GL_EXTERN_INLINE
#endif
int
c32width (char32_t wc)
{
  /* The char32_t encoding of a multibyte character is defined by the way
     mbrtoc32() is defined.  */

#if GNULIB_defined_mbstate_t            /* AIX, IRIX */
  /* mbrtoc32() is defined on top of mbtowc() for the non-UTF-8 locales
     and directly for the UTF-8 locales.  */
  const char *encoding = locale_charset ();
  if (STREQ_OPT (encoding, "UTF-8", 'U', 'T', 'F', '-', '8', 0, 0, 0, 0))
    return uc_width (wc, encoding);
  else
    return wcwidth (wc);

#elif HAVE_WORKING_MBRTOC32             /* glibc, Android */
  /* mbrtoc32() is essentially defined by the system libc.  */

# if _GL_WCHAR_T_IS_UCS4
  /* The char32_t encoding of a multibyte character is known to be the same as
     the wchar_t encoding.  */
  return wcwidth (wc);
# else
  /* The char32_t encoding of a multibyte character is known to be UCS-4,
     different from the wchar_t encoding.  */
  return uc_width (wc, locale_charset ());
# endif

#elif _GL_SMALL_WCHAR_T                 /* Cygwin, mingw, MSVC */
  /* The wchar_t encoding is UTF-16.
     The char32_t encoding is UCS-4.  */

# if defined __CYGWIN__ && CYGWIN_VERSION_DLL_MAJOR >= 1007 && 0
  /* As an extension to POSIX, the wcwidth() function of Cygwin >= 1.7
     supports also wc arguments outside the Unicode BMP, that is, outside
     the 'wchar_t' range.  See
     <https://www.cygwin.com/cgit/newlib-cygwin/commit/?id=098a75dc51caa98f369d98a9809d773bc45329aa>.
     But the resulting values for these characters are not of good quality.  */
  return wcwidth (wc);
# else
  if (wc == (wchar_t) wc)
    /* wc is in the range for the wcwidth function.  */
    return wcwidth (wc);
  else
    return uc_width (wc, locale_charset ());
# endif

#else /* macOS, FreeBSD, NetBSD, OpenBSD, HP-UX, Solaris, Minix, Android */
  /* char32_t and wchar_t are equivalent.  */
  static_assert (sizeof (char32_t) == sizeof (wchar_t));

# if GL_CHAR32_T_IS_UNICODE && GL_CHAR32_T_VS_WCHAR_T_NEEDS_CONVERSION
  return uc_width (wc, locale_charset ());
# endif
  return wcwidth (wc);
#endif
}
