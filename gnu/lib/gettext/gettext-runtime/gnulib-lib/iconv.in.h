/* A GNU-like <iconv.h>.

   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

#ifndef _@GUARD_PREFIX@_ICONV_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* The include_next requires a split double-inclusion guard.  */
#@INCLUDE_NEXT@ @NEXT_ICONV_H@

#ifndef _@GUARD_PREFIX@_ICONV_H
#define _@GUARD_PREFIX@_ICONV_H

/* This file uses GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


#if @GNULIB_ICONV@
# if @REPLACE_ICONV_OPEN@
/* An iconv_open wrapper that supports the IANA standardized encoding names
   ("ISO-8859-1" etc.) as far as possible.  */
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define iconv_open rpl_iconv_open
#  endif
_GL_FUNCDECL_RPL (iconv_open, iconv_t,
                  (const char *tocode, const char *fromcode)
                  _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (iconv_open, iconv_t,
                  (const char *tocode, const char *fromcode));
# else
_GL_CXXALIAS_SYS (iconv_open, iconv_t,
                  (const char *tocode, const char *fromcode));
# endif
_GL_CXXALIASWARN (iconv_open);
#elif defined GNULIB_POSIXCHECK
# undef iconv_open
# if HAVE_RAW_DECL_ICONV_OPEN
_GL_WARN_ON_USE (iconv_open, "iconv_open is not working correctly everywhere - "
                 "use gnulib module iconv for portability");
# endif
#endif

#if @REPLACE_ICONV_UTF@
/* Special constants for supporting UTF-{16,32}{BE,LE} encodings.
   Not public.  */
# define _ICONV_UTF8_UTF16BE (iconv_t)(-161)
# define _ICONV_UTF8_UTF16LE (iconv_t)(-162)
# define _ICONV_UTF8_UTF32BE (iconv_t)(-163)
# define _ICONV_UTF8_UTF32LE (iconv_t)(-164)
# define _ICONV_UTF16BE_UTF8 (iconv_t)(-165)
# define _ICONV_UTF16LE_UTF8 (iconv_t)(-166)
# define _ICONV_UTF32BE_UTF8 (iconv_t)(-167)
# define _ICONV_UTF32LE_UTF8 (iconv_t)(-168)
#endif

#if @GNULIB_ICONV@
# if @REPLACE_ICONV@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define iconv rpl_iconv
#  endif
_GL_FUNCDECL_RPL (iconv, size_t,
                  (iconv_t cd,
                   @ICONV_CONST@ char **restrict inbuf,
                   size_t *restrict inbytesleft,
                   char **restrict outbuf, size_t *restrict outbytesleft));
_GL_CXXALIAS_RPL (iconv, size_t,
                  (iconv_t cd,
                   @ICONV_CONST@ char **restrict inbuf,
                   size_t *restrict inbytesleft,
                   char **restrict outbuf, size_t *restrict outbytesleft));
# else
/* Need to cast, because on some versions of Solaris, ICONV_CONST does
   not have the right value for C++.  */
_GL_CXXALIAS_SYS_CAST (iconv, size_t,
                       (iconv_t cd,
                        @ICONV_CONST@ char **restrict inbuf,
                        size_t *restrict inbytesleft,
                        char **restrict outbuf, size_t *restrict outbytesleft));
# endif
_GL_CXXALIASWARN (iconv);
# ifndef ICONV_CONST
#  define ICONV_CONST @ICONV_CONST@
# endif
#elif defined GNULIB_POSIXCHECK
# undef iconv
# if HAVE_RAW_DECL_ICONV
_GL_WARN_ON_USE (iconv, "iconv is not working correctly everywhere - "
                 "use gnulib module iconv for portability");
# endif
#endif

#if @GNULIB_ICONV@
# if @REPLACE_ICONV@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define iconv_close rpl_iconv_close
#  endif
_GL_FUNCDECL_RPL (iconv_close, int, (iconv_t cd));
_GL_CXXALIAS_RPL (iconv_close, int, (iconv_t cd));
# else
_GL_CXXALIAS_SYS (iconv_close, int, (iconv_t cd));
# endif
_GL_CXXALIASWARN (iconv_close);
#endif


#endif /* _@GUARD_PREFIX@_ICONV_H */
#endif /* _@GUARD_PREFIX@_ICONV_H */
