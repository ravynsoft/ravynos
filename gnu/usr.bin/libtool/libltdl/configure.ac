# Process this file with autoconf to create configure. -*- autoconf -*-
#
#    Copyright (C) 2004-2005, 2007-2008, 2011-2019, 2021-2022 Free
#    Software Foundation, Inc.
#    Written by Gary V. Vaughan, 2004
#
#    NOTE: The canonical source of this file is maintained with the
#    GNU Libtool package.  Report bugs to bug-libtool@gnu.org.
#
# GNU Libltdl is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# As a special exception to the GNU Lesser General Public License,
# if you distribute this file as part of a program or library that
# is built using GNU libtool, you may include this file under the
# same distribution terms that you use for the rest of that program.
#
# GNU Libltdl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU LesserGeneral Public
# License along with GNU Libltdl; see the file COPYING.LIB.  If not, a
# copy can be downloaded from http://www.gnu.org/licenses/lgpl.html,
# or obtained by writing to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
####

# This configure.ac is not used at all by the libtool bootstrap, but
# is copied to the ltdl subdirectory if you libtoolize --ltdl your own
# project.  Adding LT_WITH_LTDL to your project configure.ac will then
# configure this directory if your user doesn't want to use the installed
# libltdl.

AC_PREREQ(2.59)dnl We use AS_HELP_STRING


## ------------------------ ##
## Autoconf initialisation. ##
## ------------------------ ##
AC_INIT([libltdl], [2.4.3a], [bug-libtool@gnu.org])
AC_CONFIG_HEADERS([config.h:config-h.in])
AC_CONFIG_SRCDIR([ltdl.c])
AC_CONFIG_AUX_DIR([../build-aux])
LT_CONFIG_LTDL_DIR([.]) # I am me!


## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##

AM_INIT_AUTOMAKE([gnu subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])


## ------------------------------- ##
## Libtool specific configuration. ##
## ------------------------------- ##
pkgdatadir='$datadir'"/$PACKAGE"


## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
LT_INIT([dlopen win32-dll])
_LTDL_SETUP


## -------- ##
## Outputs. ##
## -------- ##
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
