## Makefile for the adhoc-tests subdirectory of GNU libtextstyle.
## Copyright (C) 2019 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.13 gnits
EXTRA_DIST =
MOSTLYCLEANFILES = core *.stackdump
CLEANFILES =
DISTCLEANFILES =

RM = rm -f

# The list of programs that are built.
noinst_PROGRAMS = hello

# The source files of each program.
hello_SOURCES = hello.c

# Define the styles.
noinst_DATA = hello-default.css

AM_CPPFLAGS = \
  -I. -I$(srcdir) \
  -I.. \
  -I../lib -I$(srcdir)/../lib \
  -DSRCDIR=\"$(srcdir)/\"

# Link dependencies.
LDADD = ../lib/libtextstyle.la

# Additional files to be distributed.
EXTRA_DIST += $(noinst_DATA)


# Remove .deps directories created by 'configure'.
# I would say that this left-over is an Automake 1.16.1 bug.
distclean-local:
	rm -rf $(DEPDIR) */$(DEPDIR)
