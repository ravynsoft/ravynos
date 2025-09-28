## Process this file with automake to generate Makefile.in
#
#   Copyright (C) 2012-2023 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING3.  If not see
# <http://www.gnu.org/licenses/>.
#

AUTOMAKE_OPTIONS = foreign no-dist no-texinfo.tex info-in-builddir
ACLOCAL_AMFLAGS = -I .. -I ../config -I ../bfd
TEXINFO_TEX = ../texinfo/texinfo.tex

SUFFIXES = .c .m

SUBDIRS = po

BASEDIR = $(srcdir)/..
BFDDIR = $(BASEDIR)/bfd
INCDIR	= $(BASEDIR)/include

WARN_CFLAGS = @WARN_CFLAGS@
NO_WERROR = @NO_WERROR@
AM_CFLAGS = $(WARN_CFLAGS)

AM_CPPFLAGS = -DDEBUG -I../bfd -I$(srcdir)/../include \
	-I$(srcdir)/../bfd @INCINTL@ @LARGEFILE_CPPFLAGS@ -I. \
	-DLOCALEDIR="\"$(datadir)/locale\""

bin_PROGRAMS = gprof

## Convenience var listing pure sources.
sources = basic_blocks.c call_graph.c cg_arcs.c cg_dfn.c \
	cg_print.c corefile.c gmon_io.c gprof.c hertz.c hist.c source.c \
	search_list.c symtab.c sym_ids.c utils.c \
	i386.c alpha.c vax.c sparc.c mips.c aarch64.c
gprof_SOURCES = $(sources) flat_bl.c bsd_callg_bl.c fsf_callg_bl.c
gprof_DEPENDENCIES = ../bfd/libbfd.la ../libiberty/libiberty.a $(LIBINTL_DEP)
gprof_LDADD = ../bfd/libbfd.la ../libiberty/libiberty.a $(LIBINTL)

noinst_HEADERS = \
	basic_blocks.h call_graph.h cg_arcs.h cg_dfn.h cg_print.h \
	corefile.h gmon.h gmon_io.h gmon_out.h gprof.h hertz.h hist.h \
	search_list.h source.h sym_ids.h symtab.h utils.h

BUILT_SOURCES = flat_bl.c bsd_callg_bl.c fsf_callg_bl.c
EXTRA_DIST    = $(BUILT_SOURCES) bbconv.pl $(man_MANS)

diststuff: $(BUILT_SOURCES) info $(man_MANS)

# development.sh is used to determine -Werror default.
CONFIG_STATUS_DEPENDENCIES = $(BFDDIR)/development.sh

# This empty rule is a hack against gmake patched by Apple.
%.o:%.m

.m.c:
	$(AM_V_GEN)awk -f $(srcdir)/gen-c-prog.awk > $@ \
	    FUNCTION=`(echo $*|sed -e 's,.*/,,g' -e 's/_bl//')`_blurb \
	    FILE=$*.m $<

POTFILES = $(sources) $(noinst_HEADERS)
po/POTFILES.in: @MAINT@ Makefile
	for f in $(POTFILES); do echo $$f; done | LC_ALL=C sort > tmp \
	  && mv tmp $(srcdir)/po/POTFILES.in

MANCONF  = -Dman

TEXI2POD = perl $(srcdir)/../etc/texi2pod.pl $(AM_MAKEINFOFLAGS)

POD2MAN = pod2man --center="GNU" --release="binutils-$(VERSION)" --section=1

info_TEXINFOS = gprof.texi
gprof_TEXINFOS = config.texi
man_MANS = gprof.1

AM_MAKEINFOFLAGS = -I "$(srcdir)" -I "$(top_srcdir)/../bfd/doc" -I ../bfd/doc \
		   --no-split
TEXI2DVI = texi2dvi -I "$(srcdir)" -I "$(top_srcdir)/../bfd/doc" -I ../bfd/doc

config.texi:
	$(AM_V_GEN)echo "@set top_srcdir $(top_srcdir)" >> ./config.texi

# Build the man page from the texinfo file
# The sed command removes the no-adjust Nroff command so that
# the man output looks standard.
gprof.1: $(srcdir)/gprof.texi config.texi
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dgprof < $(srcdir)/gprof.texi > gprof.pod
	$(AM_V_at)-($(POD2MAN) gprof.pod | \
	       sed -e '/^.if n .na/d' > $@.T$$$$ && \
	       mv -f $@.T$$$$ $@) || \
	       (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f gprof.pod

html-local: doc/gprof/index.html
doc/gprof/index.html: gprof.texi $(gprof_TEXINFOS)
	$(AM_V_at)$(MKDIR_P) doc
	$(AM_V_GEN)$(MAKEINFOHTML) $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) \
	  --split=node -I$(srcdir) $(srcdir)/gprof.texi -o doc/gprof

MAINTAINERCLEANFILES = gprof.info
