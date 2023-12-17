dnl Configuration for the GNU libasprintf library
dnl Copyright (C) 2002-2012, 2016, 2018-2023 Free Software Foundation, Inc.
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <https://www.gnu.org/licenses/>.

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([libasprintf], [1.0])
AC_CONFIG_SRCDIR([vasprintf.c])
AC_CONFIG_AUX_DIR([../../build-aux])
AM_INIT_AUTOMAKE([silent-rules])
AC_CONFIG_HEADERS([config.h])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

dnl Check for host type.
AC_CANONICAL_HOST

dnl Checks for compiler output filename suffixes.
AC_OBJEXT
AC_EXEEXT

dnl For a 64-bit build on AIX, AC_PROG_RANLIB is not sufficient.
gl_PROG_AR_RANLIB

dnl Early checks, assembled by gnulib.
gl_EARLY

dnl Check for build configuration.

gt_MORE_WARNINGS

gl_WOE32_DLL

LT_INIT([win32-dll])

dnl Prepares the libtool configuration for handling of Windows resources, and
dnl sets the RC variable to a program that compiles Windows resource files.
LT_LANG([Windows Resource])

dnl On mingw and Cygwin, we can activate special Makefile rules which add
dnl version information to the shared libraries and executables.
case "$host_os" in
  mingw* | cygwin*) is_woe32=yes ;;
  *) is_woe32=no ;;
esac
AM_CONDITIONAL([WOE32], [test $is_woe32 = yes])

case "$host_os" in
  # On Cygwin, without -no-undefined, a warning is emitted and only a static
  # library is built.
  beos* | mingw* | cygwin* | os2*) LTNOUNDEF='-no-undefined' ;;
  *) LTNOUNDEF='' ;;
esac
AC_SUBST([LTNOUNDEF])

dnl Checks for libraries.

dnl Checks for header files.
AC_CHECK_HEADERS([features.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_LONG_LONG_INT
gt_TYPE_WCHAR_T
gt_TYPE_WINT_T
AC_TYPE_MBSTATE_T
AC_CHECK_TYPE([ptrdiff_t], ,
  [AC_DEFINE([ptrdiff_t], [long],
     [Define as the type of the result of subtracting two pointers, if the system doesn't define it.])
  ])
gl_AC_TYPE_INTMAX_T
gl_XSIZE

dnl Checks for library functions.

gl_INIT
AC_CHECK_FUNCS([snprintf vasprintf strnlen wcslen wcsnlen mbrtowc wcrtomb])
dnl Use the _snprintf function only if it is declared (because on NetBSD it
dnl is defined as a weak alias of snprintf; we prefer to use the latter).
AC_CHECK_DECLS([_snprintf], , , [#include <stdio.h>])
gt_PRINTF_POSIX

dnl Hide undesired symbols from the global namespace, by prefixing them with
dnl "libasprintf_".
AH_VERBATIM([0noexport], [
#define asnprintf        libasprintf_asnprintf
#define rpl_asnprintf    libasprintf_asnprintf
#define rpl_memchr       libasprintf_memchr
#define printf_fetchargs libasprintf_printf_fetchargs
#define printf_parse     libasprintf_printf_parse
#define vasnprintf       libasprintf_vasnprintf
#define rpl_vasnprintf   libasprintf_vasnprintf
#define xmax             libasprintf_xmax
#define xsum             libasprintf_xsum
#define xsum3            libasprintf_xsum3
#define xsum4            libasprintf_xsum4
])

dnl Check for tools needed for formatting the documentation.
ac_aux_dir_abs=`cd $ac_aux_dir && pwd`
AC_PATH_PROG([DVIPS], [dvips], [$ac_aux_dir_abs/missing dvips])
AC_PATH_PROG([TEXI2PDF], [texi2pdf], [$ac_aux_dir_abs/missing texi2pdf])
AC_PATH_PROG([PERL], [perl], [$ac_aux_dir_abs/missing perl])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([gnulib-lib/Makefile])

AC_OUTPUT
