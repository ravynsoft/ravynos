## Makefile for the gettext-runtime/tests subdirectory of GNU gettext
## Copyright (C) 2005-2006, 2008-2009, 2014-2015, 2019 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.11 gnits no-dependencies color-tests subdir-objects
EXTRA_DIST =

TESTS =

AM_CPPFLAGS = \
  -I.. \
  -I$(srcdir)/../intl

check_PROGRAMS =

# Clean up after Solaris cc.
clean-local:
	rm -rf SunWS_cache
