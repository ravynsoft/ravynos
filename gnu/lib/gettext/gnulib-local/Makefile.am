## Makefile for the gnulib-local directory of GNU gettext
## Copyright (C) 2006-2008, 2010-2012, 2014-2023 Free Software Foundation, Inc.
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

# Generate this list with
# find . '(' -name Makefile.am ')' -prune -o -type f '!' '(' -name '*.orig' -or -name '*~' -or -name 'ChangeLog.*' ')' -printf '%P\n' | sort | tr '\012' @ | sed 's/@$/%/;s/@/ \\@/g' | tr @% '\012\012'
EXTRA_DIST = \
lib/addext.c \
lib/alloca.in.h \
lib/argmatch.h.diff \
lib/backupfile.c \
lib/backupfile.h \
lib/closeout.c \
lib/closeout.h \
lib/error.in.h.diff \
lib/error-progname.c \
lib/error-progname.h \
lib/exitfail.h.diff \
lib/getopt-core.h.diff \
lib/gettext.h \
lib/gl_array_list.h.diff \
lib/gl_linkedhash_list.h.diff \
lib/hash.c.diff \
lib/hash.h.diff \
lib/libxml/buf.c \
lib/libxml/buf.h \
lib/libxml/c14n.c \
lib/libxml/c14n.in.h \
lib/libxml/catalog.c \
lib/libxml/catalog.in.h \
lib/libxml/chvalid.c \
lib/libxml/chvalid.in.h \
lib/libxml/COPYING \
lib/libxml/debugXML.c \
lib/libxml/debugXML.in.h \
lib/libxml/dict.c \
lib/libxml/dict.in.h \
lib/libxml/DOCBparser.c \
lib/libxml/DOCBparser.in.h \
lib/libxml/elfgcchack.h \
lib/libxml/enc.h \
lib/libxml/encoding.c \
lib/libxml/encoding.in.h \
lib/libxml/entities.c \
lib/libxml/entities.in.h \
lib/libxml/error.c \
lib/libxml/globals.c \
lib/libxml/globals.in.h \
lib/libxml/hash.c \
lib/libxml/hash.in.h \
lib/libxml/HTMLparser.c \
lib/libxml/HTMLparser.in.h \
lib/libxml/HTMLtree.c \
lib/libxml/HTMLtree.in.h \
lib/libxml/legacy.c \
lib/libxml/libxml.h \
lib/libxml/list.c \
lib/libxml/list.in.h \
lib/libxml/nanoftp.c \
lib/libxml/nanoftp.in.h \
lib/libxml/nanohttp.c \
lib/libxml/nanohttp.in.h \
lib/libxml/parser.c \
lib/libxml/parser.in.h \
lib/libxml/parserInternals.c \
lib/libxml/parserInternals.in.h \
lib/libxml/pattern.c \
lib/libxml/pattern.in.h \
lib/libxml/README \
lib/libxml/relaxng.c \
lib/libxml/relaxng.in.h \
lib/libxml/save.h \
lib/libxml/SAX2.c \
lib/libxml/SAX2.in.h \
lib/libxml/SAX.c \
lib/libxml/SAX.in.h \
lib/libxml/schemasInternals.in.h \
lib/libxml/schematron.c \
lib/libxml/schematron.in.h \
lib/libxml/threads.c \
lib/libxml/threads.in.h \
lib/libxml/timsort.h \
lib/libxml/tree.c \
lib/libxml/tree.in.h \
lib/libxml/trionan.c \
lib/libxml/uri.c \
lib/libxml/uri.in.h \
lib/libxml/valid.c \
lib/libxml/valid.in.h \
lib/libxml/xinclude.c \
lib/libxml/xinclude.in.h \
lib/libxml/xlink.c \
lib/libxml/xlink.in.h \
lib/libxml/xmlautomata.in.h \
lib/libxml/xmlerror.in.h \
lib/libxml/xmlexports.in.h \
lib/libxml/xmlIO.c \
lib/libxml/xmlIO.in.h \
lib/libxml/xmlmemory.c \
lib/libxml/xmlmemory.in.h \
lib/libxml/xmlmodule.c \
lib/libxml/xmlmodule.in.h \
lib/libxml/xmlreader.c \
lib/libxml/xmlreader.in.h \
lib/libxml/xmlregexp.c \
lib/libxml/xmlregexp.in.h \
lib/libxml/xmlsave.c \
lib/libxml/xmlsave.in.h \
lib/libxml/xmlschemas.c \
lib/libxml/xmlschemas.in.h \
lib/libxml/xmlschemastypes.c \
lib/libxml/xmlschemastypes.in.h \
lib/libxml/xmlstring.c \
lib/libxml/xmlstring.in.h \
lib/libxml/xmlunicode.c \
lib/libxml/xmlunicode.in.h \
lib/libxml/xmlversion.in.h \
lib/libxml/xmlwriter.c \
lib/libxml/xmlwriter.in.h \
lib/libxml/xpath.c \
lib/libxml/xpath.in.h \
lib/libxml/xpathInternals.in.h \
lib/libxml/xpointer.c \
lib/libxml/xpointer.in.h \
lib/markup.c \
lib/markup.h \
lib/mem-hash-map.c \
lib/mem-hash-map.h \
lib/obstack.h.diff \
lib/progname.h.diff \
lib/regex_internal.h.diff \
lib/unistd.in.h.diff \
lib/xerror.c \
lib/xerror.h \
lib/xgetcwd.c \
lib/xgetcwd.h \
m4/backupfile.m4 \
m4/java.m4 \
m4/libxml.m4 \
modules/backupfile \
modules/closeout \
modules/error-progname \
modules/gettext-runtime-intl-misc \
modules/gettext-runtime-misc \
modules/gettext-tools-misc \
modules/java \
modules/libtextstyle-dummy \
modules/libxml \
modules/markup \
modules/mem-hash-map \
modules/xerror \
modules/xgetcwd

# Extra files to be installed.

gettextsrcdir = $(datadir)/gettext
gettextsrc_DATA = lib/gettext.h

maintainer-update-po:
