## Process this file with automake to generate Makefile.in
##
##   Copyright (C) 2012-2023 Free Software Foundation, Inc.
##
## This file is free software; you can redistribute it and/or modify
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
## along with this program; see the file COPYING3.  If not see
## <http://www.gnu.org/licenses/>.
##

DOCFILES = \
	%D%/aoutx.texi \
	%D%/archive.texi \
	%D%/archures.texi \
	%D%/bfdio.texi \
	%D%/bfdt.texi \
	%D%/bfdver.texi \
	%D%/bfdwin.texi \
	%D%/cache.texi \
	%D%/coffcode.texi \
	%D%/corefile.texi \
	%D%/elfcode.texi \
	%D%/elf.texi \
	%D%/format.texi \
	%D%/hash.texi \
	%D%/libbfd.texi \
	%D%/linker.texi \
	%D%/mmo.texi \
	%D%/opncls.texi \
	%D%/reloc.texi \
	%D%/section.texi \
	%D%/syms.texi \
	%D%/targets.texi

# SRCDOC, SRCPROT, SRCIPROT only used to sidestep Sun Make bug in interaction
# between VPATH and suffix rules.  If you use GNU Make, perhaps other Makes,
# you don't need these three:
SRCDOC = \
	$(srcdir)/aoutx.h $(srcdir)/archive.c \
	$(srcdir)/archures.c $(srcdir)/bfd.c \
	$(srcdir)/bfdio.c $(srcdir)/bfdwin.c \
	$(srcdir)/cache.c $(srcdir)/coffcode.h \
	$(srcdir)/corefile.c $(srcdir)/elf.c \
	$(srcdir)/elfcode.h $(srcdir)/format.c \
	$(srcdir)/libbfd.c $(srcdir)/opncls.c \
	$(srcdir)/reloc.c $(srcdir)/section.c \
	$(srcdir)/syms.c $(srcdir)/targets.c \
	$(srcdir)/hash.c $(srcdir)/linker.c \
	$(srcdir)/mmo.c

SRCPROT = $(srcdir)/archive.c $(srcdir)/archures.c \
	$(srcdir)/bfd.c $(srcdir)/coffcode.h $(srcdir)/corefile.c \
	$(srcdir)/format.c $(srcdir)/libbfd.c \
	$(srcdir)/bfdio.c $(srcdir)/bfdwin.c \
	$(srcdir)/opncls.c $(srcdir)/reloc.c \
	$(srcdir)/section.c $(srcdir)/syms.c \
	$(srcdir)/targets.c

SRCIPROT = $(srcdir)/cache.c $(srcdir)/libbfd.c \
	$(srcdir)/bfdio.c $(srcdir)/bfdwin.c \
	$(srcdir)/reloc.c $(srcdir)/cpu-h8300.c \
	$(srcdir)/archures.c

TEXIDIR = $(srcdir)/../texinfo/fsf

info_TEXINFOS = %D%/bfd.texi
%C%_bfd_TEXINFOS = $(DOCFILES) %D%/bfdsumm.texi

AM_MAKEINFOFLAGS = --no-split -I "$(srcdir)/%D%" -I %D%
TEXI2DVI = texi2dvi -I "$(srcdir)/%D%" -I %D%

MKDOC = %D%/chew$(EXEEXT_FOR_BUILD)

$(MKDOC): %D%/chew.stamp ; @true
%D%/chew.stamp: $(srcdir)/%D%/chew.c %D%/$(am__dirstamp)
	$(AM_V_CCLD)$(CC_FOR_BUILD) -o %D%/chw$$$$$(EXEEXT_FOR_BUILD) $(CFLAGS_FOR_BUILD) \
	  $(LDFLAGS_FOR_BUILD) \
	  -I. -I$(srcdir) -I%D% -I$(srcdir)/../include -I$(srcdir)/../intl -I../intl \
	  $(srcdir)/%D%/chew.c && \
	$(SHELL) $(srcdir)/../move-if-change \
	  %D%/chw$$$$$(EXEEXT_FOR_BUILD) $(MKDOC) && \
	touch $@

# We can't replace these rules with an implicit rule, because
# makes without VPATH support couldn't find the .h files in `..'.

# We do not depend on chew directly so that we can distribute the info
# files, and permit people to rebuild them, without requiring the makeinfo
# program.  If somebody tries to rebuild info, but none of the .texi files
# have changed, then nothing will be rebuilt.

REGEN_TEXI = \
	( \
	set -e; \
	$(MKDOC) -f $(srcdir)/%D%/doc.str < $< > $@.tmp; \
	texi=$@; \
	texi=$${texi%.stamp}.texi; \
	test -e $$texi || test ! -f $(srcdir)/$$texi || $(LN_S) $(srcdir)/$$texi .; \
	$(SHELL) $(srcdir)/../move-if-change $@.tmp $$texi; \
	touch $@; \
	)

.PRECIOUS: %D%/%.stamp
%D%/%.texi: %D%/%.stamp ; @true
%D%/%.stamp: $(srcdir)/%.h $(srcdir)/%D%/doc.str $(MKDOC) %D%/$(am__dirstamp)
	$(AM_V_GEN)$(REGEN_TEXI)
%D%/%.stamp: $(srcdir)/%.c $(srcdir)/%D%/doc.str $(MKDOC) %D%/$(am__dirstamp)
	$(AM_V_GEN)$(REGEN_TEXI)

# Avoid the %.stamp generating a builddir/bfd.texi that overrides the
# srcdir/ as well as regenerating doc/bfd.info for each make run.
%D%/bfd.stamp: $(srcdir)/%D%/bfd.texi ; $(AM_V_at)touch $@

# We use bfdt.texi, rather than bfd.texi, to avoid conflicting with
# bfd.texi on an 8.3 filesystem.
%D%/bfdt.stamp: $(srcdir)/bfd.c $(srcdir)/%D%/doc.str $(MKDOC) %D%/$(am__dirstamp)
	$(AM_V_GEN)$(REGEN_TEXI)

%D%/bfdver.texi: $(srcdir)/Makefile.in
	$(AM_V_GEN)\
	$(MKDIR_P) $(@D); \
	echo "@set VERSION $(VERSION)" > $@; \
	if test -n "$(PKGVERSION)"; then \
	  echo "@set VERSION_PACKAGE $(PKGVERSION)" >> $@; \
	fi; \
	echo "@set UPDATED `date '+%B %Y'`" >> $@; \
	if test -n "$(REPORT_BUGS_TEXI)"; then \
	  echo "@set BUGURL $(REPORT_BUGS_TEXI)" >> $@; \
	fi

noinst_TEXINFOS = %D%/bfdint.texi

MOSTLYCLEANFILES += $(MKDOC) %D%/*.o %D%/*.stamp

DISTCLEANFILES += %D%/bfd.?? %D%/bfd.??? texput.log

MAINTAINERCLEANFILES += $(DOCFILES)

html-local: %D%/bfd/index.html
%D%/bfd/index.html: %D%/bfd.texi $(bfd_TEXINFOS) %D%/$(am__dirstamp)
	$(AM_V_at)$(MAKEINFOHTML) $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) \
	  --split=node -o %D%/bfd $(srcdir)/%D%/bfd.texi

MAINTAINERCLEANFILES += %D%/bfd.info
