dnl autoconf configuration for gperf

dnl Copyright (C) 1998, 2002, 2003, 2005-2007, 2009-2010, 2012 Free Software Foundation, Inc.
dnl Written by Douglas C. Schmidt <schmidt@ics.uci.edu>
dnl and Bruno Haible <bruno@clisp.org>.
dnl
dnl This file is part of GNU GPERF.
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
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.60])
AC_INIT([doc/gperf.texi])
AC_CONFIG_AUX_DIR([build-aux])

AC_PROG_MAKE_SET
AC_OBJEXT
AC_EXEEXT

AC_CONFIG_SUBDIRS([lib src tests doc])

dnl This piece of sed script replaces every line containing '@subdir@'
dnl by several consecutive lines, each referencing one subdir.
extrasub="$extrasub"'
/@subdir@/{
h
g
s/@subdir@/lib/
p
g
s/@subdir@/src/
p
g
s/@subdir@/tests/
p
g
s/@subdir@/doc/
p
d
}
'
AC_OUTPUT([Makefile])
