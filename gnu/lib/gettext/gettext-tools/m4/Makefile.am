## Makefile for the gettext-tools/m4 directory of GNU gettext
## Copyright (C) 2003-2010, 2013, 2017-2023 Free Software Foundation, Inc.
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

# Files installed for use by gettextize.

aclocaldir = @aclocaldir@
aclocal_DATA = \
  ../../gettext-runtime/gnulib-m4/host-cpu-c-abi.m4 \
  ../../gettext-runtime/gnulib-m4/lib-ld.m4 \
  ../../gettext-runtime/gnulib-m4/lib-link.m4 \
  ../../gettext-runtime/gnulib-m4/lib-prefix.m4 \
  ../../gettext-runtime/m4/build-to-host.m4 \
  ../../gettext-runtime/m4/gettext.m4 \
  ../../gettext-runtime/m4/iconv.m4 \
  ../../gettext-runtime/m4/intlmacosx.m4 \
  ../../gettext-runtime/m4/nls.m4 \
  ../../gettext-runtime/m4/po.m4 \
  ../../gettext-runtime/m4/progtest.m4

# Files installed for the examples.

examplesbuildauxdir = $(docdir)/examples/build-aux
examplesbuildaux_DATA = \
  ../gnulib-m4/javacomp.m4 ../gnulib-m4/javaexec.m4 \
  ../gnulib-m4/csharpcomp.m4 ../gnulib-m4/csharpexec.m4 csharpexec-test.exe

# Generate this list with
# find . -type f -name '*.m4' -printf '  %f\n' | sort | tr '\012' @ | sed 's/@$/%/;s/@/ \\@/g' | tr @% '\012\012'
EXTRA_DIST = \
  README \
  csharpexec-test.cs csharpexec-test.exe \
  exported.m4 \
  hostname.m4 \
  locale-de.m4 \
  siginfo.m4
