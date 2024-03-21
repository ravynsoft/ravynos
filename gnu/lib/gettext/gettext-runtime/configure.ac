dnl Configuration for the gettext-runtime directory of GNU gettext
dnl Copyright (C) 1995-2010, 2012-2015, 2018-2023 Free Software Foundation, Inc.
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
AC_INIT([gettext-runtime],
	m4_esyscmd([../build-aux/git-version-gen ../.tarball-version]),
	[bug-gettext@gnu.org])
AC_CONFIG_SRCDIR([intl/dcigettext.c])
AC_CONFIG_AUX_DIR([../build-aux])
AM_INIT_AUTOMAKE([1.11.1 silent-rules parallel-tests])
AC_CONFIG_HEADERS([config.h])

dnl Installation directories.
test "$docdir" != '${datarootdir}/doc/${PACKAGE_TARNAME}' || docdir='${datarootdir}/doc/gettext'

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

gt_JAVA_CHOICE
AS_IF([test "$JAVA_CHOICE" != no], [
  gt_JAVACOMP([1.5], [1.6])
  AC_CHECK_PROG([JAR], [jar], [jar])
  if test -n "$HAVE_JAVACOMP" && test -n "$JAR"; then
    BUILDJAVA=yes
  else
    BUILDJAVA=no
  fi
], [
  gt_JAVACOMP_DISABLED
  JAR=
  BUILDJAVA=no
])
AC_SUBST([BUILDJAVA])

gt_CSHARPCOMP
if test -n "$HAVE_CSHARPCOMP" && test "$CSHARP_CHOICE" != no; then
  BUILDCSHARP=yes
else
  BUILDCSHARP=no
fi
AC_SUBST([BUILDCSHARP])

dnl Check for host type.
AC_CANONICAL_HOST

dnl Checks for compiler output filename suffixes.
AC_OBJEXT
AC_EXEEXT

dnl For a 64-bit build on AIX, AC_PROG_RANLIB is not sufficient.
gl_PROG_AR_RANLIB

dnl Set preferences for the gnulib module 'threadlib'.
gl_AVOID_WINPTHREAD

dnl Make sure we see all GNU and Solaris extensions.
gl_EARLY
AM_PROG_CC_C_O

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
if test $is_woe32 = yes; then
  dnl Check for a program that compiles Windows resource files.
  AC_CHECK_TOOL([WINDRES], [windres])
fi

dnl Some code is only meant to be compiled on native Windows.
case "$host_os" in
  mingw*) is_windows_native=yes ;;
  *) is_windows_native=no ;;
esac
AM_CONDITIONAL([WINDOWS_NATIVE], [test $is_windows_native = yes])

dnl Checks for libraries.

dnl These are the only lines required to internationalize the package.
dnl (OK, not quite, the AC_CONFIG_FILES has also some parts.)
AM_GNU_GETTEXT([use-libtool], [need-ngettext], [\${top_builddir}/intl])

dnl Checks for header files, functions and declarations.
gl_INIT
AC_TYPE_SIZE_T
AC_CHECK_FUNCS([setlocale])

dnl Compilation on mingw and Cygwin needs special Makefile rules, because
dnl 1. when we install a shared library, we must arrange to export
dnl    auxiliary pointer variables for every exported variable,
dnl 2. when we install a shared library and a static library simultaneously,
dnl    the include file specifies __declspec(dllimport) and therefore we
dnl    must arrange to define the auxiliary pointer variables for the
dnl    exported variables _also_ in the static library.
if test "$enable_shared" = yes; then
  case "$host_os" in
    mingw* | cygwin*) is_woe32dll=yes ;;
    *) is_woe32dll=no ;;
  esac
else
  is_woe32dll=no
fi
AM_CONDITIONAL([WOE32DLL], [test $is_woe32dll = yes])
if test $is_woe32dll = yes; then
  AC_DEFINE([WOE32DLL], [1],
    [Define when --enable-shared is used on mingw or Cygwin.])
fi

dnl Put some default definitions into config.h.
AH_BOTTOM([
/* On Windows, variables that may be in a DLL must be marked specially.
   But here, the only such variable is 'exit_failure', and it is always
   built into a static library, never into a shared library.  */
#if defined WOE32DLL && !defined IN_RELOCWRAPPER && 0
# define DLL_VARIABLE __declspec (dllimport)
#else
# define DLL_VARIABLE
#endif

/* Extra OS/2 (emx+gcc) defines.  */
#if defined __EMX__ && !defined __KLIBC__
# include "intl/os2compat.h"
#endif
])

dnl Check for tools needed for formatting the documentation.
ac_aux_dir_abs=`cd $ac_aux_dir && pwd`
AC_PATH_PROG([PERL], [perl], [$ac_aux_dir_abs/missing perl])

dnl Check for tools needed for formatting man pages.
CROSS_COMPILING=$cross_compiling
AC_SUBST([CROSS_COMPILING])

dnl Checks for optional programs for the tests/lang-* tests.
gl_PROG_ANSI_CXX([CXX], [ANSICXX])

dnl Checks for special libraries for the tests/test-lock test.
dnl On some systems, sched_yield is in librt, rather than in libpthread.
LIBSCHED=
if test $gl_threads_api = posix; then
  dnl Solaris has sched_yield in librt, not in libpthread or libc.
  AC_CHECK_LIB([rt], [sched_yield], [LIBSCHED=-lrt],
    [dnl Solaris 2.5.1, 2.6 has sched_yield in libposix4, not librt.
     AC_CHECK_LIB([posix4], [sched_yield], [LIBSCHED=-lposix4])])
fi
AC_SUBST([LIBSCHED])

dnl Determine whether to build libasprintf.
AC_ARG_ENABLE([libasprintf],
  [  --disable-libasprintf   do not build libasprintf],
  [if test "$enableval" != no; then
     SUBDIR_libasprintf=libasprintf
   else
     SUBDIR_libasprintf=
   fi
  ], [SUBDIR_libasprintf=libasprintf])
if test "$CXX" = "no"; then
  SUBDIR_libasprintf=
fi
AM_CONDITIONAL([ENABLE_LIBASPRINTF], [test -n "$SUBDIR_libasprintf"])

dnl Determine variables used by uninstalled-config.sh.
ENABLE_SHARED="$enable_shared"
AC_SUBST([ENABLE_SHARED])

AC_CONFIG_SUBDIRS([intl])

AC_CONFIG_SUBDIRS([libasprintf])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([uninstalled-config.sh])

AC_CONFIG_FILES([doc/Makefile])

AC_CONFIG_FILES([intl-java/Makefile])

AC_CONFIG_FILES([intl-csharp/Makefile])

AC_CONFIG_FILES([gnulib-lib/Makefile])

AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/gettext.sh], [chmod a+x src/gettext.sh])

AC_CONFIG_FILES([po/Makefile.in])

AC_CONFIG_FILES([man/Makefile],
                [FIX_MAKEFILE_DISTRIB])
AC_CONFIG_FILES([man/x-to-1])

AC_CONFIG_FILES([m4/Makefile])

AC_CONFIG_FILES([tests/Makefile])

AC_OUTPUT
