/* Substitute for and wrapper around <utime.h>.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

#ifndef _@GUARD_PREFIX@_UTIME_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* This file uses #include_next of a system file that defines time_t.
   For the 'year2038' module to work right, <config.h> needs to have been
   included before.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* The include_next requires a split double-inclusion guard.  */
#if @HAVE_UTIME_H@
# @INCLUDE_NEXT@ @NEXT_UTIME_H@
#endif

#ifndef _@GUARD_PREFIX@_UTIME_H
#define _@GUARD_PREFIX@_UTIME_H

/* This file uses GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#if !@HAVE_UTIME_H@
# include <sys/utime.h>
#endif

#if @GNULIB_UTIME@
/* Get struct timespec.  */
# include <time.h>
#endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


#if defined _WIN32 && ! defined __CYGWIN__

/* Define 'struct utimbuf' as an alias of 'struct _utimbuf'
   (or possibly, if present, 'struct __utimbuf64').  */
# define utimbuf _utimbuf

#endif


#if @GNULIB_UTIME@
# if @REPLACE_UTIME@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define utime rpl_utime
#  endif
_GL_FUNCDECL_RPL (utime, int, (const char *filename, const struct utimbuf *ts)
                              _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (utime, int, (const char *filename, const struct utimbuf *ts));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef utime
#   define utime _utime
#  endif
_GL_CXXALIAS_MDA (utime, int, (const char *filename, const struct utimbuf *ts));
# else
#  if !@HAVE_UTIME@
_GL_FUNCDECL_SYS (utime, int, (const char *filename, const struct utimbuf *ts)
                              _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (utime, int, (const char *filename, const struct utimbuf *ts));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (utime);
# endif
#elif defined GNULIB_POSIXCHECK
# undef utime
# if HAVE_RAW_DECL_UTIME
_GL_WARN_ON_USE (utime,
                 "utime is unportable - "
                 "use gnulib module canonicalize-lgpl for portability");
# endif
#elif @GNULIB_MDA_UTIME@
/* On native Windows, map 'utime' to '_utime', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::utime always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef utime
#   define utime _utime
#  endif
_GL_CXXALIAS_MDA (utime, int, (const char *filename, const struct utimbuf *ts));
# else
_GL_CXXALIAS_SYS (utime, int, (const char *filename, const struct utimbuf *ts));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (utime);
# endif
#endif

#if @GNULIB_UTIME@
extern int _gl_utimens_windows (const char *filename, struct timespec ts[2]);
#endif


#endif /* _@GUARD_PREFIX@_UTIME_H */
#endif /* _@GUARD_PREFIX@_UTIME_H */
