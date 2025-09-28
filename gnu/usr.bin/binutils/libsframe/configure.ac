dnl                                            -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright (C) 2022-2023 Free Software Foundation, Inc.
dnl
dnl This file is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; see the file COPYING.  If not see
dnl <http://www.gnu.org/licenses/>.
dnl

AC_INIT([libsframe], BFD_VERSION)
AC_CONFIG_SRCDIR(sframe.c)
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

dnl Default to a non shared library.  This may be overridden by the
dnl configure option --enable-shared.
AC_DISABLE_SHARED

LT_INIT
AC_SYS_LARGEFILE

MISSING=`cd $ac_aux_dir && ${PWDCMD-pwd}`/missing
AC_CHECK_PROGS([ACLOCAL], [aclocal], [$MISSING aclocal])
AC_CHECK_PROGS([AUTOCONF], [autoconf], [$MISSING autoconf])
AC_CHECK_PROGS([AUTOHEADER], [autoheader], [$MISSING autoheader])

dnl Figure out what compiler warnings we can enable.
dnl See config/warnings.m4 for details.

ACX_PROG_CC_WARNING_OPTS([-Wall -Wextra -Wwrite-strings \
			  -Wmissing-format-attribute \
			  -Wstrict-prototypes -Wmissing-prototypes],
			  [ac_libsframe_warn_cflags])

dnl The libsframe testsuite uses dejagnu.h.
dnl If the version of dejagnu and gcc are incompatible then the libsframe
dnl testsuite is not built.

DEJAGNU_CHECK_VERSION
AM_CONDITIONAL([HAVE_COMPAT_DEJAGNU], [test "x$ac_cv_dejagnu_compat" = "xyes"])

COMPAT_DEJAGNU=$ac_cv_dejagnu_compat
AC_SUBST(COMPAT_DEJAGNU)

dnl Check for makeinfo for building documentation
build_info=
AC_CHECK_PROGS([MAKEINFO], makeinfo, makeinfo, )
if test "x$MAKEINFO" = "x"; then
    MAKEINFO="@echo makeinfo missing; true"
    build_info=
else
    BUILD_INFO=info
    case "$MAKEINFO" in
      */missing\ makeinfo*)
	build_info=
	AC_MSG_WARN([
*** Makeinfo is missing. Info documentation will not be built.])
        ;;
      *)
	build_info=yes
	;;
    esac
fi
AM_CONDITIONAL(BUILD_INFO, test "${build_info}" = yes)

dnl Determine if using Solaris linker
AC_MSG_CHECKING([if using Solaris linker])
SLD=`$LD --version 2>&1 | grep Solaris`
if test "$SLD"; then
  have_solaris_ld=yes
  AC_MSG_RESULT(yes)
else
  have_solaris_ld=no
  AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(HAVE_SOLARIS_LD, test "$have_solaris_ld" = "yes")

if test "$have_solaris_ld" = "yes"; then
  GLD=`$LD --help < /dev/null 2>&1 | grep 'M mapfile'`
else
  GLD=`$LD --help < /dev/null 2>/dev/null | grep version-script`
fi

if test "$GLD"; then
  have_ld_version_script=yes
  AC_MSG_RESULT(yes)
else
  have_ld_version_script=no
  AC_MSG_RESULT(no)
  AC_MSG_WARN(*** Versioned symbols not enabled.)
fi
AM_CONDITIONAL(HAVE_LD_VERSION_SCRIPT, test "$have_ld_version_script" = "yes")

AM_MAINTAINER_MODE
AM_INSTALL_LIBBFD

AC_FUNC_MMAP
AC_CHECK_HEADERS(byteswap.h endian.h)

dnl Check for bswap_{16,32,64}
AC_CHECK_DECLS([bswap_16, bswap_32, bswap_64], [], [], [[#include <byteswap.h>]])

AC_CONFIG_FILES(Makefile)
AC_CONFIG_HEADERS(config.h)
AC_OUTPUT

GNU_MAKE_JOBSERVER
