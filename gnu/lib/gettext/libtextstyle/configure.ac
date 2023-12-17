dnl Configuration for the toplevel directory of GNU libtextstyle
dnl Copyright (C) 2009-2023 Free Software Foundation, Inc.
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
AC_INIT
AC_CONFIG_SRCDIR([version.sh])
AC_CONFIG_AUX_DIR([build-aux])
. $srcdir/version.sh
gl_INIT_PACKAGE([libtextstyle], [$VERSION_NUMBER])
AM_INIT_AUTOMAKE([1.13 silent-rules])
AC_CONFIG_HEADERS([config.h])

dnl Override automake's tar command used for creating distributions.
am__tar='${AMTAR} chof - --owner=root --group=root "$$tardir"'

dnl Checks for basic programs.

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MKDIR_P

dnl Check for host type.

AC_CANONICAL_HOST

dnl Checks that augment the CPPFLAGS.

lts_EARLY

dnl Checks for compiler output filename suffixes.

AC_OBJEXT
AC_EXEEXT

dnl Check for build configuration.

gt_MORE_WARNINGS
gl_CC_INHIBIT_WARNINGS

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

dnl Checks for types, header files, functions and declarations.

lts_INIT

dnl Arrange that the gnulib-generated <unistd.h> does not declare isatty(). The
dnl adhoc-tests want to get this declaration from <textstyle.h>, not <unistd.h>.
GL_LTS_GNULIB_ISATTY=0
GL_LTS_GNULIB_MDA_ISATTY=0

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
dnl For substitution in woe32dll.in.h.
if test $is_woe32dll = yes; then
  DLL_VARIABLE='__declspec (dllimport)'
else
  DLL_VARIABLE=
fi
AC_SUBST([DLL_VARIABLE])

dnl Put some default definitions into config.h.
AH_BOTTOM([
/* In order to define all the symbols declared by textstyle.h, do not use
   'inline' functions, even when the compiler would support them.  */
#include "notinline.h"

/* Ignore DLL_VARIABLE markup that is meant for gettext.  */
#define DLL_VARIABLE

/* Get LIBTEXTSTYLE_DLL_VARIABLE.  */
#include "textstyle/woe32dll.h"
])

dnl Namespacing is the default: it builds a namespace clean library.
dnl Allow the user to disable namespacing. This speeds up "make" considerably.
AC_ARG_ENABLE([namespacing],
  [  --disable-namespacing   build a library that exports undesired symbols],
  [:], [enable_namespacing=yes])
if test "$enable_namespacing" != no; then
  NAMESPACING=yes
else
  NAMESPACING=
fi
AC_SUBST([NAMESPACING])

dnl Check for prerequisites of exported.sh.
gt_GLOBAL_SYMBOL_PIPE

dnl Substitutable version number.
AC_PROG_AWK
changequote(,)
sed_extract_major='/^[0-9]/{s/^\([0-9]*\).*/\1/p;q;}
i\
0
q
'
sed_extract_minor='/^[0-9][0-9]*[.][0-9]/{s/^[0-9]*[.]\([0-9]*\).*/\1/p;q;}
i\
0
q
'
sed_extract_subminor='/^[0-9][0-9]*[.][0-9][0-9]*[.][0-9]/{s/^[0-9]*[.][0-9]*[.]\([0-9]*\).*/\1/p;q;}
i\
0
q
'
changequote([,])
version_major=`echo "${VERSION}" | sed -n -e "$sed_extract_major"`
version_minor=`echo "${VERSION}" | sed -n -e "$sed_extract_minor"`
version_subminor=`echo "${VERSION}" | sed -n -e "$sed_extract_subminor"`
HEXVERSION=0x`$AWK 'BEGIN { printf("%02X%02X%02X"',"$version_major","$version_minor","$version_subminor"') }'`
AC_SUBST([HEXVERSION])

dnl For substitution in version.in.h.
if test "$am_func_iconv" = yes; then
  HAVE_ICONV=1
else
  HAVE_ICONV=0
fi
AC_SUBST([HAVE_ICONV])

dnl Check for tools needed for formatting the documentation.
ac_aux_dir_abs=`cd $ac_aux_dir && pwd`
AC_PATH_PROG([TEXI2DVI], [texi2dvi], [$ac_aux_dir_abs/missing texi2dvi])
AC_PATH_PROG([DVIPS], [dvips], [$ac_aux_dir_abs/missing dvips])
AC_PATH_PROG([PERL], [perl], [$ac_aux_dir_abs/missing perl])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([lib/Makefile])
AC_CONFIG_FILES([lib/exported.sh])
AC_CONFIG_FILES([lib/textstyle/version.h:lib/textstyle/version.in.h])
AC_CONFIG_FILES([lib/textstyle/woe32dll.h:lib/textstyle/woe32dll.in.h])
AC_CONFIG_FILES([tests/Makefile])
AC_CONFIG_FILES([adhoc-tests/Makefile])
AC_OUTPUT
