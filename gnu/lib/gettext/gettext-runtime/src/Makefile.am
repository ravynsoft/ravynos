## Makefile for the gettext-runtime/src subdirectory of GNU gettext
## Copyright (C) 1995-1998, 2000-2007, 2009, 2019, 2023 Free Software Foundation, Inc.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies
EXTRA_DIST =
MOSTLYCLEANFILES = core *.stackdump
CLEANFILES =
DISTCLEANFILES =

RM = rm -f

bin_PROGRAMS = gettext ngettext envsubst

AM_CPPFLAGS = \
  -I. -I$(srcdir) \
  -I.. \
  -I../intl -I$(srcdir)/../intl \
  -I../gnulib-lib -I$(srcdir)/../gnulib-lib
DEFS = -DLOCALEDIR=$(localedir_c_make) @DEFS@

# Enable more warning options in this directory.
AM_CFLAGS = @WARN_CFLAGS@

# Source dependencies.
gettext_SOURCES = gettext.c escapes.h
ngettext_SOURCES = ngettext.c escapes.h
envsubst_SOURCES = envsubst.c

# Link dependencies.
# Need @LTLIBICONV@ because striconv.c uses iconv().
LDADD = ../gnulib-lib/libgrt.a @LTLIBINTL@ @LTLIBICONV@ $(WOE32_LDADD)

# Specify installation directory, for --enable-relocatable.
gettext_CFLAGS = -DINSTALLDIR=\"$(bindir)\"
ngettext_CFLAGS = -DINSTALLDIR=\"$(bindir)\"
envsubst_CFLAGS = -DINSTALLDIR=\"$(bindir)\"
if RELOCATABLE_VIA_LD
gettext_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
ngettext_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
envsubst_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
endif

# For installing gettext.sh in $(bindir).
bin_SCRIPTS = gettext.sh
DISTCLEANFILES += gettext.sh

# Support for relocatability.
RELOCATABLE_LIBRARY_PATH = $(libdir)
RELOCATABLE_SRC_DIR = $(top_srcdir)/gnulib-lib
RELOCATABLE_BUILD_DIR = ../gnulib-lib
RELOCATABLE_CONFIG_H_DIR = ..
RELOCATABLE_STRIP = :

# Version information according to Woe32 conventions.
if WOE32
WOE32_LDADD = gettext.res
gettext.res : $(top_srcdir)/../windows/gettext.rc
	$(WINDRES) `$(SHELL) $(top_srcdir)/../windows/windres-options --escape $(VERSION)` -i $(top_srcdir)/../windows/gettext.rc -o gettext.res --output-format=coff
MOSTLYCLEANFILES += gettext.res
else
WOE32_LDADD =
endif
