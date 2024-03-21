dnl Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright (C) 2012-2023 Free Software Foundation, Inc.
dnl
dnl This file is free software; you can redistribute it and/or modify
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
dnl along with this program; see the file COPYING3.  If not see
dnl <http://www.gnu.org/licenses/>.
dnl
m4_include([../bfd/version.m4])
AC_INIT([gprof], BFD_VERSION)
AC_CONFIG_SRCDIR([gprof.c])

AC_CANONICAL_TARGET([])

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

dnl For simplicity, we use the BFD configuration file for most
dnl things.  However, we also need our own configuration file for
dnl the automake PACKAGE and VERSION macros.  We don't name it
dnl config.h, to avoid any possible confusion with the bfd config.h.
AC_CONFIG_HEADERS([gconfig.h:gconfig.in])

AC_PROG_CC
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_INSTALL

LT_INIT
ACX_LARGEFILE

# These are in addition to what is made available in bfd/.
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_FUNCS(setitimer)

ALL_LINGUAS="bg da de eo es fi fr ga hu id it ja ms nl pt_BR ro ru rw sr sv tr uk vi"
ZW_GNU_GETTEXT_SISTER_DIR
AM_PO_SUBDIRS

AM_MAINTAINER_MODE
AM_CONDITIONAL(GENINSRC_NEVER, false)
AC_EXEEXT

AC_CHECK_HEADERS(sys/gmon_out.h)

AC_MSG_CHECKING(for a known getopt prototype in unistd.h)
AC_CACHE_VAL(gprof_cv_decl_getopt_unistd_h,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>], [extern int getopt (int, char *const*, const char *);])],
gprof_cv_decl_getopt_unistd_h=yes, gprof_cv_decl_getopt_unistd_h=no)])
AC_MSG_RESULT($gprof_cv_decl_getopt_unistd_h)
if test $gprof_cv_decl_getopt_unistd_h = yes; then
  AC_DEFINE([HAVE_DECL_GETOPT], 1,
	    [Is the prototype for getopt in <unistd.h> in the expected format?])
fi

# Some systems have fabs only in -lm, not in -lc.
AC_SEARCH_LIBS(fabs, m)

AM_BINUTILS_WARNINGS

dnl Required by html, pdf, install-pdf and install-html
AC_SUBST(datarootdir)
AC_SUBST(docdir)
AC_SUBST(htmldir)
AC_SUBST(pdfdir)

AC_CONFIG_FILES([Makefile po/Makefile.in:po/Make-in])
AC_OUTPUT

GNU_MAKE_JOBSERVER
