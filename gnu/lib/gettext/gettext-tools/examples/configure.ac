dnl Configuration for the gettext-tools/examples directory of GNU gettext
dnl Copyright (C) 2006, 2009, 2014, 2019-2020 Free Software Foundation, Inc.
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
AC_INIT([gettext-examples],
	m4_esyscmd([../../build-aux/git-version-gen ../../.tarball-version]),
	[bug-gettext@gnu.org])
AC_CONFIG_SRCDIR([installpaths.in])
AC_CONFIG_AUX_DIR([../../build-aux])
AM_INIT_AUTOMAKE([silent-rules])

dnl Installation directories.
test "$docdir" != '${datarootdir}/doc/${PACKAGE_TARNAME}' || docdir='${datarootdir}/doc/gettext'

AM_PO_SUBDIRS

aclocaldir='${datadir}/aclocal'
AC_SUBST([aclocaldir])

AC_CONFIG_FILES([Makefile],
                [FIX_MAKEFILE_TOPDIR_DISTRIB])
AC_CONFIG_FILES([installpaths])
AC_CONFIG_FILES([po/Makefile], [AM_POSTPROCESS_PO_MAKEFILE])

AC_OUTPUT
