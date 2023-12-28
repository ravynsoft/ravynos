## Process this file with automake to produce Makefile.in.
#
#   Copyright (C) 2022-2023 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not see
# <http://www.gnu.org/licenses/>.
#
ACLOCAL_AMFLAGS = -I .. -I ../config -I ../bfd

AUTOMAKE_OPTIONS = dejagnu foreign no-texinfo.tex info-in-builddir subdir-objects

# Variables that we might accumulate conditionally or in subdirs.
info_TEXINFOS =
DISTCLEANFILES =
MAINTAINERCLEANFILES =

INCDIR = $(srcdir)/../include
# include libctf for swap.h
AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/../include -I$(srcdir)/../libctf
AM_CFLAGS = @ac_libsframe_warn_cflags@
libsframe_version_info = -version-info `grep -v '^\#' $(srcdir)/libtool-version`
# libsframe does not restrict the set of exported symbols.  So, if linker does
# not support symbol versioning, there is no need to fall back on libtool's
# -export-symbols option.
if HAVE_LD_VERSION_SCRIPT
if HAVE_SOLARIS_LD
  libsframe_version_script = -Wl,-M -Wl,$(srcdir)/libsframe.ver
else
  libsframe_version_script = -Wl,--version-script=$(srcdir)/libsframe.ver
endif
endif

if INSTALL_LIBBFD
lib_LTLIBRARIES = libsframe.la
include_HEADERS = $(INCDIR)/sframe.h $(INCDIR)/sframe-api.h
else
include_HEADERS =
noinst_LTLIBRARIES = libsframe.la
endif

libsframe_la_SOURCES = sframe.c sframe-dump.c sframe-error.c
libsframe_la_CPPFLAGS = $(AM_CPPFLAGS)
libsframe_la_LDFLAGS = $(libsframe_version_info) $(libsframe_version_script)

EXTRA_DIST = libtool-version libsframe.ver
diststuff: $(EXTRA_DIST) info

include doc/local.mk

include testsuite/local.mk
