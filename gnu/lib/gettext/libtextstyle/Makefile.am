## Makefile for the toplevel directory of GNU libtextstyle
## Copyright (C) 1995-2022 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.13 gnu no-dependencies
ACLOCAL_AMFLAGS = -I m4 -I gnulib-m4

# The list of subdirectories containing Makefiles.
SUBDIRS = lib tests adhoc-tests doc

EXTRA_DIST = \
  version.sh \
  DEPENDENCIES \
  HACKING \
  autogen.sh \
  gnulib-local/build-aux/moopp \
  gnulib-local/lib/fd-ostream.oo.c \
  gnulib-local/lib/fd-ostream.oo.h \
  gnulib-local/lib/file-ostream.oo.c \
  gnulib-local/lib/file-ostream.oo.h \
  gnulib-local/lib/glibconfig.in.h \
  gnulib-local/lib/glib/ghash.c \
  gnulib-local/lib/glib/ghash.in.h \
  gnulib-local/lib/glib/glist.c \
  gnulib-local/lib/glib/glist.in.h \
  gnulib-local/lib/glib/gmacros.in.h \
  gnulib-local/lib/glib/gmessages.c \
  gnulib-local/lib/glib/gprimes.c \
  gnulib-local/lib/glib/gprimes.in.h \
  gnulib-local/lib/glib/gprintfint.in.h \
  gnulib-local/lib/glib/gstrfuncs.c \
  gnulib-local/lib/glib/gstrfuncs.in.h \
  gnulib-local/lib/glib/gstring.c \
  gnulib-local/lib/glib/gstring.in.h \
  gnulib-local/lib/glib/gtypes.in.h \
  gnulib-local/lib/glib.in.h \
  gnulib-local/lib/glib/README \
  gnulib-local/lib/html-ostream.oo.c \
  gnulib-local/lib/html-ostream.oo.h \
  gnulib-local/lib/html-styled-ostream.oo.c \
  gnulib-local/lib/html-styled-ostream.oo.h \
  gnulib-local/lib/iconv-ostream.oo.c \
  gnulib-local/lib/iconv-ostream.oo.h \
  gnulib-local/lib/libcroco/cr-additional-sel.c \
  gnulib-local/lib/libcroco/cr-additional-sel.h \
  gnulib-local/lib/libcroco/cr-attr-sel.c \
  gnulib-local/lib/libcroco/cr-attr-sel.h \
  gnulib-local/lib/libcroco/cr-cascade.c \
  gnulib-local/lib/libcroco/cr-cascade.h \
  gnulib-local/lib/libcroco/cr-declaration.c \
  gnulib-local/lib/libcroco/cr-declaration.h \
  gnulib-local/lib/libcroco/cr-doc-handler.c \
  gnulib-local/lib/libcroco/cr-doc-handler.h \
  gnulib-local/lib/libcroco/cr-enc-handler.c \
  gnulib-local/lib/libcroco/cr-enc-handler.h \
  gnulib-local/lib/libcroco/cr-fonts.c \
  gnulib-local/lib/libcroco/cr-fonts.h \
  gnulib-local/lib/libcroco/cr-input.c \
  gnulib-local/lib/libcroco/cr-input.h \
  gnulib-local/lib/libcroco/cr-num.c \
  gnulib-local/lib/libcroco/cr-num.h \
  gnulib-local/lib/libcroco/cr-om-parser.c \
  gnulib-local/lib/libcroco/cr-om-parser.h \
  gnulib-local/lib/libcroco/cr-parser.c \
  gnulib-local/lib/libcroco/cr-parser.h \
  gnulib-local/lib/libcroco/cr-parsing-location.c \
  gnulib-local/lib/libcroco/cr-parsing-location.h \
  gnulib-local/lib/libcroco/cr-prop-list.c \
  gnulib-local/lib/libcroco/cr-prop-list.h \
  gnulib-local/lib/libcroco/cr-pseudo.c \
  gnulib-local/lib/libcroco/cr-pseudo.h \
  gnulib-local/lib/libcroco/cr-rgb.c \
  gnulib-local/lib/libcroco/cr-rgb.h \
  gnulib-local/lib/libcroco/cr-selector.c \
  gnulib-local/lib/libcroco/cr-selector.h \
  gnulib-local/lib/libcroco/cr-sel-eng.c \
  gnulib-local/lib/libcroco/cr-sel-eng.h \
  gnulib-local/lib/libcroco/cr-simple-sel.c \
  gnulib-local/lib/libcroco/cr-simple-sel.h \
  gnulib-local/lib/libcroco/cr-statement.c \
  gnulib-local/lib/libcroco/cr-statement.h \
  gnulib-local/lib/libcroco/cr-string.c \
  gnulib-local/lib/libcroco/cr-string.h \
  gnulib-local/lib/libcroco/cr-style.c \
  gnulib-local/lib/libcroco/cr-style.h \
  gnulib-local/lib/libcroco/cr-stylesheet.c \
  gnulib-local/lib/libcroco/cr-stylesheet.h \
  gnulib-local/lib/libcroco/cr-term.c \
  gnulib-local/lib/libcroco/cr-term.h \
  gnulib-local/lib/libcroco/cr-tknzr.c \
  gnulib-local/lib/libcroco/cr-tknzr.h \
  gnulib-local/lib/libcroco/cr-token.c \
  gnulib-local/lib/libcroco/cr-token.h \
  gnulib-local/lib/libcroco/cr-utils.c \
  gnulib-local/lib/libcroco/cr-utils.h \
  gnulib-local/lib/libcroco/libcroco-config.h \
  gnulib-local/lib/libcroco/libcroco.h \
  gnulib-local/lib/libcroco/README \
  gnulib-local/lib/libxml/xmlversion.in.h.diff \
  gnulib-local/lib/memory-ostream.oo.c \
  gnulib-local/lib/memory-ostream.oo.h \
  gnulib-local/lib/moo.h \
  gnulib-local/lib/noop-styled-ostream.oo.c \
  gnulib-local/lib/noop-styled-ostream.oo.h \
  gnulib-local/lib/ostream.oo.c \
  gnulib-local/lib/ostream.oo.h \
  gnulib-local/lib/styled-ostream.oo.c \
  gnulib-local/lib/styled-ostream.oo.h \
  gnulib-local/lib/term-ostream.oo.c \
  gnulib-local/lib/term-ostream.oo.h \
  gnulib-local/lib/term-styled-ostream.oo.c \
  gnulib-local/lib/term-styled-ostream.oo.h \
  gnulib-local/m4/libcroco.m4 \
  gnulib-local/m4/libglib.m4 \
  gnulib-local/m4/moo.m4 \
  gnulib-local/m4/term-ostream.m4 \
  gnulib-local/modules/fd-ostream \
  gnulib-local/modules/file-ostream \
  gnulib-local/modules/html-ostream \
  gnulib-local/modules/html-styled-ostream \
  gnulib-local/modules/iconv-ostream \
  gnulib-local/modules/libcroco \
  gnulib-local/modules/libglib \
  gnulib-local/modules/libxml.diff \
  gnulib-local/modules/memory-ostream \
  gnulib-local/modules/memory-ostream-tests \
  gnulib-local/modules/moo \
  gnulib-local/modules/moo-tests \
  gnulib-local/modules/noop-styled-ostream \
  gnulib-local/modules/ostream \
  gnulib-local/modules/styled-ostream \
  gnulib-local/modules/term-ostream \
  gnulib-local/modules/term-ostream-tests \
  gnulib-local/modules/term-styled-ostream \
  gnulib-local/tests/test-memory-ostream.c \
  gnulib-local/tests/test-moo-aroot.oo.c \
  gnulib-local/tests/test-moo-aroot.oo.h \
  gnulib-local/tests/test-moo-assign.c \
  gnulib-local/tests/test-moo-asub1.oo.c \
  gnulib-local/tests/test-moo-asub1.oo.h \
  gnulib-local/tests/test-moo-root.oo.c \
  gnulib-local/tests/test-moo-root.oo.h \
  gnulib-local/tests/test-moo-sub1.oo.c \
  gnulib-local/tests/test-moo-sub1.oo.h \
  gnulib-local/tests/test-moo-sub2.oo.c \
  gnulib-local/tests/test-moo-sub2.oo.h \
  gnulib-local/tests/test-term-ostream.c \
  gnulib-local/tests/test-term-ostream-xterm.sh \
  gnulib-local/tests/test-term-ostream-xterm-16color.out \
  gnulib-local/tests/test-term-ostream-xterm-256color.out \
  gnulib-local/tests/test-term-ostream-xterm-88color.out \
  gnulib-local/tests/test-term-ostream-xterm-8bit.out \
  gnulib-local/tests/test-term-ostream-xterm-aix51.out \
  gnulib-local/tests/test-term-ostream-xterm-basic-italic.out \
  gnulib-local/tests/test-term-ostream-xterm-basic.out \
  gnulib-local/tests/test-term-ostream-xterm-freebsd101.out \
  gnulib-local/tests/test-term-ostream-xterm-freebsd-italic.out \
  gnulib-local/tests/test-term-ostream-xterm-irix65.out \
  gnulib-local/tests/test-term-ostream-xterm-linux-debian.out \
  gnulib-local/tests/test-term-ostream-xterm-linux-mandriva.out \
  gnulib-local/tests/test-term-ostream-xterm-mingw.out \
  gnulib-local/tests/test-term-ostream-xterm-netbsd3.out \
  gnulib-local/tests/test-term-ostream-xterm-osf51.out \
  gnulib-local/tests/test-term-ostream-xterm-r6.out \
  gnulib-local/tests/test-term-ostream-xterm-solaris10.out \
  gnulib-local/tests/test-term-ostream-xterm-xf86-v32.out \
  build-aux/moopp \
  build-aux/join-v-1 \
  build-aux/texi2html \
  build-aux/run-test \
  build-aux/test-driver.diff \
  m4/libtextstyle.m4 \
  examples/color-filter/README \
  examples/color-filter/INSTALL \
  examples/color-filter/autogen.sh \
  examples/color-filter/autoclean.sh \
  examples/color-filter/configure.ac \
  examples/color-filter/Makefile.am \
  examples/color-filter/filter.c \
  examples/color-filter/filter-default.css \
  examples/color-hello/README \
  examples/color-hello/INSTALL \
  examples/color-hello/autogen.sh \
  examples/color-hello/autoclean.sh \
  examples/color-hello/configure.ac \
  examples/color-hello/Makefile.am \
  examples/color-hello/hello.c \
  examples/color-hello/names.c \
  examples/color-hello/hello-default.css


# Allow users to use "gnulib-tool --update".
EXTRA_DIST += gnulib-m4/gnulib-cache.m4


# Windows support.

EXTRA_DIST += INSTALL.windows
