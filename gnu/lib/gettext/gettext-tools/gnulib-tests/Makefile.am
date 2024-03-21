## Makefile for the gettext-tools/gnulib-tests subdirectory of GNU gettext
## Copyright (C) 2007, 2009-2010, 2019, 2021 Free Software Foundation, Inc.
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

# Rules generated and collected by gnulib-tool.
include Makefile.gnulib

AUTOMAKE_OPTIONS += 1.11 color-tests

# Ensure that test-quotearg.c finds libintl.h.
AM_CPPFLAGS += -I../../gettext-runtime/intl


# Remove .deps directories created by 'configure'.
# I would say that this left-over is an Automake 1.16.1 bug.
distclean-local:
	rm -rf $(DEPDIR) */$(DEPDIR)
