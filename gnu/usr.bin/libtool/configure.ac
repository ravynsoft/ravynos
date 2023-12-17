# Process this file with autoconf to create configure. -*- autoconf -*-

#   Copyright (C) 2001, 2004-2005, 2008, 2010-2019, 2021-2022 Free
#   Software Foundation, Inc.
#   Written by Gary V. Vaughan, 2001
#
#   This file is part of GNU Libtool.
#
# GNU Libtool is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# GNU Libtool is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Libtool; see the file COPYING.  If not, a copy
# can be downloaded from  http://www.gnu.org/licenses/gpl.html,
# or obtained by writing to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
####


AC_PREREQ(2.64)
dnl Oldest automake required for bootstrap is below in AM_INIT_AUTOMAKE.


## ------------------------ ##
## Autoconf initialisation. ##
## ------------------------ ##
AC_INIT([GNU Libtool],
	m4_esyscmd([build-aux/git-version-gen .tarball-version]),
	[bug-libtool@gnu.org])
m4_ifndef([AC_PACKAGE_URL],
	  [AC_SUBST([PACKAGE_URL], [http://www.gnu.org/s/libtool/])])

# Remove this when a released Autoconf is capable of bootstrapping
# Libtool without it (probably after autoconf-2.70).
m4_ifndef([AC_CONFIG_MACRO_DIRS],
	  [m4_define([AC_CONFIG_MACRO_DIRS], m4_defn([AC_CONFIG_MACRO_DIR]))])

# Since we already declare these directories in this file, save those
# choices, and push the results into Makefile.am to avoid declaring
# them in two places making renames error prone, and where copies
# might get out of sync.
m4_define([AC_CONFIG_AUX_DIR],
     m4_defn([AC_CONFIG_AUX_DIR])[AC_SUBST([aux_dir], [$1])])
m4_define([LT_CONFIG_LTDL_DIR],
     m4_defn([LT_CONFIG_LTDL_DIR])[AC_SUBST([ltdl_dir], [$1])])
m4_define([AC_CONFIG_MACRO_DIRS],
     m4_defn([AC_CONFIG_MACRO_DIRS])[AC_SUBST([macro_dir], [$1])])

AC_CONFIG_HEADERS([config.h:config-h.in])
AC_CONFIG_SRCDIR([libtoolize.in])
LT_CONFIG_LTDL_DIR([libltdl], [nonrecursive])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_LIBOBJ_DIR([libltdl])

package_revision=`$SHELL $ac_aux_dir/git-version-gen .tarball-version`
AC_SUBST([package_revision])

## ------------------------ ##
## Autotest initialisation. ##
## ------------------------ ##

AC_CONFIG_TESTDIR([tests])
AC_SUBST([AUTOM4TE], [${AUTOM4TE=autom4te}])
AC_SUBST([AUTOTEST], ['$(AUTOM4TE) --language=autotest'])


## ---------------------------------------- ##
## Display a configure time version banner. ##
## ---------------------------------------- ##

# This is a sanity check so we can see what version is used in bug reports.
AS_BOX([Configuring AC_PACKAGE_TARNAME AC_PACKAGE_VERSION])
echo


## ------------ ##
## Shell tools. ##
## ------------ ##

AC_PROG_GNU_M4


## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##

dnl These are bootstrap requirements! Once built, libtool may work with
dnl much older releases of autoconf and automake.  See release notes.
dnl 1.11 is needed for color-tests, 1.11.1 fixes a security issue.
AM_INIT_AUTOMAKE([1.11.1 gnu subdir-objects dist-xz])
AM_SILENT_RULES([yes])


## ------------------------- ##
## Autobuild initialisation. ##
## ------------------------- ##

# You can set autobuild_mode at configure time to specify a "unique"
# string for this build.
: ${autobuild_mode=default}
AB_VERSION='AC_PACKAGE_VERSION'
AB_INIT([$autobuild_mode])


dnl Make sure config.status is regenerated when the version timestamp changes
AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/.version'])


## ------------------------------- ##
## Libtool specific configuration. ##
## ------------------------------- ##

AC_SUBST([aclocaldir], ["\$(datadir)/aclocal"])
AC_SUBST([pkgdatadir], ["\$(datadir)/$PACKAGE"])

AC_ARG_ENABLE(ltdl-install,
    [AS_HELP_STRING([--disable-ltdl-install], [do not install libltdl])])
if test set != "${enable_ltdl_install+set}"; then
  enable_ltdl_install=yes
fi

# The list of scripts required to build libltdl, and which need to be
# installed by libtoolize --ltdl, and consequently need to be placed in
# $pkgdatadir by our make install rule.
pkgaux_scripts="compile config.guess config.sub depcomp install-sh missing"
AC_SUBST([pkgaux_scripts])


## ---------------- ##
## compiler checks. ##
## ---------------- ##

# Need to be sure that we have a working C compiler, otherwise even with
# a good C compiler: if the following C++ tests fail, the whole libtool
# build fails!
AC_PROG_CC
GL_EARLY
AM_PROG_CC_C_O
AC_PROG_CPP

## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
LT_INIT([dlopen win32-dll])
_LT_SET_OPTION([LTDL_INIT], [nonrecursive])
_LTDL_SETUP

# Enable all the language support we can
LT_LANG(C++)
LT_LANG(Fortran 77)
LT_LANG(Fortran)
LT_LANG(Go)
LT_LANG(Java)
LT_LANG(Windows Resource)

# Ensure the correct file name (and path) conversion function
# is available to the test suite.
AC_SUBST([to_host_file_cmd])dnl
AC_SUBST([to_tool_file_cmd])dnl

# Propagate --with-aix-soname option to the testsuite.
AC_SUBST([with_aix_soname])dnl

## ---------------------- ##
## Gnulib initialisation. ##
## ---------------------- ##
GL_INIT

## ------------- ##
## Documentation ##
## ------------- ##
AM_MISSING_PROG([HELP2MAN], [help2man])

## -------- ##
## Outputs. ##
## -------- ##
AC_CONFIG_FILES([Makefile gnulib-tests/Makefile])
AC_OUTPUT
