dnl Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright (C) 2021-2023 Free Software Foundation, Inc.
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

m4_include([../../bfd/version.m4])
AC_INIT([gprofng], BFD_VERSION)
AC_CONFIG_MACRO_DIRS([../../config ../..])
AC_CONFIG_AUX_DIR(../..)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

AC_CONFIG_SRCDIR(libcol_util.c)

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB
AM_PROG_AR

LT_INIT
AC_ENABLE_SHARED
AC_DISABLE_STATIC

if test "$enable_shared" != "yes"; then
  AC_MSG_WARN([Cannot set --enable-shared for gprofng/libcollector. Profiling will be unavailable.])
fi

ACX_PROG_CC_WARNING_OPTS([-Wno-nonnull-compare], [GPROFNG_NO_NONNULL_COMPARE_CFLAGS])
AC_SUBST(GPROFNG_NO_NONNULL_COMPARE_CFLAGS)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([lib-config.h:../common/lib-config.h.in])

AC_OUTPUT

